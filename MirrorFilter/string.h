#pragma once

#include <fltKernel.h>

NTSTATUS MirAllocateUnicodeString(PUNICODE_STRING String, ULONG Length);
VOID MirFreeUnicodeString(PUNICODE_STRING String);