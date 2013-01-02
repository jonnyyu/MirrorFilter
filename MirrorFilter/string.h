#pragma once

#include <wdm.h>

NTSTATUS MirAllocateUnicodeString(PUNICODE_STRING String, USHORT Length);
VOID MirFreeUnicodeString(PUNICODE_STRING String);