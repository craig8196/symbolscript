/*******************************************************************************
 * The MIT License (MIT)
 *
 * Original Author:
 * Copyright (c) 2016 Dmitriy Kubyshkin <dmitriy@kubyshkin.name>
 * https://github.com/grassator/bdd-for-c
 *
 * Modifications:
 * Copyright (c) 2020-2021 Craig Jacobson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/
/**
 * @file bdd.h
 * @author Craig Jacobson
 * @brief Simple unit testing header library.
 *
 * CHANGES FROM ORIGINAL:
 * - Removes some dependencies and Windows support.
 * - Wrapped malloc, calloc, realloc for logging and to simplify code.
 *
 * TODO:
 * - Figure out if function pointers or switches can be used to reduce
 *   test runtime complexity.
 * - Erradicate flattening stage.
 *
 * ANALYSIS OF TESTING COMPLEXITY:
 * If you have n test cases and m setup/teardown steps,
 * then testing runtime/execution is bound to O(n*m) or O(n**2).
 *
 * ANALYSIS OF ORIGINAL:
 * The original code is O(n*m*(n+m)) or O(n**3) in runtime/execution complexity.
 * Granted, each step is looked at, not necessarily executed.
 * This arises because the function is executed and each stage is looked at
 * until the one being looked for is found and executed.
 * With a language designed for testing, complexity should be O(n*m) or O(n**2).
 * Furthermore, because the testing stages are flattened the memory cost is
 * O(n*m) for storage.
 *
 * ANALYSIS OF THIS CODE:
 * Our goal is to at least erradicate the O(n*m) worst case for storage.
 * This will decrease runtime for larger test suites.
 * We do this by erradicating the flattening step,
 * storing the test structure only as a tree.
 * Furthermore, we will slab allocate nodes;
 * this gives better locality, bookkeeping savings, and fast cleanup.
 * Storage and setup should then be O(n+m);
 * one pass for setup and one node per test item.
 * Since tests are written by humans, test tree depth shouldn't get too deep;
 * this implies that a recursive solution would be appropriate.
 */
#ifndef __BDD_H__
#define __BDD_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>


#ifndef BDD_USE_COLOR
#define BDD_USE_COLOR 1
#endif

#ifndef BDD_USE_TAP
#define BDD_USE_TAP 0
#endif

#define __BDD_COLOR_RESET__       "\x1B[0m"
#define __BDD_COLOR_RED__         "\x1B[31m"
#define __BDD_COLOR_GREEN__       "\x1B[32m"
#define __BDD_COLOR_BOLD_WHITE__  "\x1B[1m"
#define __BDD_COLOR_MAGENTA__     "\x1B[35m"


/** @return True if terminal; false otherwise. */
bool
__bdd_isatty__(void)
{
    struct termios term;
    return 0 == tcgetattr(fileno(stdout), &term);
}

/** Allocate memory: malloc */
void *
__bdd_mem__(size_t size)
{
    void *mem = malloc(size);
    if (!mem)
    {
        perror("BDD malloc()");
        abort();
    }
    return mem;
}

/** Zero memory: calloc */
void *
__bdd_zmem__(size_t num, size_t size)
{
    void * mem = calloc(num, size);
    if (!mem)
    {
        perror("BDD calloc()");
        abort();
    }
    return mem;
}

/** Extend memory: realloc */
void *
__bdd_xmem__(void *oldmem, size_t size)
{
    void *mem = realloc(oldmem, size);
    if (!mem)
    {
        perror("BDD realloc()");
        abort();
    }
    return mem;
}

/** Free memory: free */
void
__bdd_free__(void *mem)
{
    if (mem)
    {
        free(mem);
    }
}

typedef struct __bdd_array__
{
    void **values;
    size_t capacity;
    size_t size;
} __bdd_array__;

__bdd_array__ *
__bdd_array_new__()
{
    __bdd_array__ *arr = __bdd_mem__(sizeof(__bdd_array__));
    arr->capacity = 4;
    arr->size = 0;
    arr->values = __bdd_zmem__(arr->capacity, sizeof(void *));
    return arr;
}

void
__bdd_array_push__(__bdd_array__ *arr, void *item)
{
    if (arr->size == arr->capacity)
    {
        arr->capacity *= 2;
        arr->values =  __bdd_xmem__(arr->values, sizeof(void*) * arr->capacity);
    }
    arr->values[arr->size++] = item;
}

void *
__bdd_array_last__(__bdd_array__ *arr)
{
    return arr->size ? arr->values[arr->size - 1] : NULL;
}

void *
__bdd_array_pop__(__bdd_array__ *arr)
{
    if (arr->size == 0)
    {
        return NULL;
    }
    void *result = arr->values[arr->size - 1];
    --arr->size;
    return result;
}

void
__bdd_array_free__(__bdd_array__ *arr)
{
    __bdd_free__(arr->values);
    __bdd_free__(arr);
}

typedef enum __bdd_node_type__
{
    __BDD_NODE_GROUP__ = 1,
    __BDD_NODE_TEST__ = 2,
    __BDD_NODE_INTERIM__ = 3
} __bdd_node_type__;

typedef struct __bdd_test_step__
{
    size_t level;
    int id;
    char *name;
    __bdd_node_type__ type;
} __bdd_test_step__;

typedef struct __bdd_node__
{
    int id;
    int next_node_id;
    char *name;
    __bdd_node_type__ type;
    __bdd_array__ *list_before;
    __bdd_array__ *list_after;
    __bdd_array__ *list_before_each;
    __bdd_array__ *list_after_each;
    __bdd_array__ *list_children;
} __bdd_node__;

__bdd_test_step__ *
__bdd_test_step_new__(size_t level, __bdd_node__ *node)
{
    __bdd_test_step__ *step = __bdd_mem__(sizeof(__bdd_test_step__));
    step->id = node->id;
    step->level = level;
    step->type = node->type;
    step->name = node->name;
    return step;
}

/**
 * @note Node takes ownership of name.
 */
__bdd_node__ *
__bdd_node_new__(int id, char *name, __bdd_node_type__ type)
{
    __bdd_node__ *n = __bdd_mem__(sizeof(__bdd_node__));
    n->id = id;
    n->next_node_id = id + 1;
    n->name = name;
    n->type = type;
    n->list_before = __bdd_array_new__();
    n->list_after = __bdd_array_new__();
    n->list_before_each = __bdd_array_new__();
    n->list_after_each = __bdd_array_new__();
    n->list_children = __bdd_array_new__();
    return n;
}

bool
__bdd_node_is_leaf__(__bdd_node__ *node)
{
    return node->list_children->size == 0;
}

void
__bdd_node_flatten_internal__(
    size_t level,
    __bdd_node__ *node,
    __bdd_array__  *steps,
    __bdd_array__  *before_each_lists,
    __bdd_array__  *after_each_lists
)
{
    if (__bdd_node_is_leaf__(node))
    {

        for (size_t listIndex = 0; listIndex < before_each_lists->size; ++listIndex)
        {
            __bdd_array__ *list = before_each_lists->values[listIndex];
            for (size_t i = 0; i < list->size; ++i)
            {
                __bdd_array_push__(steps, __bdd_test_step_new__(level, list->values[i]));
            }
        }

        __bdd_array_push__(steps, __bdd_test_step_new__(level, node));

        for (size_t listIndex = 0; listIndex < after_each_lists->size; ++listIndex)
        {
            size_t reverseListIndex = after_each_lists->size - listIndex - 1;
            __bdd_array__ *list = after_each_lists->values[reverseListIndex];
            for (size_t i = 0; i < list->size; ++i)
            {
                __bdd_array_push__(steps, __bdd_test_step_new__(level, list->values[i]));
            }
        }
        return;
    }

    __bdd_array_push__(steps, __bdd_test_step_new__(level, node));

    for (size_t i = 0; i < node->list_before->size; ++i)
    {
        __bdd_array_push__(steps, __bdd_test_step_new__(level + 1, node->list_before->values[i]));
    }

    __bdd_array_push__(before_each_lists, node->list_before_each);
    __bdd_array_push__(after_each_lists, node->list_after_each);

    for (size_t i = 0; i < node->list_children->size; ++i)
    {
        __bdd_node_flatten_internal__(level + 1, node->list_children->values[i], steps, before_each_lists, after_each_lists);
    }

    __bdd_array_pop__(before_each_lists);
    __bdd_array_pop__(after_each_lists);

    for (size_t i = 0; i < node->list_after->size; ++i)
    {
        __bdd_array_push__(steps, __bdd_test_step_new__(level + 1, node->list_after->values[i]));
    }
}

void
__bdd_node_flatten__(__bdd_node__ *node, __bdd_array__ *steps)
{
    if (node)
    {
        __bdd_array__ *before_each_lists = __bdd_array_new__();
        __bdd_array__ *after_each_lists = __bdd_array_new__();
        __bdd_node_flatten_internal__(0, node, steps, before_each_lists, after_each_lists);
        __bdd_array_free__(before_each_lists);
        __bdd_array_free__(after_each_lists);
    }
}

void
__bdd_node_free__(__bdd_node__ *n)
{
    __bdd_free__(n->name);
    __bdd_array_free__(n->list_before);
    __bdd_array_free__(n->list_after);
    __bdd_array_free__(n->list_before_each);
    __bdd_array_free__(n->list_after_each);
    __bdd_array_free__(n->list_children);
    __bdd_free__(n);
}

enum __bdd_run_type__
{
    __BDD_INIT_RUN__ = 1,
    __BDD_TEST_RUN__ = 2
};

typedef struct __bdd_config_type__
{
    enum __bdd_run_type__ run;
    int id;
    size_t test_index;
    size_t test_tap_index;
    size_t failed_test_count;
    __bdd_test_step__ *current_test;
    __bdd_array__ *node_stack;
    __bdd_array__ *nodes;
    char *error;
    char *location;
    bool use_color;
    bool use_tap;
} __bdd_config_type__;

char *__bdd_spec_name__;
void __bdd_test_main__(__bdd_config_type__ *__bdd_config__);
char *__bdd_vformat__(const char *format, va_list va);

void
__bdd_indent__(FILE *fp, size_t level)
{
    for (size_t i = 0; i < level; ++i)
    {
        fprintf(fp, "  ");
    }
}

bool
__bdd_enter_node__(
    __bdd_config_type__ *config,
    __bdd_node_type__ type,
    ptrdiff_t list_offset,
    const char *fmt,
    ...
)
{
    va_list va;
    va_start(va, fmt);
    char *name = __bdd_vformat__(fmt, va);
    va_end(va);

    if (config->run == __BDD_INIT_RUN__)
    {
        __bdd_node__ *top = __bdd_array_last__(config->node_stack);
        __bdd_array__ *list = *(__bdd_array__ **)((unsigned char *)top + list_offset);

        int id = config->id++;
        __bdd_node__ *node = __bdd_node_new__(id, name, type);
        __bdd_array_push__(list, node);
        __bdd_array_push__(config->nodes, node);
        if (type == __BDD_NODE_GROUP__)
        {
            __bdd_array_push__(config->node_stack, node);
            return true;
        }
        return false;
    }

    if (config->id >= (int)config->nodes->size)
    {
        fprintf(stderr, "non-deterministic spec\n");
        abort();
    }
    __bdd_node__ *node = config->nodes->values[config->id];
    if (node->type != type || strcmp(node->name, name) != 0)
    {
        fprintf(stderr, "non-deterministic spec\n");
        abort();
    }
    __bdd_free__(name);

    __bdd_test_step__ *step = config->current_test;
    bool should_enter = step->id >= node->id && step->id < node->next_node_id;
    if (should_enter)
    {
        __bdd_array_push__(config->node_stack, node);
        config->id++;
    }
    else
    {
        config->id = node->next_node_id;
    }
#if defined(BDD_PRINT_TRACE)
    const char *color = config->use_color ? __BDD_COLOR_MAGENTA__ : "";
    fprintf(stderr, "%s% 3d ", color, step->id);
    __bdd_indent__(stderr, config->node_stack->size - 1 - (int)should_enter);
    const char *reset = config->use_color ? __BDD_COLOR_RESET__ : "";
    fprintf(stderr,
        "%s [%d, %d) %s%s\n",
        should_enter ? ">" : "|",
        node->id,
        node->next_node_id,
        node->name,
        reset);
#endif
    return should_enter;
}

void
__bdd_exit_node__(__bdd_config_type__ *config)
{
    __bdd_node__ *top = __bdd_array_pop__(config->node_stack);
    if (config->run == __BDD_INIT_RUN__)
    {
        top->next_node_id = config->id;
    }
}

void
__bdd_run__(__bdd_config_type__ *config)
{
    __bdd_test_step__ *step = config->current_test;

    if (step->type == __BDD_NODE_GROUP__ && !config->use_tap)
    {
        __bdd_indent__(stdout, step->level);
        printf(
            "%s%s%s\n",
            config->use_color ? __BDD_COLOR_BOLD_WHITE__ : "",
            step->name,
            config->use_color ? __BDD_COLOR_RESET__ : ""
        );
        return;
    }

    __bdd_test_main__(config);

    if (step->type != __BDD_NODE_TEST__)
    {
        return;
    }

    ++config->test_tap_index;

    if (config->error == NULL)
    {
        if (config->run == __BDD_TEST_RUN__)
        {
            if (config->use_tap)
            {
                // We only to report tests and not setup / teardown success
                if (config->test_tap_index)
                {
                    printf("ok %zu - %s\n", config->test_tap_index, step->name);
                }
            }
            else
            {
                __bdd_indent__(stdout, step->level);
                printf(
                    "%s %s(OK)%s\n", step->name,
                    config->use_color ? __BDD_COLOR_GREEN__ : "",
                    config->use_color ? __BDD_COLOR_RESET__ : ""
                );
            }
        }
    }
    else
    {
        ++config->failed_test_count;
        if (config->use_tap)
        {
            // We only to report tests and not setup / teardown errors
            if (config->test_tap_index)
            {
                printf("not ok %zu - %s\n", config->test_tap_index, step->name);
            }
        }
        else
        {
            __bdd_indent__(stdout, step->level);
            printf(
                "%s %s(FAIL)%s\n", step->name,
                config->use_color ? __BDD_COLOR_RED__ : "",
                config->use_color ? __BDD_COLOR_RESET__ : ""
            );
            __bdd_indent__(stdout, step->level + 1);
            printf("%s\n", config->error);
            __bdd_indent__(stdout, step->level + 2);
            printf("%s\n", config->location);
        }
        __bdd_free__(config->error);
        config->error = NULL;
    }
}

char *
__bdd_vformat__(const char *format, va_list va)
{
    // TODO allow to be more dynamic... see my string varg functions
    // First we over-allocate
    const size_t size = 2048;
    char *result = __bdd_zmem__(size, sizeof(char));
    vsnprintf(result, size - 1, format, va);

    // Then clip to an actual size
    result = __bdd_xmem__(result, strlen(result) + 1);
    return result;
}

char *
__bdd_format__(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    char *result = __bdd_vformat__(format, va);
    va_end(va);
    return result;
}

bool
__bdd_is_supported_term__()
{
    const char *term = getenv("TERM");
    return term && strcmp(term, "") != 0;
}

int
main(void)
{
    struct __bdd_config_type__ config =
    {
        .run = __BDD_INIT_RUN__,
        .id = 0,
        .test_index = 0,
        .test_tap_index = 0,
        .failed_test_count = 0,
        .node_stack = __bdd_array_new__(),
        .nodes = __bdd_array_new__(),
        .error = NULL,
        .use_color = 0,
        .use_tap = 0
    };

    const char *tap_env = getenv("BDD_USE_TAP");
    if (BDD_USE_TAP || (tap_env && strcmp(tap_env, "") != 0 && strcmp(tap_env, "0") != 0))
    {
        config.use_tap = 1;
    }

    if (!config.use_tap && BDD_USE_COLOR && __bdd_isatty__() && __bdd_is_supported_term__())
    {
        config.use_color = 1;
    }

    __bdd_node__ *root = __bdd_node_new__(-1, __bdd_spec_name__, __BDD_NODE_GROUP__);
    __bdd_array_push__(config.node_stack, root);

    // During the first run we just gather the
    // count of the tests and their descriptions
    __bdd_test_main__(&config);

    // Gather steps. Note that 'id' should be called 'node_id'
    __bdd_array__ *steps = __bdd_array_new__();
    __bdd_node_flatten__(root, steps);

    // Count actual tests.
    size_t test_count = 0;
    for (size_t i = 0; i < steps->size; ++i)
    {
        __bdd_test_step__ *step = steps->values[i];
        if(step->type == __BDD_NODE_TEST__)
        {
            ++test_count;
        }
    }

    // Outputting the name of the suite
    if (config.use_tap)
    {
        printf("TAP version 13\n1..%zu\n", test_count);
    }

    config.run = __BDD_TEST_RUN__;

    for (size_t i = 0; i < steps->size; ++i)
    {
        __bdd_test_step__ *step = steps->values[i];
        config.node_stack->size = 1;
        config.id = 0;
        config.current_test = step;
        __bdd_run__(&config);
    }

    for (size_t i = 0; i < config.nodes->size; ++i)
    {
        __bdd_node_free__(config.nodes->values[i]);
    }
    root->name = NULL; // name is statically allocated
    __bdd_node_free__(root);
    for (size_t i = 0; i < steps->size; ++i)
    {
        __bdd_free__(steps->values[i]);
    }
    __bdd_array_free__(config.nodes);
    __bdd_array_free__(config.node_stack);
    __bdd_array_free__(steps);

    if (config.failed_test_count > 0)
    {
        if (!config.use_tap)
        {
            printf(
                "\n%zu test%s run, %zu failed.\n",
                test_count, test_count == 1 ? "" : "s", config.failed_test_count
            );
        }
        return 1;
    }
    return 0;
}

#define spec(name) \
char *__bdd_spec_name__ = (name);\
void __bdd_test_main__ (__bdd_config_type__ *__bdd_config__)\

#define __BDD_NODE__(node_list, type, ...)\
for(\
    bool __bdd_has_run__ = 0;\
    (\
      !__bdd_has_run__ && \
      __bdd_enter_node__(__bdd_config__, (type), offsetof(struct __bdd_node__, node_list), __VA_ARGS__) \
    );\
    __bdd_exit_node__(__bdd_config__), \
    __bdd_has_run__ = 1 \
)

#define describe(...) __BDD_NODE__(list_children, __BDD_NODE_GROUP__, __VA_ARGS__)
#define it(...)       __BDD_NODE__(list_children, __BDD_NODE_TEST__, __VA_ARGS__)
#define xit(...) if(0)
#define before_each() __BDD_NODE__(list_before_each, __BDD_NODE_INTERIM__, "before_each")
#define after_each()  __BDD_NODE__(list_after_each, __BDD_NODE_INTERIM__, "after_each")
#define before()      __BDD_NODE__(list_before, __BDD_NODE_INTERIM__, "before")
#define after()       __BDD_NODE__(list_after, __BDD_NODE_INTERIM__, "after")

#ifndef BDD_NO_CONTEXT_KEYWORD
#define context(name) describe(name)
#endif

#define __BDD_MACRO__(M, ...) __BDD_OVERLOAD__(M, __BDD_COUNT_ARGS__(__VA_ARGS__)) (__VA_ARGS__)
#define __BDD_OVERLOAD__(macro_name, suffix) __BDD_EXPAND_OVERLOAD__(macro_name, suffix)
#define __BDD_EXPAND_OVERLOAD__(macro_name, suffix) macro_name##suffix

#define __BDD_COUNT_ARGS__(...) __BDD_PATTERN_MATCH__(__VA_ARGS__,_,_,_,_,_,_,_,_,_,ONE__)
#define __BDD_PATTERN_MATCH__(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N, ...) N

#define __BDD_STRING_HELPER__(x) #x
#define __BDD_STRING__(x) __BDD_STRING_HELPER__(x)
#define __STRING__LINE__ __BDD_STRING__(__LINE__)

#define __BDD_FMT_COLOR__ __BDD_COLOR_RED__ "Check failed:" __BDD_COLOR_RESET__ " %s"
#define __BDD_FMT_PLAIN__ "Check failed: %s"

#define __BDD_CHECK__(condition, ...) if (!(condition))\
{\
    char *message = __bdd_format__(__VA_ARGS__);\
    const char *fmt = __bdd_config__->use_color ? __BDD_FMT_COLOR__ : __BDD_FMT_PLAIN__;\
    __bdd_config__->location = "at " __FILE__ ":" __STRING__LINE__;\
    size_t bufflen = strlen(fmt) + strlen(message) + 1;\
    __bdd_config__->error = __bdd_zmem__(bufflen, sizeof(char));\
    if (__bdd_config__->use_color)\
    {\
        snprintf(__bdd_config__->error, bufflen, __BDD_FMT_COLOR__, message);\
    }\
    else\
    {\
        snprintf(__bdd_config__->error, bufflen, __BDD_FMT_PLAIN__, message);\
    }\
    __bdd_free__(message);\
    return;\
}

#define __BDD_CHECK_ONE__(condition) __BDD_CHECK__(condition, #condition)

#define check(...) __BDD_MACRO__(__BDD_CHECK_, __VA_ARGS__)


#ifdef __cplusplus
}
#endif
#endif /* __BDD_H__ */

