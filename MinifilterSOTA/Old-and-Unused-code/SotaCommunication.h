#pragma once

#include "DriverHandler.h"

#include <fltKernel.h>

#include "SharedDefs.h"



NTSTATUS InitializeCommPort();
NTSTATUS FinalizeCommPort();
NTSTATUS SotaConnectNotifyCallback(
    IN PFLT_PORT ClientPort,
    IN PVOID ServerPortCookie,
    IN PVOID ConnectionContext,
    IN ULONG SizeOfContext,
    OUT PVOID* ConnectionPortCookie
);

VOID SotaDisconnectNotifyCallback(
    IN PVOID ConnectionCookie
);

NTSTATUS SotaMessageNotifyCallback(
    IN PVOID PortCookie,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PULONG ReturnOutputBufferLength
);

VOID DriverGetIrps(PVOID Buffer, ULONG BufferSize, PULONG ReturnOutputBufferLength);

