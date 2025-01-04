#include "exception.h"

#include <assert.h>
#include <bits/pthreadtypes.h>
#include <err.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

// NOLINTBEGIN
static _Thread_local pthread_t exception_main_thread_id;

static _Thread_local exception_state_t exception_state = {
    .exception = {.message = NULL, .code = 0},
    .control_flow = {.head = NULL},
};
// NOLINTEND

static void init_exception(void) __attribute__((constructor));
static void init_exception(void)
{
    exception_main_thread_id = pthread_self();
}

static void exception_set_message(const char *fmt, va_list varg)
{
    char *message = NULL;
    va_list varg_copy;
    size_t size = 0;

    if (fmt != NULL) {
        va_copy(varg_copy, varg);
        size = vsnprintf(NULL, 0, fmt, varg_copy);
        va_end(varg_copy);
        if (size > 0) {
            message = calloc(size + 1, sizeof(char));
            if (message) {
                (void)vsnprintf(message, size + 1, fmt, varg);
            }
        }
    }
    if (exception_state.exception.message != NULL) {
        free(exception_state.exception.message);
    }
    exception_state.exception.message = message;
}

control_flow_t *exception_control_flow_push(control_flow_node_t *cflow)
{
    assert(cflow != NULL && cflow->jmp_buf != NULL);
    cflow->next = exception_state.control_flow.head;
    exception_state.control_flow.head = cflow;
    return &exception_state.control_flow;
}

control_flow_node_t *exception_control_flow_pop()
{
    control_flow_node_t *tmp = exception_state.control_flow.head;

    if (tmp != NULL) {
        exception_state.control_flow.head = tmp->next;
    }
    return tmp;
}

const exception_t *exception()
{
    return &exception_state.exception;
}

int exception_try(int code)
{
    exception_state.exception.code = code;
    return code;
}

bool exception_catch(int code)
{
    if (code == all_exception || exception_state.exception.code == code) {
        return true;
    }
    return false;
}

void exception_throw(int code, const char *fmt, ...)
{
    control_flow_node_t *cflow = exception_state.control_flow.head;
    va_list varg;

    va_start(varg, fmt);
    exception_set_message(fmt, varg);
    va_end(varg);

    exception_state.exception.code = code;

    if (cflow != NULL && cflow->jmp_buf != NULL) {
        longjmp(*cflow->jmp_buf, code);
    }
}

void exception_rethrow()
{
    control_flow_node_t *cflow = exception_state.control_flow.head;

    if (cflow != NULL && cflow->jmp_buf != NULL) {
        longjmp(*cflow->jmp_buf, exception_state.exception.code);
    }
}

void exception_cleanup()
{
    if (exception_state.exception.message != NULL) {
        free(exception_state.exception.message);
        exception_state.exception.message = NULL;
    }
    exception_state.exception.code = 0;
}

void exception_exit(const char *func, const char *file, int line)
{
    const char *msg = exception_state.exception.message;
    int code = exception_state.exception.code;
    pthread_t thread_id = pthread_self();

    if (thread_id == exception_main_thread_id) {
#ifndef NDEBUG
        warnx("%s:%d: %s(): %s (code %d)", file, line, func, msg, code);
#else
        warnx("%s (code %d)", msg, code);
#endif
        exception_cleanup();
        exit(code); // NOLINT
    } else {
#ifndef NDEBUG
        warnx("thread %ld: %s:%d: %s(): %s (code %d)", thread_id, file, line,
            func, msg, code);
#else
        warnx("thread %ld: %s (code %d)", thread_id, msg, code);
#endif
        exception_cleanup();
        pthread_exit((void *)code); // NOLINT
    }
}

static void destroy_exception(void) __attribute__((destructor));
static void destroy_exception(void)
{
    exception_cleanup();
}
