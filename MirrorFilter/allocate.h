#pragma once

#include <wdm.h>

PVOID MirAllocatePagedPool(ULONG Size);
VOID MirFreePool(PVOID Block);

