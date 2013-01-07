#pragma once

#include <fltKernel.h>

NTSTATUS MirAllocateUnicodeString(PUNICODE_STRING String, USHORT Length);

VOID MirFreeUnicodeString(PUNICODE_STRING String);

NTSTATUS 
MirInitUnicodeString(
	_In_ PUNICODE_STRING UnicodeString,
	_In_z_ PWSTR String);

NTSTATUS
MirCopyUnicodeString(
	_In_ PUNICODE_STRING TargetString,
	_In_ PUNICODE_STRING SourceString);