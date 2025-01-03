#include "exception.h"

#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

void init_exception(void) __attribute__((constructor));
void init_exception(void)
{
    exception_main_thread_id(pthread_self());
}

pthread_t exception_main_thread_id(pthread_t thread_id)
{
    static _Thread_local pthread_t main_thread_id;

    if (thread_id != -1) {
        main_thread_id = thread_id;
    }
    return main_thread_id;
}

internal_exception_t *internal_exception(void)
{
    static _Thread_local exception_t exception = {.message = NULL, .code = 0};
    static _Thread_local internal_exception_t iex = {
        .jmp_buf = NULL, .exception = NULL};

    if (iex.exception == NULL) {
        iex.exception = &exception;
    }

    return &iex;
}

exception_t *exception(void)
{
    internal_exception_t *iex = internal_exception();

    return iex->exception;
}

jmp_buf *internal_exception_save()
{
    internal_exception_t *iex = internal_exception();

    return iex->jmp_buf;
}

jmp_buf *internal_exception_try(jmp_buf *buf)
{
    internal_exception_t *iex = internal_exception();

    iex->jmp_buf = buf;
    return iex->jmp_buf;
}

bool internal_exception_catch(int code)
{
    internal_exception_t *iex = internal_exception();

    if (!code || iex->exception->code == code) {
        return true;
    }
    return false;
}

static void internal_exception_set_message(const char *fmt, va_list varg)
{
    internal_exception_t *iex = internal_exception();
    char *message = NULL;
    va_list varg_copy;
    size_t size = 0;

    va_copy(varg_copy, varg);
    size = vsnprintf(NULL, 0, fmt, varg_copy);
    va_end(varg_copy);
    if (size > 0) {
        message = calloc(size + 1, sizeof(char));
        if (message) {
            size = vsnprintf(message, size + 1, fmt, varg);
        }
    }
    if (iex->exception->message) {
        free(iex->exception->message);
    }
    iex->exception->message = message;
}

void internal_exception_throw(int code, const char *fmt, ...)
{
    internal_exception_t *iex = internal_exception();
    va_list varg;

    if (iex->jmp_buf) {
        va_start(varg, fmt);
        internal_exception_set_message(fmt, varg);
        va_end(varg);
        longjmp(*iex->jmp_buf, code);
    }
}

void internal_exception_rethrow(void)
{
    internal_exception_t *iex = internal_exception();

    if (iex->jmp_buf) {
        longjmp(*iex->jmp_buf, iex->exception->code);
    }
}

void internal_exception_destroy(void)
{
    internal_exception_t *iex = internal_exception();

    if (iex->exception->message) {
        free(iex->exception->message);
    }
    iex->exception->message = NULL;
}
