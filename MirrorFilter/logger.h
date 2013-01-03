#pragma once

#include <fltKernel.h>

PCHAR OsrNTStatusToString(NTSTATUS Status);

typedef enum _DbgLevel {
	MirError = 1,
	MirWarning = 2,
	MirInfo  = 4
} DbgLevel;

VOID 
_DbgPrint(
	_In_ DbgLevel level, 
	_In_z_ _Printf_format_string_ PCSTR Format, 
	...
);

VOID
_DbgInfoPrintMajorFunction(
	_In_ PCSTR VariableName,
	_In_ UCHAR MajorFunction
);

VOID
_DbgInfoPrintMinorFunction(
	_In_ PCSTR VariableName,
	_In_ UCHAR MajorFunction,
	_In_ UCHAR MinorFunction
);

VOID
_DbgInfoPrintIrpFlags(
	_In_ PCSTR Variable,
	_In_ ULONG IrpFlags);

VOID
_DbgInfoPrintFltFlags(
	_In_ PCSTR VariableName,
	_In_ FLT_CALLBACK_DATA_FLAGS Flags);


#define DBG_PRINT(_level, _msg, ...) \
	_DbgPrint(_level, _msg, __VA_ARGS__); 

#define DBG_ERROR(_msg, ...) \
	DBG_PRINT(MirError, _msg, __VA_ARGS__)

#define DBG_WARNING(_msg, ...) \
	DBG_PRINT(MirWarning, _msg, __VA_ARGS__)

#define DBG_INFO(_msg, ...) \
	DBG_PRINT(MirInfo, _msg, __VA_ARGS__)

#define DBG_ERROR_ALLOC_FAIL(_var, _size) \
	DBG_ERROR("Failed to allocated %s, size= %u\n", (_var), (_size))

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

#define DBG_INFO_FLT_FLAGS(_flags) \
	_DbgInfoPrintFltFlags(#_flags, _flags)

#define DBG_INFO_PRINT_MAJOR_MINOR(_majorCode, _minorCode) \
	_DbgInfoPrintMajorFunction(#_majorCode, _minorCode); \
	_DbgInfoPrintMinorFunction(#_minorCode, _majorCode, _minorCode);

#define DBG_INFO_IRP_FLAGS(_irpFlags) \
	_DbgInfoPrintIrpFlags(#_irpFlags, _irpFlags)