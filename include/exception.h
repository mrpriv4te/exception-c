#pragma once

#include <err.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define try(code_block)                                                        \
    do {                                                                       \
        if (!exception_try(setjmp(*(exception_control_flow_push(               \
                &(control_flow_node_t){.jmp_buf = &(jmp_buf){}, .next = NULL}) \
                    ->head->jmp_buf))))                                        \
            code_block /* NOLINT */                                            \
                else                                                           \
            {                                                                  \
                (void)exception_control_flow_pop();                            \
                rethrow();                                                     \
            }                                                                  \
        (void)exception_control_flow_pop();                                    \
        exception_cleanup();                                                   \
    } while (0);

#define catch(code) else if (exception_catch(code))

#define throw(code, fmt, ...)                         \
    do {                                              \
        exception_throw(code, fmt, ##__VA_ARGS__);    \
        exception_exit(__func__, __FILE__, __LINE__); \
    } while (0)

#define rethrow()                                     \
    do {                                              \
        exception_rethrow();                          \
        exception_exit(__func__, __FILE__, __LINE__); \
    } while (0)

#define all_exception 0

typedef struct exception_s {
    char *message;
    int code;
} __attribute__((aligned(16))) exception_t;

typedef struct control_flow_node_s {
    jmp_buf *jmp_buf;
    struct control_flow_node_s *next;
} __attribute__((aligned(16))) control_flow_node_t;

typedef struct control_flow_s {
    control_flow_node_t *head;
} control_flow_t;

typedef struct exception_state_s {
    exception_t exception;
    control_flow_t control_flow;
} __attribute__((aligned(32))) exception_state_t;

control_flow_t *exception_control_flow_push(control_flow_node_t *cflow);
control_flow_node_t *exception_control_flow_pop();

const exception_t *exception();
int exception_try(int code);
bool exception_catch(int code);
void exception_throw(int code, const char *fmt, ...);
void exception_rethrow();
void exception_cleanup();
_Noreturn void exception_exit(const char *func, const char *file, int line);
