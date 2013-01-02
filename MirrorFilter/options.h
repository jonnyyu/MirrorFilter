#pragma once

#include <fltKernel.h>


typedef VOID (*PREGISTRY_KEYVALUE_CALLBACK)(UNICODE_STRING KeyName, KEY_VALUE_PARTIAL_INFORMATION Value, PVOID Data);

NTSTATUS 
MirEnumerateMirrorPaths(
	_In_ PREGISTRY_KEYVALUE_CALLBACK Callback, 
	_In_ PVOID Data
);