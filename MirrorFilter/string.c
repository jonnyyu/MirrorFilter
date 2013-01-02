#include "string.h"

#include "allocate.h"
#include "logger.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MirAllocateUnicodeString)
#pragma alloc_text(PAGE, MirFreeUnicodeString)
#endif

NTSTATUS MirAllocateUnicodeString(PUNICODE_STRING String, ULONG Length) {
	USHORT BufferLength = 0;

	if (String == NULL) {
		DBG_ERROR("String cannot be null.\n");
		return STATUS_INVALID_PARAMETER;
	}

	if (Length + sizeof(WCHAR) > MAXUSHORT) {
		DBG_ERROR("String Length %u exceeds MAXUSHORT(%u).\n", Length, MAXUSHORT);
		return STATUS_INVALID_PARAMETER;
	}

	BufferLength = (USHORT)Length + sizeof(WCHAR);
	String->Buffer = (PWCHAR)MirAllocatePagedPool(BufferLength);
	if (String->Buffer == NULL) {
		DBG_ERROR_ALLOC_FAIL(String->Buffer, BufferLength);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	String->MaximumLength = BufferLength;
	String->Length = (USHORT)Length;
	return STATUS_SUCCESS;
}

VOID MirFreeUnicodeString(PUNICODE_STRING String) {
	if (String->Buffer) {
		MirFreePool(String->Buffer);
		String->Buffer = NULL;
	}
	String->Length = 0;
	String->MaximumLength = 0;
}