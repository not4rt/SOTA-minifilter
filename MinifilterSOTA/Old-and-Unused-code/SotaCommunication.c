#include "SotaCommunication.h"

NTSTATUS InitializeCommPort() {
    NTSTATUS status = STATUS_SUCCESS;

    //
    //  Create a communication port.
    //
    UNICODE_STRING UnicodeCommPortName;
    RtlInitUnicodeString(&UnicodeCommPortName, Globals.CommPortName);

    PSECURITY_DESCRIPTOR sd = NULL;
    status = FltBuildDefaultSecurityDescriptor(&sd, FLT_PORT_ALL_ACCESS);
    status = RtlSetDaclSecurityDescriptor(
        sd,     // SecurityDescriptor
        TRUE,   // DaclPresent
        NULL,   // Dacl (OPTIONAL)
        FALSE);  // DaclDefaulted (allow user application without admin to enter)

    if (!NT_SUCCESS(status)) {
        FltFreeSecurityDescriptor(sd);
        return status;
    }

    OBJECT_ATTRIBUTES oa;
    InitializeObjectAttributes(
        &oa,
        &UnicodeCommPortName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        sd);

    status = FltCreateCommunicationPort(
        Globals.Filter,                 // Filter
        &Globals.CommKernelPort,        // *ServerPort
        &oa,                            // ObjectAttributes
        NULL,                           // ServerPortCookie
        SotaConnectNotifyCallback,      // ConnectNotifyCallback
        SotaDisconnectNotifyCallback,   // DisconnectNotifyCallback
        SotaMessageNotifyCallback,      // MessageNotifyCallback
        1                               // MaxConnections
    );

    FltFreeSecurityDescriptor(sd);
    return status;
}

NTSTATUS FinalizeCommPort() {
    if (Globals.CommUserPort) {
        FltCloseClientPort(Globals.Filter, &Globals.CommUserPort);
        Globals.CommUserPort = NULL;
    }

    if (Globals.CommKernelPort) {
        FltCloseCommunicationPort(Globals.CommKernelPort);
        Globals.CommKernelPort = NULL;
    }
    Globals.UserModePid = 0;
    Globals.isCommOpen = FALSE;

    return STATUS_SUCCESS;
}

NTSTATUS SotaConnectNotifyCallback(
    IN PFLT_PORT ClientPort,
    IN PVOID ServerPortCookie,
    IN PVOID ConnectionContext,
    IN ULONG SizeOfContext,
    OUT PVOID* ConnectionPortCookie
) {
    UNREFERENCED_PARAMETER(ServerPortCookie);
    UNREFERENCED_PARAMETER(ConnectionContext);
    UNREFERENCED_PARAMETER(SizeOfContext);
    UNREFERENCED_PARAMETER(ConnectionPortCookie);

    if (Globals.isCommOpen == TRUE) {
        DbgPrint("[SOTA] SotaConnectNotifyCallback: ERROR trying to connect when communication is already open.");
        return STATUS_UNSUCCESSFUL;
    }

    Globals.CommUserPort = ClientPort;
    Globals.isCommOpen = TRUE;

    DbgPrint("[SOTA] SotaConnectNotifyCallback: Connected.");
    return STATUS_SUCCESS;
}

VOID SotaDisconnectNotifyCallback(
    IN PVOID ConnectionCookie
) {
    UNREFERENCED_PARAMETER(ConnectionCookie);

    DbgPrint("[SOTA] SotaDisconnectNotifyCallback: Begin.");

    FltCloseClientPort(Globals.Filter, &Globals.CommUserPort);
    Globals.isCommOpen = FALSE;

    //DbgPrint("[SOTA] SotaDisconnectNotifyCallback: End.");
}

NTSTATUS SotaMessageNotifyCallback(
    IN PVOID PortCookie,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PULONG ReturnOutputBufferLength
) {
    UNREFERENCED_PARAMETER(PortCookie);
    UNREFERENCED_PARAMETER(InputBufferLength);

    COM_MESSAGE* message = (COM_MESSAGE*)InputBuffer;
    if (message == NULL)
        return STATUS_UNSUCCESSFUL;

    switch (message->type) {
        case MESSAGE_GET_OPS: {
            if (OutputBuffer == NULL || OutputBufferLength != MAX_COMM_BUFFER_SIZE) {
                return STATUS_INVALID_PARAMETER;
            }

            DriverGetIrps(OutputBuffer, OutputBufferLength, ReturnOutputBufferLength);
            return STATUS_SUCCESS;
        }
        case MESSAGE_SET_PID: {
            if (message->pid == 0) {
                return STATUS_INVALID_PARAMETER;
            }

            Globals.UserModePid = message->pid;
            Globals.isCommOpen = TRUE;

            return STATUS_SUCCESS;
        }
        case MESSAGE_KILL_GID: {
            if (OutputBuffer == NULL || OutputBufferLength != sizeof(LONG)) {
                return STATUS_INVALID_PARAMETER;
            }
            int pid = message->pid;
            
            return kill_pfamily(pid);
        }
    }
    return STATUS_SUCCESS;
}

//sn99
VOID DriverGetIrps(PVOID Buffer, ULONG BufferSize, PULONG ReturnOutputBufferLength) {
    *ReturnOutputBufferLength = sizeof(RWD_REPLY_IRPS);

    PCHAR OutputBuffer = (PCHAR)Buffer;
    if (OutputBuffer == NULL)
        return;
    OutputBuffer += sizeof(RWD_REPLY_IRPS);

    ULONG BufferSizeRemain = BufferSize - sizeof(RWD_REPLY_IRPS);

    RWD_REPLY_IRPS outHeader;
    PLIST_ENTRY irpEntryList;

    PIRP_ENTRY PrevEntry = nullptr;
    PDRIVER_MESSAGE Prev = nullptr;
    USHORT prevBufferSize = 0;

    KIRQL irql = KeGetCurrentIrql();
    KeAcquireSpinLock(&Globals.irpOpsLock, &irql); 

    while (Globals.irpOpsSize) {
        irpEntryList = RemoveHeadList(&Globals.irpOps);
        Globals.irpOpsSize--;
        PIRP_ENTRY irp =
            (PIRP_ENTRY)CONTAINING_RECORD(irpEntryList, IRP_ENTRY, entry);
        UNICODE_STRING FilePath = irp->filePath;
        PDRIVER_MESSAGE irpMsg = &(irp->data);
        USHORT nameBufferSize = FilePath.Length;
        irpMsg->next = nullptr;
        irpMsg->filePath.Buffer = nullptr;
        if (FilePath.Length) {
            irpMsg->filePath.Length = nameBufferSize;
            irpMsg->filePath.MaximumLength = nameBufferSize;
        }
        else {
            irpMsg->filePath.Length = 0;
            irpMsg->filePath.MaximumLength = 0;
        }

        if (sizeof(DRIVER_MESSAGE) + nameBufferSize >=
            BufferSizeRemain) { // return to an irps list, not enough space
            InsertHeadList(&Globals.irpOps, irpEntryList);
            Globals.irpOpsSize++;
            break;
        }
        else {
            if (Prev != nullptr) {
                Prev->next =
                    PDRIVER_MESSAGE(OutputBuffer + sizeof(DRIVER_MESSAGE) +
                        prevBufferSize); // PrevFilePath might be 0 size
                if (prevBufferSize) {
                    Prev->filePath.Buffer =
                        PWCH(OutputBuffer +
                            sizeof(DRIVER_MESSAGE)); // filePath buffer is after irp
                }
                RtlCopyMemory(OutputBuffer, Prev,
                    sizeof(DRIVER_MESSAGE)); // copy previous irp
                OutputBuffer += sizeof(DRIVER_MESSAGE);
                outHeader.addSize(sizeof(DRIVER_MESSAGE));
                *ReturnOutputBufferLength += sizeof(DRIVER_MESSAGE);
                if (prevBufferSize) {
                    RtlCopyMemory(OutputBuffer, PrevEntry->Buffer,
                        prevBufferSize); // copy previous filePath
                    OutputBuffer += prevBufferSize;
                    outHeader.addSize(prevBufferSize);
                    *ReturnOutputBufferLength += prevBufferSize;
                }
                delete PrevEntry;
            }
        }

        PrevEntry = irp;
        Prev = irpMsg;
        prevBufferSize = nameBufferSize;
        if (prevBufferSize > MAX_FILE_NAME_SIZE)
            prevBufferSize = MAX_FILE_NAME_SIZE;
        BufferSizeRemain -= (sizeof(DRIVER_MESSAGE) + prevBufferSize);
        outHeader.addOp();
    }
    KeReleaseSpinLock(&Globals.irpOpsLock, irql);
    if (prevBufferSize > MAX_FILE_NAME_SIZE)
        prevBufferSize = MAX_FILE_NAME_SIZE;
    if (Prev != nullptr && PrevEntry != nullptr) {
        Prev->next = nullptr;
        if (prevBufferSize) {
            Prev->filePath.Buffer =
                PWCH(OutputBuffer +
                    sizeof(DRIVER_MESSAGE)); // filePath buffer is after irp
        }
        RtlCopyMemory(OutputBuffer, Prev,
            sizeof(DRIVER_MESSAGE)); // copy previous irp
        OutputBuffer += sizeof(DRIVER_MESSAGE);
        outHeader.addSize(sizeof(DRIVER_MESSAGE));
        *ReturnOutputBufferLength += sizeof(DRIVER_MESSAGE);
        if (prevBufferSize) {
            RtlCopyMemory(OutputBuffer, PrevEntry->Buffer,
                prevBufferSize); // copy previous filePath
            OutputBuffer += prevBufferSize;
            outHeader.addSize(prevBufferSize);
            *ReturnOutputBufferLength += prevBufferSize;
        }
        delete PrevEntry;
    }

    if (outHeader.numOps()) {
        outHeader.data = PDRIVER_MESSAGE((PCHAR)Buffer + sizeof(RWD_REPLY_IRPS));
    }

    RtlCopyMemory((PCHAR)Buffer, &(outHeader), sizeof(RWD_REPLY_IRPS));
}