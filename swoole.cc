/*
  +----------------------------------------------------------------------+
  | Swoole                                                               |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.0 of the Apache license,    |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.apache.org/licenses/LICENSE-2.0.html                      |
  | If you did not receive a copy of the Apache2.0 license and are unable|
  | to obtain it through the world-wide-web, please send a note to       |
  | license@swoole.com so we can mail you a copy immediately.            |
  +----------------------------------------------------------------------+
  | Author: Tianfeng Han  <mikan.tenny@gmail.com>                        |
  +----------------------------------------------------------------------+
*/
#include "php_swoole_cxx.h"
#include "php_swoole_library.h"

#if (HAVE_PCRE || HAVE_BUNDLED_PCRE) && !defined(COMPILE_DL_PCRE)
#include "ext/pcre/php_pcre.h"
#endif

#include "mime_types.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>

ZEND_DECLARE_MODULE_GLOBALS(swoole)

extern sapi_module_struct sapi_module;

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_void, 0, 0, 0)
ZEND_END_ARG_INFO()

//arginfo event
ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_event_add, 0, 0, 2)
    ZEND_ARG_INFO(0, fd)
    ZEND_ARG_CALLABLE_INFO(0, read_callback, 1)
    ZEND_ARG_CALLABLE_INFO(0, write_callback, 1)
    ZEND_ARG_INFO(0, events)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_event_set, 0, 0, 1)
    ZEND_ARG_INFO(0, fd)
    ZEND_ARG_CALLABLE_INFO(0, read_callback, 1)
    ZEND_ARG_CALLABLE_INFO(0, write_callback, 1)
    ZEND_ARG_INFO(0, events)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_event_write, 0, 0, 2)
    ZEND_ARG_INFO(0, fd)
    ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_event_defer, 0, 0, 1)
    ZEND_ARG_CALLABLE_INFO(0, callback, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_event_cycle, 0, 0, 1)
    ZEND_ARG_CALLABLE_INFO(0, callback, 1)
    ZEND_ARG_INFO(0, before)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_event_del, 0, 0, 1)
    ZEND_ARG_INFO(0, fd)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_event_isset, 0, 0, 1)
    ZEND_ARG_INFO(0, fd)
    ZEND_ARG_INFO(0, events)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_async_set, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, settings, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_async_dns_lookup_coro, 0, 0, 1)
    ZEND_ARG_INFO(0, domain_name)
    ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_coroutine_create, 0, 0, 1)
    ZEND_ARG_CALLABLE_INFO(0, func, 0)
    ZEND_ARG_VARIADIC_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_coroutine_defer, 0, 0, 1)
    ZEND_ARG_CALLABLE_INFO(0, callback, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_coroutine_exec, 0, 0, 1)
    ZEND_ARG_INFO(0, command)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_client_select, 0, 0, 3)
    ZEND_ARG_INFO(1, read_array)
    ZEND_ARG_INFO(1, write_array)
    ZEND_ARG_INFO(1, error_array)
    ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_set_process_name, 0, 0, 1)
    ZEND_ARG_INFO(0, process_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_strerror, 0, 0, 1)
    ZEND_ARG_INFO(0, errno)
    ZEND_ARG_INFO(0, error_type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_hashcode, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_get_mime_type, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

//arginfo end

#include "zend_exceptions.h"

static PHP_FUNCTION(swoole_get_mime_type);
static PHP_FUNCTION(swoole_hashcode);

const zend_function_entry swoole_functions[] =
{
    PHP_FE(swoole_version, arginfo_swoole_void)
    PHP_FE(swoole_cpu_num, arginfo_swoole_void)
    PHP_FE(swoole_last_error, arginfo_swoole_void)
    /*------swoole_event-----*/
    PHP_FE(swoole_event_add, arginfo_swoole_event_add)
    PHP_FE(swoole_event_set, arginfo_swoole_event_set)
    PHP_FE(swoole_event_del, arginfo_swoole_event_del)
    PHP_FE(swoole_event_exit, arginfo_swoole_void)
    PHP_FE(swoole_event_wait, arginfo_swoole_void)
    PHP_FE(swoole_event_write, arginfo_swoole_event_write)
    PHP_FE(swoole_event_defer, arginfo_swoole_event_defer)
    PHP_FE(swoole_event_cycle, arginfo_swoole_event_cycle)
    PHP_FE(swoole_event_dispatch, arginfo_swoole_void)
    PHP_FE(swoole_event_isset, arginfo_swoole_event_isset)
    /*------swoole_async_io------*/
    PHP_FE(swoole_async_dns_lookup_coro, arginfo_swoole_async_dns_lookup_coro)
    PHP_FE(swoole_async_set, arginfo_swoole_async_set)
    /*------swoole_coroutine------*/
    PHP_FE(swoole_coroutine_create, arginfo_swoole_coroutine_create)
    PHP_FE(swoole_coroutine_exec, arginfo_swoole_coroutine_exec)
    PHP_FE(swoole_coroutine_defer, arginfo_swoole_coroutine_defer)
    PHP_FALIAS(go, swoole_coroutine_create, arginfo_swoole_coroutine_create)
    PHP_FALIAS(defer, swoole_coroutine_defer, arginfo_swoole_coroutine_defer)
    /*------other-----*/
    PHP_FE(swoole_client_select, arginfo_swoole_client_select)
    PHP_FALIAS(swoole_select, swoole_client_select, arginfo_swoole_client_select)
    PHP_FE(swoole_set_process_name, arginfo_swoole_set_process_name)
    PHP_FE(swoole_get_local_ip, arginfo_swoole_void)
    PHP_FE(swoole_get_local_mac, arginfo_swoole_void)
    PHP_FE(swoole_strerror, arginfo_swoole_strerror)
    PHP_FE(swoole_errno, arginfo_swoole_void)
    PHP_FE(swoole_hashcode, arginfo_swoole_hashcode)
    PHP_FE(swoole_get_mime_type, arginfo_swoole_get_mime_type)
    PHP_FE(swoole_clear_dns_cache, arginfo_swoole_void)
    PHP_FE(swoole_internal_call_user_shutdown_begin, arginfo_swoole_void)
    PHP_FE_END /* Must be the last line in swoole_functions[] */
};

static const zend_function_entry swoole_event_methods[] =
{
    ZEND_FENTRY(add, ZEND_FN(swoole_event_add), arginfo_swoole_event_add, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_FENTRY(del, ZEND_FN(swoole_event_del), arginfo_swoole_event_del, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_FENTRY(set, ZEND_FN(swoole_event_set), arginfo_swoole_event_set, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_FENTRY(exit, ZEND_FN(swoole_event_exit), arginfo_swoole_void, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_FENTRY(write, ZEND_FN(swoole_event_write), arginfo_swoole_event_write, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_FENTRY(wait, ZEND_FN(swoole_event_wait), arginfo_swoole_void, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_FENTRY(defer, ZEND_FN(swoole_event_defer), arginfo_swoole_event_defer, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_FENTRY(cycle, ZEND_FN(swoole_event_cycle), arginfo_swoole_event_cycle, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_FENTRY(dispatch, ZEND_FN(swoole_event_dispatch), arginfo_swoole_void, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_FENTRY(isset, ZEND_FN(swoole_event_isset), arginfo_swoole_event_isset, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

#if PHP_MEMORY_DEBUG
php_vmstat_t php_vmstat;
#endif

static zend_class_entry *swoole_event_ce;
static zend_object_handlers swoole_event_handlers;

zend_class_entry *swoole_exception_ce;
zend_object_handlers swoole_exception_handlers;

zend_module_entry swoole_module_entry =
{
#if ZEND_MODULE_API_NO >= 20050922
    STANDARD_MODULE_HEADER_EX,
    NULL,
    NULL,
#else
    STANDARD_MODULE_HEADER,
#endif
    "swoole",
    swoole_functions,
    PHP_MINIT(swoole),
    PHP_MSHUTDOWN(swoole),
    PHP_RINIT(swoole),     //RINIT
    PHP_RSHUTDOWN(swoole), //RSHUTDOWN
    PHP_MINFO(swoole),
    PHP_SWOOLE_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SWOOLE
ZEND_GET_MODULE(swoole)
#endif

/* {{{ PHP_INI
 */

PHP_INI_BEGIN()
/**
 * enable swoole coroutine
 */
STD_ZEND_INI_BOOLEAN("swoole.enable_coroutine", "On", PHP_INI_ALL, OnUpdateBool, enable_coroutine, zend_swoole_globals, swoole_globals)
STD_ZEND_INI_BOOLEAN("swoole.enable_library", "On", PHP_INI_ALL, OnUpdateBool, enable_library, zend_swoole_globals, swoole_globals)
/**
 * enable swoole coroutine epreemptive scheduler
 */
STD_ZEND_INI_BOOLEAN("swoole.enable_preemptive_scheduler", "Off", PHP_INI_ALL, OnUpdateBool, enable_preemptive_scheduler, zend_swoole_globals, swoole_globals)
/**
 * display error
 */
STD_ZEND_INI_BOOLEAN("swoole.display_errors", "On", PHP_INI_ALL, OnUpdateBool, display_errors, zend_swoole_globals, swoole_globals)
/**
 * use short class name
 */
STD_ZEND_INI_BOOLEAN("swoole.use_shortname", "On", PHP_INI_SYSTEM, OnUpdateBool, use_shortname, zend_swoole_globals, swoole_globals)
/**
 * unix socket buffer size
 */
STD_PHP_INI_ENTRY("swoole.unixsock_buffer_size", ZEND_TOSTR(SW_SOCKET_BUFFER_SIZE), PHP_INI_ALL, OnUpdateLong, socket_buffer_size, zend_swoole_globals, swoole_globals)
PHP_INI_END()

static void php_swoole_init_globals(zend_swoole_globals *swoole_globals)
{
    swoole_globals->enable_coroutine = 1;
    swoole_globals->enable_library = 1;
    swoole_globals->enable_preemptive_scheduler = 0;
    swoole_globals->socket_buffer_size = SW_SOCKET_BUFFER_SIZE;
    swoole_globals->display_errors = 1;
    swoole_globals->use_shortname = 1;
    swoole_globals->rshutdown_functions = NULL;
}

ssize_t php_swoole_length_func(swProtocol *protocol, swConnection *conn, char *data, uint32_t length)
{
    SwooleG.lock.lock(&SwooleG.lock);

    zval *callback = (zval *) protocol->private_data;
    zval args[1];
    zval *retval = NULL;
    int ret;

    ZVAL_STRINGL(&args[0], data, length);

    ret = sw_call_user_function_ex(EG(function_table), NULL, callback, &retval, 1, args, 0, NULL);
    zval_ptr_dtor(&args[0]);
    if (ret == FAILURE)
    {
        swoole_php_fatal_error(E_WARNING, "length function handler error");
        goto _error;
    }
    if (retval)
    {
        ssize_t length = zval_get_long(retval);
        zval_ptr_dtor(retval);
        SwooleG.lock.unlock(&SwooleG.lock);
        return length;
    }
    _error:
    SwooleG.lock.unlock(&SwooleG.lock);
    return -1;
}

int php_swoole_dispatch_func(swServer *serv, swConnection *conn, swSendData *data)
{
    zend_fcall_info_cache *fci_cache = (zend_fcall_info_cache*) serv->private_data_3;
    zval args[4];
    zval *zserver = &args[0], *zfd = &args[1], *ztype = &args[2], *zdata = NULL;
    zval _retval, *retval = &_retval;
    int worker_id = -1;

    SwooleG.lock.lock(&SwooleG.lock);
    *zserver = *((zval *) serv->ptr2);
    ZVAL_LONG(zfd, (zend_long) (conn ? conn->session_id : data->info.fd));
    ZVAL_LONG(ztype, (zend_long) data->info.type);
    if (sw_zend_function_max_num_args(fci_cache->function_handler) > 3)
    {
        // optimization: reduce memory copy
        zdata = &args[3];
        ZVAL_STRINGL(zdata, data->data, data->info.len > SW_IPC_BUFFER_SIZE ? SW_IPC_BUFFER_SIZE : data->info.len);
    }
    if (sw_call_user_function_fast_ex(NULL, fci_cache, zdata ? 4 : 3, args, retval) == FAILURE)
    {
        swoole_php_fatal_error(E_WARNING, "dispatch function handler error");
    }
    else if (!ZVAL_IS_NULL(retval))
    {
        worker_id = (int) zval_get_long(retval);
        if (worker_id >= serv->worker_num)
        {
            swoole_php_fatal_error(E_WARNING, "invalid target worker-id[%d]", worker_id);
            worker_id = -1;
        }
        zval_ptr_dtor(retval);
    }
    if (zdata)
    {
        zval_ptr_dtor(zdata);
    }
    SwooleG.lock.unlock(&SwooleG.lock);
    return worker_id;
}

static sw_inline uint32_t swoole_get_new_size(uint32_t old_size, int handle)
{
    uint32_t new_size = old_size * 2;
    if (handle > SWOOLE_OBJECT_MAX)
    {
        swoole_php_fatal_error(E_ERROR, "handle %d exceed %d", handle, SWOOLE_OBJECT_MAX);
        return 0;
    }
    while (new_size <= (uint32_t) handle)
    {
        new_size *= 2;
    }
    if (new_size > SWOOLE_OBJECT_MAX)
    {
        new_size = SWOOLE_OBJECT_MAX;
    }
    return new_size;
}

void swoole_set_object_by_handle(uint32_t handle, void *ptr)
{
    assert(handle < SWOOLE_OBJECT_MAX);

    if (unlikely(handle >= swoole_objects.size))
    {
        uint32_t old_size = swoole_objects.size;
        uint32_t new_size = swoole_get_new_size(old_size, handle);

        void *old_ptr = swoole_objects.array;
        void *new_ptr = NULL;

        new_ptr = sw_realloc(old_ptr, sizeof(void*) * new_size);
        if (!new_ptr)
        {
            swoole_php_fatal_error(E_ERROR, "malloc(%d) failed", (int )(new_size * sizeof(void *)));
            return;
        }
        bzero((char*) new_ptr + (old_size * sizeof(void*)), (new_size - old_size) * sizeof(void*));
        swoole_objects.array = (void**) new_ptr;
        swoole_objects.size = new_size;
    }
#ifdef ZEND_DEBUG
    else if (ptr)
    {
        assert(swoole_objects.array[handle] == NULL);
    }
#endif
    swoole_objects.array[handle] = ptr;
}

void swoole_set_property_by_handle(uint32_t handle, int property_id, void *ptr)
{
    assert(handle < SWOOLE_OBJECT_MAX);

    if (unlikely(handle >= swoole_objects.property_size[property_id]))
    {
        uint32_t old_size = swoole_objects.property_size[property_id];
        uint32_t new_size = 0;

        void **old_ptr = NULL;
        void **new_ptr = NULL;

        if (old_size == 0)
        {
            new_size = handle < SWOOLE_OBJECT_DEFAULT ? SWOOLE_OBJECT_DEFAULT : swoole_get_new_size(SWOOLE_OBJECT_DEFAULT, handle);
            new_ptr = (void **) sw_calloc(new_size, sizeof(void *));
        }
        else
        {
            new_size = swoole_get_new_size(old_size, handle);
            old_ptr = swoole_objects.property[property_id];
            new_ptr = (void **) sw_realloc(old_ptr, new_size * sizeof(void *));
        }
        if (new_ptr == NULL)
        {
            swoole_php_fatal_error(E_ERROR, "malloc(%d) failed", (int )(new_size * sizeof(void *)));
            return;
        }
        if (old_size > 0)
        {
            bzero((char *) new_ptr + old_size * sizeof(void*), (new_size - old_size) * sizeof(void*));
        }
        swoole_objects.property_size[property_id] = new_size;
        swoole_objects.property[property_id] = new_ptr;
    }
#ifdef ZEND_DEBUG
    else if (ptr)
    {
        assert(swoole_objects.property[property_id][handle] == NULL);
    }
#endif
    swoole_objects.property[property_id][handle] = ptr;
}

int swoole_register_rshutdown_function(swCallback func, int push_back)
{
    if (SWOOLE_G(rshutdown_functions) == NULL)
    {
        SWOOLE_G(rshutdown_functions) = swLinkedList_new(0, NULL);
        if (SWOOLE_G(rshutdown_functions) == NULL)
        {
            return SW_ERR;
        }
    }
    if (push_back)
    {
        return swLinkedList_append(SWOOLE_G(rshutdown_functions), (void*) func);
    }
    else
    {
        return swLinkedList_prepend(SWOOLE_G(rshutdown_functions), (void*) func);
    }
}

void php_swoole_register_shutdown_function(const char *function)
{
    php_shutdown_function_entry shutdown_function_entry;
    shutdown_function_entry.arg_count = 1;
    shutdown_function_entry.arguments = (zval *) safe_emalloc(sizeof(zval), 1, 0);
    ZVAL_STRING(&shutdown_function_entry.arguments[0], function);
    register_user_shutdown_function((char *) function, ZSTR_LEN(Z_STR(shutdown_function_entry.arguments[0])),
            &shutdown_function_entry);
}

static void php_swoole_old_shutdown_function_move(zval *zv)
{
    php_shutdown_function_entry *old_shutdown_function_entry = (php_shutdown_function_entry *) Z_PTR_P(zv);
    zend_hash_next_index_insert_mem(BG(user_shutdown_function_names), old_shutdown_function_entry, sizeof(php_shutdown_function_entry));
    efree(old_shutdown_function_entry);
}

void php_swoole_register_shutdown_function_prepend(const char *function)
{
    HashTable *old_user_shutdown_function_names = BG(user_shutdown_function_names);
    if (!old_user_shutdown_function_names)
    {
        php_swoole_register_shutdown_function(function);
        return;
    }
    BG(user_shutdown_function_names) = NULL;
    php_swoole_register_shutdown_function(function);
    old_user_shutdown_function_names->pDestructor = php_swoole_old_shutdown_function_move;
    zend_hash_destroy(old_user_shutdown_function_names);
    FREE_HASHTABLE(old_user_shutdown_function_names);
}

static void php_swoole_fatal_error(int code, const char *format, ...)
{
    va_list args;
    swString *buffer = SwooleTG.buffer_stack;
    zend_string *backtrace;
    const char *space, *class_name = get_active_class_name(&space);

    SwooleGS->lock_2.lock(&SwooleGS->lock_2);
    swString_clear(buffer);
    buffer->length += sw_snprintf(buffer->str, buffer->size, "(PHP Fatal Error: %d):\n%s%s%s: ", code, class_name, space, get_active_function_name());
    va_start(args, format);
    buffer->length += sw_vsnprintf(buffer->str + buffer->length, buffer->size - buffer->length, format, args);
    va_end(args);
    swString_append_ptr(buffer, SW_STRL("\nStack trace:\n"));
    backtrace = sw_get_debug_print_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS, 0);
    if (likely(backtrace))
    {
        swString_append_ptr(buffer, ZSTR_VAL(backtrace), ZSTR_LEN(backtrace));
        zend_string_release(backtrace);
    }
    SwooleG.write_log(SW_LOG_ERROR, buffer->str, buffer->length);
    SwooleGS->lock_2.unlock(&SwooleGS->lock_2);
    exit(255);
}

void swoole_call_rshutdown_function(void *arg)
{
    if (SWOOLE_G(rshutdown_functions))
    {
        swLinkedList *rshutdown_functions = SWOOLE_G(rshutdown_functions);
        swLinkedList_node *node = rshutdown_functions->head;
        swCallback func = NULL;

        while (node)
        {
            func = (swCallback) node->data;
            func(arg);
            node = node->next;
        }
    }
}

swoole_object_array swoole_objects;

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(swoole)
{
    ZEND_INIT_MODULE_GLOBALS(swoole, php_swoole_init_globals, NULL);
    REGISTER_INI_ENTRIES();

    SW_REGISTER_STRING_CONSTANT("SWOOLE_VERSION", SWOOLE_VERSION);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_VERSION_ID", SWOOLE_VERSION_ID);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_MAJOR_VERSION", SWOOLE_MAJOR_VERSION);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_MINOR_VERSION", SWOOLE_MINOR_VERSION);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_RELEASE_VERSION", SWOOLE_RELEASE_VERSION);
    SW_REGISTER_STRING_CONSTANT("SWOOLE_EXTRA_VERSION", SWOOLE_EXTRA_VERSION);
#ifndef SW_DEBUG
    SW_REGISTER_BOOL_CONSTANT("SWOOLE_DEBUG", 0);
#else
    SW_REGISTER_BOOL_CONSTANT("SWOOLE_DEBUG", 1);
#endif

    /**
     * mode type
     */
    SW_REGISTER_LONG_CONSTANT("SWOOLE_BASE", SW_MODE_BASE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_PROCESS", SW_MODE_PROCESS);

    /**
     * task ipc mode
     */
    SW_REGISTER_LONG_CONSTANT("SWOOLE_IPC_UNSOCK", SW_TASK_IPC_UNIXSOCK);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_IPC_MSGQUEUE", SW_TASK_IPC_MSGQUEUE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_IPC_PREEMPTIVE", SW_TASK_IPC_PREEMPTIVE);

    /**
     * socket type
     */
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SOCK_TCP", SW_SOCK_TCP);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SOCK_TCP6", SW_SOCK_TCP6);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SOCK_UDP", SW_SOCK_UDP);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SOCK_UDP6", SW_SOCK_UDP6);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SOCK_UNIX_DGRAM", SW_SOCK_UNIX_DGRAM);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SOCK_UNIX_STREAM", SW_SOCK_UNIX_STREAM);

    /**
     * simple socket type alias
     */
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TCP", SW_SOCK_TCP);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TCP6", SW_SOCK_TCP6);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_UDP", SW_SOCK_UDP);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_UDP6", SW_SOCK_UDP6);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_UNIX_DGRAM", SW_SOCK_UNIX_DGRAM);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_UNIX_STREAM", SW_SOCK_UNIX_STREAM);

    /**
     * simple api
     */
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SOCK_SYNC", SW_SOCK_SYNC);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SOCK_ASYNC", SW_SOCK_ASYNC);

    SW_REGISTER_LONG_CONSTANT("SWOOLE_SYNC", SW_FLAG_SYNC);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ASYNC", SW_FLAG_ASYNC);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_KEEP", SW_FLAG_KEEP);

#ifdef SW_USE_OPENSSL
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SSL", SW_SOCK_SSL);

    /**
     * SSL method
     */
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SSLv3_METHOD", SW_SSLv3_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SSLv3_SERVER_METHOD", SW_SSLv3_SERVER_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SSLv3_CLIENT_METHOD", SW_SSLv3_CLIENT_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SSLv23_METHOD", SW_SSLv23_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SSLv23_SERVER_METHOD", SW_SSLv23_SERVER_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_SSLv23_CLIENT_METHOD", SW_SSLv23_CLIENT_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TLSv1_METHOD", SW_TLSv1_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TLSv1_SERVER_METHOD", SW_TLSv1_SERVER_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TLSv1_CLIENT_METHOD", SW_TLSv1_CLIENT_METHOD);
#ifdef TLS1_1_VERSION
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TLSv1_1_METHOD", SW_TLSv1_1_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TLSv1_1_SERVER_METHOD", SW_TLSv1_1_SERVER_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TLSv1_1_CLIENT_METHOD", SW_TLSv1_1_CLIENT_METHOD);
#endif
#ifdef TLS1_2_VERSION
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TLSv1_2_METHOD", SW_TLSv1_2_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TLSv1_2_SERVER_METHOD", SW_TLSv1_2_SERVER_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TLSv1_2_CLIENT_METHOD", SW_TLSv1_2_CLIENT_METHOD);
#endif
    SW_REGISTER_LONG_CONSTANT("SWOOLE_DTLSv1_METHOD", SW_DTLSv1_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_DTLSv1_SERVER_METHOD", SW_DTLSv1_SERVER_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_DTLSv1_CLIENT_METHOD", SW_DTLSv1_CLIENT_METHOD);
#endif

    SW_REGISTER_LONG_CONSTANT("SWOOLE_EVENT_READ", SW_EVENT_READ);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_EVENT_WRITE", SW_EVENT_WRITE);

    /**
     * Register ERROR types
     */
    SW_REGISTER_LONG_CONSTANT("SWOOLE_STRERROR_SYSTEM", SW_STRERROR_SYSTEM);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_STRERROR_GAI", SW_STRERROR_GAI);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_STRERROR_DNS", SW_STRERROR_DNS);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_STRERROR_SWOOLE", SW_STRERROR_SWOOLE);

    /**
     * Register ERROR constants
     */
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_MALLOC_FAIL", SW_ERROR_MALLOC_FAIL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SYSTEM_CALL_FAIL", SW_ERROR_SYSTEM_CALL_FAIL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_PHP_FATAL_ERROR", SW_ERROR_PHP_FATAL_ERROR);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_NAME_TOO_LONG", SW_ERROR_NAME_TOO_LONG);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_INVALID_PARAMS", SW_ERROR_INVALID_PARAMS);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_QUEUE_FULL", SW_ERROR_QUEUE_FULL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_OPERATION_NOT_SUPPORT", SW_ERROR_OPERATION_NOT_SUPPORT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_FILE_NOT_EXIST", SW_ERROR_FILE_NOT_EXIST);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_FILE_TOO_LARGE", SW_ERROR_FILE_TOO_LARGE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_FILE_EMPTY", SW_ERROR_FILE_EMPTY);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_DNSLOOKUP_DUPLICATE_REQUEST", SW_ERROR_DNSLOOKUP_DUPLICATE_REQUEST);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_DNSLOOKUP_RESOLVE_FAILED", SW_ERROR_DNSLOOKUP_RESOLVE_FAILED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_DNSLOOKUP_RESOLVE_TIMEOUT", SW_ERROR_DNSLOOKUP_RESOLVE_TIMEOUT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_BAD_IPV6_ADDRESS", SW_ERROR_BAD_IPV6_ADDRESS);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_UNREGISTERED_SIGNAL", SW_ERROR_UNREGISTERED_SIGNAL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SESSION_CLOSED_BY_SERVER", SW_ERROR_SESSION_CLOSED_BY_SERVER);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SESSION_CLOSED_BY_CLIENT", SW_ERROR_SESSION_CLOSED_BY_CLIENT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SESSION_CLOSING", SW_ERROR_SESSION_CLOSING);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SESSION_CLOSED", SW_ERROR_SESSION_CLOSED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SESSION_NOT_EXIST", SW_ERROR_SESSION_NOT_EXIST);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SESSION_INVALID_ID", SW_ERROR_SESSION_INVALID_ID);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SESSION_DISCARD_TIMEOUT_DATA", SW_ERROR_SESSION_DISCARD_TIMEOUT_DATA);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_OUTPUT_BUFFER_OVERFLOW", SW_ERROR_OUTPUT_BUFFER_OVERFLOW);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SSL_NOT_READY", SW_ERROR_SSL_NOT_READY);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SSL_CANNOT_USE_SENFILE", SW_ERROR_SSL_CANNOT_USE_SENFILE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SSL_EMPTY_PEER_CERTIFICATE", SW_ERROR_SSL_EMPTY_PEER_CERTIFICATE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SSL_VEFIRY_FAILED", SW_ERROR_SSL_VEFIRY_FAILED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SSL_BAD_CLIENT", SW_ERROR_SSL_BAD_CLIENT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SSL_BAD_PROTOCOL", SW_ERROR_SSL_BAD_PROTOCOL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_PACKAGE_LENGTH_TOO_LARGE", SW_ERROR_PACKAGE_LENGTH_TOO_LARGE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_DATA_LENGTH_TOO_LARGE", SW_ERROR_DATA_LENGTH_TOO_LARGE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_TASK_PACKAGE_TOO_BIG", SW_ERROR_TASK_PACKAGE_TOO_BIG);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_TASK_DISPATCH_FAIL", SW_ERROR_TASK_DISPATCH_FAIL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_HTTP2_STREAM_ID_TOO_BIG", SW_ERROR_HTTP2_STREAM_ID_TOO_BIG);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_HTTP2_STREAM_NO_HEADER", SW_ERROR_HTTP2_STREAM_NO_HEADER);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_HTTP2_STREAM_NOT_FOUND", SW_ERROR_HTTP2_STREAM_NOT_FOUND);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_AIO_BAD_REQUEST", SW_ERROR_AIO_BAD_REQUEST);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_AIO_CANCELED", SW_ERROR_AIO_CANCELED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CLIENT_NO_CONNECTION", SW_ERROR_CLIENT_NO_CONNECTION);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SOCKET_CLOSED", SW_ERROR_SOCKET_CLOSED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SOCKS5_UNSUPPORT_VERSION", SW_ERROR_SOCKS5_UNSUPPORT_VERSION);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SOCKS5_UNSUPPORT_METHOD", SW_ERROR_SOCKS5_UNSUPPORT_METHOD);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SOCKS5_AUTH_FAILED", SW_ERROR_SOCKS5_AUTH_FAILED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SOCKS5_SERVER_ERROR", SW_ERROR_SOCKS5_SERVER_ERROR);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_HTTP_PROXY_HANDSHAKE_ERROR", SW_ERROR_HTTP_PROXY_HANDSHAKE_ERROR);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_HTTP_INVALID_PROTOCOL", SW_ERROR_HTTP_INVALID_PROTOCOL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_WEBSOCKET_BAD_CLIENT", SW_ERROR_WEBSOCKET_BAD_CLIENT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_WEBSOCKET_BAD_OPCODE", SW_ERROR_WEBSOCKET_BAD_OPCODE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_WEBSOCKET_UNCONNECTED", SW_ERROR_WEBSOCKET_UNCONNECTED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_WEBSOCKET_HANDSHAKE_FAILED", SW_ERROR_WEBSOCKET_HANDSHAKE_FAILED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SERVER_MUST_CREATED_BEFORE_CLIENT", SW_ERROR_SERVER_MUST_CREATED_BEFORE_CLIENT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SERVER_TOO_MANY_SOCKET", SW_ERROR_SERVER_TOO_MANY_SOCKET);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SERVER_WORKER_TERMINATED", SW_ERROR_SERVER_WORKER_TERMINATED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SERVER_INVALID_LISTEN_PORT", SW_ERROR_SERVER_INVALID_LISTEN_PORT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SERVER_TOO_MANY_LISTEN_PORT", SW_ERROR_SERVER_TOO_MANY_LISTEN_PORT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SERVER_PIPE_BUFFER_FULL", SW_ERROR_SERVER_PIPE_BUFFER_FULL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SERVER_NO_IDLE_WORKER", SW_ERROR_SERVER_NO_IDLE_WORKER);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SERVER_ONLY_START_ONE", SW_ERROR_SERVER_ONLY_START_ONE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SERVER_SEND_IN_MASTER", SW_ERROR_SERVER_SEND_IN_MASTER);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SERVER_INVALID_REQUEST", SW_ERROR_SERVER_INVALID_REQUEST);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SERVER_CONNECT_FAIL", SW_ERROR_SERVER_CONNECT_FAIL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_SERVER_WORKER_EXIT_TIMEOUT", SW_ERROR_SERVER_WORKER_EXIT_TIMEOUT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_OUT_OF_COROUTINE", SW_ERROR_CO_OUT_OF_COROUTINE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_HAS_BEEN_BOUND", SW_ERROR_CO_HAS_BEEN_BOUND);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_MUTEX_DOUBLE_UNLOCK", SW_ERROR_CO_MUTEX_DOUBLE_UNLOCK);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_BLOCK_OBJECT_LOCKED", SW_ERROR_CO_BLOCK_OBJECT_LOCKED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_BLOCK_OBJECT_WAITING", SW_ERROR_CO_BLOCK_OBJECT_WAITING);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_YIELD_FAILED", SW_ERROR_CO_YIELD_FAILED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_GETCONTEXT_FAILED", SW_ERROR_CO_GETCONTEXT_FAILED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_SWAPCONTEXT_FAILED", SW_ERROR_CO_SWAPCONTEXT_FAILED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_MAKECONTEXT_FAILED", SW_ERROR_CO_MAKECONTEXT_FAILED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_IOCPINIT_FAILED", SW_ERROR_CO_IOCPINIT_FAILED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_PROTECT_STACK_FAILED", SW_ERROR_CO_PROTECT_STACK_FAILED);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_STD_THREAD_LINK_ERROR", SW_ERROR_CO_STD_THREAD_LINK_ERROR);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_ERROR_CO_DISABLED_MULTI_THREAD", SW_ERROR_CO_DISABLED_MULTI_THREAD);

    /**
     * trace log
     */
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_SERVER", SW_TRACE_SERVER);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_CLIENT", SW_TRACE_CLIENT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_BUFFER", SW_TRACE_BUFFER);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_CONN", SW_TRACE_CONN);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_EVENT", SW_TRACE_EVENT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_WORKER", SW_TRACE_WORKER);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_REACTOR", SW_TRACE_REACTOR);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_PHP", SW_TRACE_PHP);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_HTTP", SW_TRACE_HTTP);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_HTTP2", SW_TRACE_HTTP2);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_EOF_PROTOCOL", SW_TRACE_EOF_PROTOCOL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_LENGTH_PROTOCOL", SW_TRACE_LENGTH_PROTOCOL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_CLOSE", SW_TRACE_CLOSE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_REDIS_CLIENT", SW_TRACE_REDIS_CLIENT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_MYSQL_CLIENT", SW_TRACE_MYSQL_CLIENT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_HTTP_CLIENT", SW_TRACE_HTTP_CLIENT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_AIO", SW_TRACE_AIO);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_SSL", SW_TRACE_SSL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_NORMAL", SW_TRACE_NORMAL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_CHANNEL", SW_TRACE_CHANNEL);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_TIMER", SW_TRACE_TIMER);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_SOCKET", SW_TRACE_SOCKET);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_COROUTINE", SW_TRACE_COROUTINE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_CONTEXT", SW_TRACE_CONTEXT);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_CO_HTTP_SERVER", SW_TRACE_CO_HTTP_SERVER);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_TRACE_ALL", SW_TRACE_ALL);

    /**
     * log level
     */
    SW_REGISTER_LONG_CONSTANT("SWOOLE_LOG_DEBUG", SW_LOG_DEBUG);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_LOG_TRACE", SW_LOG_TRACE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_LOG_INFO", SW_LOG_INFO);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_LOG_NOTICE", SW_LOG_NOTICE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_LOG_WARNING", SW_LOG_WARNING);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_LOG_ERROR", SW_LOG_ERROR);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_LOG_NONE", SW_LOG_NONE);

    SW_REGISTER_LONG_CONSTANT("SWOOLE_IPC_NONE", SW_IPC_NONE);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_IPC_UNIXSOCK", SW_IPC_UNIXSOCK);
    SW_REGISTER_LONG_CONSTANT("SWOOLE_IPC_SOCKET", SW_IPC_SOCKET);

    if (!SWOOLE_G(use_shortname))
    {
        zend_hash_str_del(CG(function_table), ZEND_STRL("go"));
        zend_hash_str_del(CG(function_table), ZEND_STRL("defer"));
    }

    SW_INIT_CLASS_ENTRY(swoole_event, "Swoole\\Event", "swoole_event", NULL, swoole_event_methods);
    SW_SET_CLASS_CREATE(swoole_event, sw_zend_create_object_deny);

    SW_INIT_EXCEPTION_CLASS_ENTRY(swoole_exception, "Swoole\\Exception", "swoole_exception", NULL, NULL);

    swoole_init();
    if (!SWOOLE_G(enable_coroutine))
    {
        SwooleG.enable_coroutine = 0;
    }
    if (strcmp("cli", sapi_module.name) == 0 || strcmp("phpdbg", sapi_module.name) == 0)
    {
        SWOOLE_G(cli) = 1;
    }

    /** <Sort by dependency> **/
    // base
    swoole_atomic_init(module_number);
    swoole_buffer_init(module_number);
    swoole_lock_init(module_number);
    swoole_process_init(module_number);
    swoole_process_pool_init(module_number);
    swoole_table_init(module_number);
    swoole_timer_init(module_number);
    // coroutine
    swoole_async_coro_init(module_number);
    swoole_coroutine_util_init(module_number);
    swoole_channel_coro_init(module_number);
    swoole_runtime_init(module_number);
    // client
    swoole_socket_coro_init(module_number);
    swoole_client_init(module_number);
    swoole_client_coro_init(module_number);
    swoole_http_client_coro_init(module_number);
    swoole_mysql_coro_init(module_number);
    swoole_redis_coro_init(module_number);
#ifdef SW_USE_HTTP2
    swoole_http2_client_coro_init(module_number);
#endif
    // server
    swoole_server_init(module_number);
    swoole_server_port_init(module_number);
    swoole_http_request_init(module_number);
    swoole_http_response_init(module_number);
    swoole_http_server_init(module_number);
    swoole_http_server_coro_init(module_number);
    swoole_websocket_server_init(module_number);
    swoole_redis_server_init(module_number);

    SwooleG.fatal_error = php_swoole_fatal_error;
    SwooleG.socket_buffer_size = SWOOLE_G(socket_buffer_size);
    SwooleG.dns_cache_refresh_time = 60;

    swoole_objects.size = SWOOLE_OBJECT_DEFAULT;
    swoole_objects.array = (void**) sw_calloc(swoole_objects.size, sizeof(void*));
    if (!swoole_objects.array)
    {
        swoole_php_fatal_error(E_ERROR, "malloc([swoole_objects]) failed");
        exit(253);
    }

    // enable pcre.jit and use swoole extension on MacOS will lead to coredump, disable it temporarily
#if defined(PHP_PCRE_VERSION) && PHP_VERSION_ID >= 70300 && __MACH__ && !defined(SW_DEBUG)
    PCRE_G(jit) = 0;
#endif

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(swoole)
{
    swoole_clean();

    return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(swoole)
{
    char buf[64];
    php_info_print_table_start();
    php_info_print_table_header(2, "Swoole", "enabled");
    php_info_print_table_row(2, "Author", "Swoole Team <team@swoole.com>");
    php_info_print_table_row(2, "Version", SWOOLE_VERSION);
    snprintf(buf, sizeof(buf), "%s %s", __DATE__, __TIME__);
    php_info_print_table_row(2, "Built", buf);
    php_info_print_table_row(2, "coroutine", "enabled");
#ifdef SW_DEBUG
    php_info_print_table_row(2, "debug", "enabled");
#endif
#ifdef SW_LOG_TRACE_OPEN
    php_info_print_table_row(2, "trace_log", "enabled");
#endif
#ifdef SW_NO_USE_ASM_CONTEXT
#ifdef HAVE_BOOST_CONTEXT
    php_info_print_table_row(2, "boost.context", "enabled");
#else
    php_info_print_table_row(2, "ucontext", "enabled");
#endif
#endif
#ifdef HAVE_EPOLL
    php_info_print_table_row(2, "epoll", "enabled");
#endif
#ifdef HAVE_EVENTFD
    php_info_print_table_row(2, "eventfd", "enabled");
#endif
#ifdef HAVE_KQUEUE
    php_info_print_table_row(2, "kqueue", "enabled");
#endif
#ifdef HAVE_SIGNALFD
    php_info_print_table_row(2, "signalfd", "enabled");
#endif
#ifdef SW_USE_ACCEPT4
    php_info_print_table_row(2, "accept4", "enabled");
#endif
#ifdef HAVE_CPU_AFFINITY
    php_info_print_table_row(2, "cpu_affinity", "enabled");
#endif
#ifdef HAVE_SPINLOCK
    php_info_print_table_row(2, "spinlock", "enabled");
#endif
#ifdef HAVE_RWLOCK
    php_info_print_table_row(2, "rwlock", "enabled");
#endif
#ifdef SW_SOCKETS
    php_info_print_table_row(2, "sockets", "enabled");
#endif
#ifdef SW_USE_OPENSSL
#ifdef OPENSSL_VERSION_TEXT
    php_info_print_table_row(2, "openssl", OPENSSL_VERSION_TEXT);
#else
    php_info_print_table_row(2, "openssl", "enabled");
#endif
#endif
#ifdef SW_USE_HTTP2
#ifdef NGHTTP2_VERSION
    php_info_print_table_row(2, "http2", NGHTTP2_VERSION);
#else
    php_info_print_table_row(2, "http2", "enabled");
#endif
#endif
#ifdef HAVE_PCRE
    php_info_print_table_row(2, "pcre", "enabled");
#endif
#ifdef SW_HAVE_ZLIB
    php_info_print_table_row(2, "zlib", "enabled");
#endif
#ifdef SW_HAVE_BROTLI
    php_info_print_table_row(2, "brotli", "enabled");
#endif
#ifdef HAVE_MUTEX_TIMEDLOCK
    php_info_print_table_row(2, "mutex_timedlock", "enabled");
#endif
#ifdef HAVE_PTHREAD_BARRIER
    php_info_print_table_row(2, "pthread_barrier", "enabled");
#endif
#ifdef HAVE_FUTEX
    php_info_print_table_row(2, "futex", "enabled");
#endif
#ifdef SW_USE_MYSQLND
    php_info_print_table_row(2, "mysqlnd", "enabled");
#endif
#ifdef SW_USE_JEMALLOC
    php_info_print_table_row(2, "jemalloc", "enabled");
#endif
#ifdef SW_USE_TCMALLOC
    php_info_print_table_row(2, "tcmalloc", "enabled");
#endif
#ifdef SW_USE_HUGEPAGE
    php_info_print_table_row(2, "hugepage", "enabled");
#endif
    php_info_print_table_row(2, "async_redis", "enabled");
#ifdef SW_USE_POSTGRESQL
    php_info_print_table_row(2, "coroutine_postgresql", "enabled");
#endif
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

PHP_RINIT_FUNCTION(swoole)
{
    SWOOLE_G(req_status) = PHP_SWOOLE_RINIT_BEGIN;
    SwooleG.running = 1;
    php_swoole_register_shutdown_function("swoole_internal_call_user_shutdown_begin");
    if (SWOOLE_G(enable_library))
    {
        php_swoole_load_library();
    }
    SWOOLE_G(req_status) = PHP_SWOOLE_RINIT_END;
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(swoole)
{
    SWOOLE_G(req_status) = PHP_SWOOLE_RSHUTDOWN_BEGIN;
    swoole_call_rshutdown_function(NULL);

    //clear pipe buffer
    if (SwooleG.serv && swIsWorker())
    {
        swWorker_clean();
    }

    if (SwooleG.serv && SwooleG.serv->gs->start > 0 && SwooleG.running > 0)
    {
        if (PG(last_error_message))
        {
            switch(PG(last_error_type))
            {
            case E_ERROR:
            case E_CORE_ERROR:
            case E_USER_ERROR:
            case E_COMPILE_ERROR:
                swoole_error_log(
                    SW_LOG_ERROR, SW_ERROR_PHP_FATAL_ERROR, "Fatal error: %s in %s on line %d",
                    PG(last_error_message), PG(last_error_file)?PG(last_error_file):"-", PG(last_error_lineno)
                );
                break;
            default:
                break;
            }
        }
        else
        {
            swoole_error_log(SW_LOG_NOTICE, SW_ERROR_SERVER_WORKER_TERMINATED, "worker process is terminated by exit()/die()");
        }
    }

    swAio_free();

    swoole_async_coro_shutdown();
    swoole_redis_server_shutdown();
    swoole_coroutine_shutdown();
    swoole_runtime_shutdown();

    SwooleG.running = 0;
    SwooleWG.reactor_wait_onexit = 0;

    SWOOLE_G(req_status) = PHP_SWOOLE_RSHUTDOWN_END;

    return SUCCESS;
}

PHP_FUNCTION(swoole_version)
{
    RETURN_STRING(SWOOLE_VERSION);
}

static uint32_t hashkit_one_at_a_time(const char *key, size_t key_length)
{
    const char *ptr = key;
    uint32_t value = 0;

    while (key_length--)
    {
        uint32_t val = (uint32_t) *ptr++;
        value += val;
        value += (value << 10);
        value ^= (value >> 6);
    }
    value += (value << 3);
    value ^= (value >> 11);
    value += (value << 15);

    return value;
}

static PHP_FUNCTION(swoole_hashcode)
{
    char *data;
    size_t l_data;
    zend_long type = 0;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STRING(data, l_data)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(type)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    switch(type)
    {
    case 1:
        RETURN_LONG(hashkit_one_at_a_time(data, l_data));
    default:
        RETURN_LONG(zend_hash_func(data, l_data));
    }
}

PHP_FUNCTION(swoole_last_error)
{
    RETURN_LONG(SwooleG.error);
}

PHP_FUNCTION(swoole_cpu_num)
{
    static long cpu_num = 0;
    if (cpu_num == 0)
    {
        cpu_num = SW_MAX(1, sysconf(_SC_NPROCESSORS_CONF));
    }
    RETURN_LONG(cpu_num);
}

PHP_FUNCTION(swoole_strerror)
{
    zend_long swoole_errno;
    zend_long error_type = SW_STRERROR_SYSTEM;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(swoole_errno)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(error_type)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    if (error_type == SW_STRERROR_GAI)
    {
        RETURN_STRING(gai_strerror(swoole_errno));
    }
    else if (error_type == SW_STRERROR_DNS)
    {
        RETURN_STRING(hstrerror(swoole_errno));
    }
    else if (error_type == SW_STRERROR_SWOOLE || (swoole_errno > SW_ERROR_START && swoole_errno < SW_ERROR_END))
    {
        RETURN_STRING(swoole_strerror(swoole_errno));
    }
    else
    {
        RETURN_STRING(strerror(swoole_errno));
    }
}

PHP_FUNCTION(swoole_get_mime_type)
{
    char *filename;
    size_t filename_len;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(filename, filename_len)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    RETURN_STRING(swoole_mime_type_get(filename));
}

PHP_FUNCTION(swoole_errno)
{
    RETURN_LONG(errno);
}

PHP_FUNCTION(swoole_set_process_name)
{
#ifdef __MACH__
    // OSX doesn't support 'cli_set_process_title'
    swoole_php_fatal_error(E_WARNING, "swoole_set_process_name is not supported on OSX");
    RETURN_FALSE;
#else
    zend_function *cli_set_process_title = (zend_function *) zend_hash_str_find_ptr(EG(function_table),
            ZEND_STRL("cli_set_process_title"));
    if (!cli_set_process_title)
    {
        swoole_php_fatal_error(E_WARNING, "swoole_set_process_name only support in CLI mode");
        RETURN_FALSE;
    }
    cli_set_process_title->internal_function.handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
#endif
}

PHP_FUNCTION(swoole_get_local_ip)
{
    struct sockaddr_in *s4;
    struct ifaddrs *ipaddrs, *ifa;
    void *in_addr;
    char ip[64];

    if (getifaddrs(&ipaddrs) != 0)
    {
        swoole_php_sys_error(E_WARNING, "getifaddrs() failed");
        RETURN_FALSE;
    }
    array_init(return_value);
    for (ifa = ipaddrs; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL || !(ifa->ifa_flags & IFF_UP))
        {
            continue;
        }

        switch (ifa->ifa_addr->sa_family)
        {
            case AF_INET:
                s4 = (struct sockaddr_in *)ifa->ifa_addr;
                in_addr = &s4->sin_addr;
                break;
            case AF_INET6:
                //struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                //in_addr = &s6->sin6_addr;
                continue;
            default:
                continue;
        }
        if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, ip, sizeof(ip)))
        {
            php_error_docref(NULL, E_WARNING, "%s: inet_ntop failed", ifa->ifa_name);
        }
        else
        {
            //if (ifa->ifa_addr->sa_family == AF_INET && ntohl(((struct in_addr *) in_addr)->s_addr) == INADDR_LOOPBACK)
            if (strcmp(ip, "127.0.0.1") == 0)
            {
                continue;
            }
            add_assoc_string(return_value, ifa->ifa_name, ip);
        }
    }
    freeifaddrs(ipaddrs);
}

PHP_FUNCTION(swoole_get_local_mac)
{
#ifdef SIOCGIFHWADDR
    struct ifconf ifc;
    struct ifreq buf[16];
    char mac[32] = {0};

    int sock;
    int i = 0,num = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        swoole_php_sys_error(E_WARNING, "new socket failed");
        RETURN_FALSE;
    }
    array_init(return_value);

    ifc.ifc_len = sizeof (buf);
    ifc.ifc_buf = (caddr_t) buf;
    if (!ioctl(sock, SIOCGIFCONF, (char *) &ifc))
    {
        num = ifc.ifc_len / sizeof (struct ifreq);
        while (i < num)
        {
            if (!(ioctl(sock, SIOCGIFHWADDR, (char *) &buf[i])))
            {
                sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
                        (unsigned char) buf[i].ifr_hwaddr.sa_data[0],
                        (unsigned char) buf[i].ifr_hwaddr.sa_data[1],
                        (unsigned char) buf[i].ifr_hwaddr.sa_data[2],
                        (unsigned char) buf[i].ifr_hwaddr.sa_data[3],
                        (unsigned char) buf[i].ifr_hwaddr.sa_data[4],
                        (unsigned char) buf[i].ifr_hwaddr.sa_data[5]);
                add_assoc_string(return_value, buf[i].ifr_name, mac);
            }
            i++;
        }
    }
    close(sock);
#else
    php_error_docref(NULL, E_WARNING, "swoole_get_local_mac is not supported");
    RETURN_FALSE;
#endif
}

PHP_FUNCTION(swoole_internal_call_user_shutdown_begin)
{
    if (SWOOLE_G(req_status) == PHP_SWOOLE_RINIT_END)
    {

        SWOOLE_G(req_status) = PHP_SWOOLE_CALL_USER_SHUTDOWNFUNC_BEGIN;
        RETURN_TRUE;
    }
    else
    {
        php_error_docref(NULL, E_WARNING, "can not call this function in user level");
        RETURN_FALSE;
    }
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
