#pragma once

#include <wdm.h>

#define DBG_PRINT(_level, _msg, ...) \
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, _level, _msg, __VA_ARGS__)

#define DBG_ERROR(_msg, ...) \
	DBG_PRINT(0x01, _msg, __VA_ARGS__)

#define DBG_ERROR_ALLOC_FAIL(_var, _size) \
	DBG_ERROR("Failed to allocated %s, size= %u\n", _var, _size);

#define DBG_ERROR_CALL_FAIL(_func, _status) \
	DBG_ERROR("%s Failed, status = 0x%x\n", #_func, _status)

#define DBG_WARNING(_msg, ...) \
	DBG_PRINT(0x02, _msg, __VA_ARGS__)

#define DBG_INFO(_msg, ...) \
	DBG_PRINT(0x04, _msg, __VA_ARGS__)

#define DBG_INFO_FUNC_CALL(_func) \
	DBG_INFO("Calling %s\n", #_func)

#define DBG_INFO_FUNC_ENTER() \
	DBG_INFO("===> %s\n", __FUNCTION__)

#define DBG_INFO_FUNC_LEAVE() \
	DBG_INFO("<=== %s\n", __FUNCTION__)