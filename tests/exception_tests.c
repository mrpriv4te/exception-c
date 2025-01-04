#include "exception.h"

#include <bits/pthreadtypes.h>
#include <cmocka.h>
#include <pthread.h>
#include <sched.h>
#include <setjmp.h> // IWYU pragma: keep
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define NUM_THREADS 4

void test_throw_and_catch(void **state)
{
    (void)state;

    try({
        throw(1, "Test exception");
    } catch (1) {
        assert_string_equal(exception()->message, "Test exception");
        assert_int_equal(exception()->code, 1);
    })
}

void test_throw_uncaught(void **state) // NOLINT
{
    (void)state;

    pid_t pid = fork();

    if (pid == 0) {
        try({
            throw(2, "Uncaught exception");
        })

        exit(EXIT_SUCCESS); // NOLINT
    } else if (pid > 0) {
        int status = 0;
        waitpid(pid, &status, 0);
        assert_true(WIFEXITED(status));
        assert_int_equal(WEXITSTATUS(status), 2);
    } else {
        fail_msg("Fork failed");
    }
}

void *thread_function(void *arg) // NOLINT
{
    int thread_id = *(int *)arg;

    try({
        if (thread_id % 2 == 0) {
            throw(100 + thread_id, "Thread %d exception", thread_id);
        }
    } catch (100 + thread_id) {
        char expected_message[64];
        (void)snprintf(expected_message, sizeof(expected_message),
            "Thread %d exception", thread_id);
        assert_string_equal(exception()->message, expected_message);
        assert_int_equal(exception()->code, 100 + thread_id);
    })

    return NULL;
}

void test_multithreaded_exceptions(void **state)
{
    (void)state;

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        assert_int_equal(
            pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]),
            0);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        assert_int_equal(pthread_join(threads[i], NULL), 0);
    }
}

void test_message_update(void **state) // NOLINT
{
    (void)state;

    try({
        throw(3, "First exception");
    } catch (3) {
        assert_string_equal(exception()->message, "First exception");
        throw(4, "Second exception");
    } catch (4) {
        assert_string_equal(exception()->message, "Second exception");
    })
}

void test_thread_safety(void **state)
{
    (void)state;

    pthread_t thread1 = 0;
    pthread_t thread2 = 0;
    int id1 = 1;
    int id2 = 2;

    assert_int_equal(pthread_create(&thread1, NULL, thread_function, &id1), 0);
    assert_int_equal(pthread_create(&thread2, NULL, thread_function, &id2), 0);

    assert_int_equal(pthread_join(thread1, NULL), 0);
    assert_int_equal(pthread_join(thread2, NULL), 0);
}

void *thread_function_uncaught(void *arg) // NOLINT
{
    int thread_id = *(int *)arg;

    if (thread_id % 2 == 0) {
        try({
            throw(50 + thread_id, "Thread %d uncaught exception", thread_id);
        })

        (void)fprintf(stderr, "Thread %d should have terminated.\n", thread_id);
        pthread_exit((void *)EXIT_FAILURE);
    }

    pthread_exit((void *)EXIT_SUCCESS);
}

void test_multithreaded_exceptions_uncaught(void **state)
{
    (void)state;

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    void *thread_return[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        assert_int_equal(pthread_create(&threads[i], NULL,
                             thread_function_uncaught, &thread_ids[i]),
            0);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        assert_int_equal(pthread_join(threads[i], &thread_return[i]), 0);
        if (i % 2 == 0) {
            assert_int_equal(thread_return[i], 50 + thread_ids[i]);
        } else {
            assert_int_equal(thread_return[i], EXIT_SUCCESS);
        }
    }
}

static void third_level_function(void)
{
    throw(5, "Third level exception");
}

static void second_level_function(void)
{
    third_level_function();
}

static void first_level_function(void)
{
    try({
        second_level_function();
    } catch (5) {
        assert_string_equal(exception()->message, "Third level exception");
        assert_int_equal(exception()->code, 5);
        throw(6, "First level rethrown exception");
    })
}

void test_nested_function_call(void **state)
{
    (void)state;

    try({
        first_level_function();
    } catch (6) {
        assert_string_equal(
            exception()->message, "First level rethrown exception");
        assert_int_equal(exception()->code, 6);
    })
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_throw_and_catch),
        cmocka_unit_test(test_throw_uncaught),
        cmocka_unit_test(test_message_update),
        cmocka_unit_test(test_nested_function_call),
        cmocka_unit_test(test_multithreaded_exceptions),
        cmocka_unit_test(test_multithreaded_exceptions_uncaught),
        cmocka_unit_test(test_thread_safety),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
