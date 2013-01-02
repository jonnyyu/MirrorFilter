#pragma once

#include <fltKernel.h>

PVOID MirAllocatePagedPool(ULONG Size);
VOID MirFreePool(PVOID Block);

