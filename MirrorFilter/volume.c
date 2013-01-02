#include "volume.h"

#include "logger.h"
#include "string.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGED, MirGetFltVolumeName)
#endif


NTSTATUS
MirGetFltVolumeName(
	_In_ PFLT_VOLUME Volume,
	_Out_ PUNICODE_STRING VolumeName
)
{
	ULONG NameLength = 0;
	NTSTATUS status = STATUS_SUCCESS;

	DBG_INFO_FUNC_ENTER();

	if (VolumeName == NULL || Volume == NULL) {
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	status = FltGetVolumeName(Volume, NULL, &NameLength);
	ASSERT(status == STATUS_BUFFER_TOO_SMALL);
	if (status != STATUS_BUFFER_TOO_SMALL) {
		DBG_ERROR_CALL_FAIL("FltGetVolumeName query size", status);
		goto Exit;
	}

	status = MirAllocateUnicodeString(VolumeName, NameLength);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_ALLOC_FAIL(VolumeName, NameLength);
		goto Exit;
	}

	status = FltGetVolumeName(Volume, VolumeName, &NameLength);
	if (!NT_SUCCESS(status)) {
		DBG_ERROR_CALL_FAIL(FltGetVolumeName, status);
		MirFreeUnicodeString(VolumeName);
	}

Exit:
	DBG_INFO_FUNC_LEAVE();
	return status;
}


