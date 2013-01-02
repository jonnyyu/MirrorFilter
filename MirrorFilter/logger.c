#include "logger.h"
#include <stdarg.h>

static
PCHAR _DbgLevel[] = {
	NULL,      // 0
	"[MIR]ERROR:",   // 1
	"[MIR]WARNING:", // 2
	NULL,      // 3
	"[MIR]INFO:"     // 4
};


VOID
_DbgPrint(
	_In_ DbgLevel level,
	_In_z_ _Printf_format_string_ PCSTR Format,
	...
)
{
	va_list argList;
	va_start(argList, Format);
	vDbgPrintExWithPrefix(_DbgLevel[level], DPFLTR_IHVDRIVER_ID, level,Format, argList);
	va_end(argList);
}