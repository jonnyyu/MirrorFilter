#pragma once

#include <fltKernel.h>

PCHAR OsrNTStatusToString(NTSTATUS Status);

typedef enum _DbgLevel {
	MirError = 1,
	MirWarning = 2,
	MirInfo  = 4
} DbgLevel;

VOID _DbgPrint(_In_ DbgLevel level, _In_z_ _Printf_format_string_ PCSTR Format, ...);

#define DBG_PRINT(_level, _msg, ...) \
	_DbgPrint(_level, _msg, __VA_ARGS__); 

#define DBG_ERROR(_msg, ...) \
	DBG_PRINT(MirError, _msg, __VA_ARGS__)

#define DBG_WARNING(_msg, ...) \
	DBG_PRINT(MirWarning, _msg, __VA_ARGS__)

#define DBG_INFO(_msg, ...) \
	DBG_PRINT(MirInfo, _msg, __VA_ARGS__)

#define DBG_ERROR_ALLOC_FAIL(_var, _size) \
	DBG_ERROR("Failed to allocated %s, size= %u\n", (_var), (_size));

#define DBG_ERROR_CALL_FAIL(_func, _status) \
	DBG_ERROR("%s Failed, status = %s (0x%x)\n", #_func, OsrNTStatusToString(_status), _status)

#define DBG_INFO_FUNC_CALL(_func) \
	DBG_INFO("Calling %s\n", #_func)

#define DBG_INFO_FUNC_ENTER() \
	DBG_INFO(">>>> %s\n", __FUNCTION__)

#define DBG_INFO_FUNC_LEAVE() \
	DBG_INFO("<<<< %s\n", __FUNCTION__)

#define DBG_INFO_FLAG(_flags, _flag) \
	if (FlagOn(_flags, _flag))				\
		DBG_INFO("%s: %s\n", #_flags, #_flag);  