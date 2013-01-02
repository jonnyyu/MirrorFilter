#include "string.h"

#include "allocate.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MirAllocateUnicodeString)
#pragma alloc_text(PAGE, MirFreeUnicodeString)
#endif

NTSTATUS MirAllocateUnicodeString(PUNICODE_STRING String, USHORT Length) {
	
	if (String == NULL) {
		return STATUS_INVALID_PARAMETER;
	}

	if (Length + 1 == MAXUSHORT) {
		return STATUS_INVALID_PARAMETER;
	}

	String->Buffer = (PWCHAR)MirAllocatePagedPool(Length + sizeof(WCHAR));
	if (String->Buffer == NULL) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	String->MaximumLength = Length + sizeof(WCHAR);
	String->Length = Length;
	return STATUS_SUCCESS;
}

VOID MirFreeUnicodeString(PUNICODE_STRING String) {
	if (String->Buffer) {
		MirFreeUnicodeString(String);
		String->Buffer = NULL;
	}
	String->Length = 0;
	String->MaximumLength = 0;
}