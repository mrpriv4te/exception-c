#pragma once

#include <err.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define try                                                            \
    do {                                                               \
        jmp_buf __local;                                               \
        jmp_buf *__save = internal_exception_save();                   \
        exception()->code = setjmp(*internal_exception_try(&__local)); \
        if (!exception()->code)

#define all_exception 0

#define catch(code) else if (internal_exception_catch(code))

#define endtry                                  \
    else                                        \
    {                                           \
        internal_exception()->jmp_buf = __save; \
        rethrow();                              \
    }                                           \
    internal_exception()->jmp_buf = __save;     \
    }                                           \
    while (0)

#ifndef NDEBUG
    #define throw(code, fmt, ...)                                       \
        do {                                                            \
            internal_exception_throw(code, fmt, ##__VA_ARGS__);         \
            if (pthread_self() == exception_main_thread_id(-1)) {       \
                errx(code, "%s:%d: %s(): " fmt " (code %d)", __FILE__,  \
                    __LINE__, __func__, ##__VA_ARGS__, code);           \
            } else {                                                    \
                warnx("Thread %ld: %s:%d: %s(): " fmt " (code %d)",     \
                    (long)pthread_self(), __FILE__, __LINE__, __func__, \
                    ##__VA_ARGS__, code);                               \
                pthread_exit((void *)code);                             \
            }                                                           \
        } while (0)

    #define rethrow()                                                          \
        do {                                                                   \
            internal_exception_rethrow();                                      \
            if (pthread_self() == exception_main_thread_id(-1)) {              \
                errx(exception()->code, "%s:%d: %s(): %s (code %d)", __FILE__, \
                    __LINE__, __func__, exception()->message,                  \
                    exception()->code);                                        \
            } else {                                                           \
                warnx("Thread %ld: %s:%d: %s(): %s (code %d)",                 \
                    (long)pthread_self(), __FILE__, __LINE__, __func__,        \
                    exception()->message, exception()->code);                  \
                pthread_exit(                                                  \
                    (void *)(intptr_t)exception()->code); /* NOLINT */         \
            }                                                                  \
        } while (0)
#else
    #define throw(code, fmt, ...)                                            \
        do {                                                                 \
            internal_exception_throw(code, fmt, ##__VA_ARGS__);              \
            if (pthread_self() == exception_main_thread_id(-1)) {            \
                errx(code, fmt " (code %d)", ##__VA_ARGS__, code);           \
            } else {                                                         \
                warnx("Thread %ld: " fmt " (code %d)", (long)pthread_self(), \
                    ##__VA_ARGS__, code);                                    \
                pthread_exit((void *)code);                                  \
            }                                                                \
        } while (0)

    #define rethrow()                                                         \
        do {                                                                  \
            internal_exception_rethrow();                                     \
            if (pthread_self() == exception_main_thread_id(-1)) {             \
                errx(exception()->code, "%s (code %d)", exception()->message, \
                    exception()->code);                                       \
            } else {                                                          \
                warnx("Thread %ld: %s (code %d)", (long)pthread_self(),       \
                    exception()->message, exception()->code);                 \
                pthread_exit(                                                 \
                    (void *)(intptr_t)exception()->code); /* NOLINT */        \
            }                                                                 \
        } while (0)
#endif

typedef struct exception_s {
    char *message;
    int code;
} __attribute__((aligned(16))) exception_t;

typedef struct internal_exception_s {
    jmp_buf *jmp_buf;
    exception_t *exception;
} __attribute__((aligned(16))) internal_exception_t;

exception_t *exception(void);

pthread_t exception_main_thread_id(pthread_t thread_id);

internal_exception_t *internal_exception(void);

jmp_buf *internal_exception_save();

jmp_buf *internal_exception_try(jmp_buf *buf);

bool internal_exception_catch(int code);

void internal_exception_endtry(jmp_buf *save);

void internal_exception_throw(int code, const char *fmt, ...);
void internal_exception_rethrow(void);

void internal_exception_destroy(void) __attribute__((destructor));
