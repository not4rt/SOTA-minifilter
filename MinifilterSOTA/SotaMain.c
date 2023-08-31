/*++

Module Name:

    sotamain.c

Abstract:

    This is the main module of the sota mini-filter driver.
    This filter implement the kernel side of the 
    State Of The Art (SOTA) Ransomware Detector.

    Sota prefix denotes "State Of The Art" module.

Environment:

    Kernel mode

--*/

#include <SotaMain.h>

//
//  Constant FLT_REGISTRATION structure for our filter.  This
//  initializes the callback routines our filter wants to register
//  for.  This is only used to register with the filter manager
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
    {
        IRP_MJ_CREATE,
        0,
        SotaPreOperationCallback,
        SotaPostOperationCallback
    },
    {
        IRP_MJ_READ,
        0,
        SotaPreOperationCallback,
        SotaPostOperationCallback
    },
    {
        IRP_MJ_CLEANUP,
        0,
        SotaPreOperationCallback,
        NULL
    },
    {
        IRP_MJ_WRITE,
        0,
        SotaPreOperationCallback,
        NULL    
    },
    {
        IRP_MJ_SET_INFORMATION,
        0,
        SotaPreOperationCallback,
        NULL
    },
    {
        IRP_MJ_OPERATION_END
    }
};

//
//  FilterRegistration defines what we want to filter
//

CONST FLT_REGISTRATION FilterRegistration = {
    sizeof(FLT_REGISTRATION),       // Size
    FLT_REGISTRATION_VERSION,       // Version
    0,                              // Flags
    NULL,                           // Context
    Callbacks,                      // Operation callbacks
    SotaUnload,                     // Minifilter Unload
    SotaInstanceSetup,              // InstanceSetup
    SotaInstanceQueryTeardown,      // InstanceQueryTeardown
    NULL, //SotaInstanceTeardownStart,      // InstanceTeardownStart
    NULL, //SotaInstanceTeardownComplete,   // InstanceTeardownComplete
    NULL,                           // GenerateFileName
    NULL,                           // GenerateDestinationFileName
    NULL                            // NormalizeNameComponent
};

/*************************************************************************
    SOTA MiniFilter initialization and unload routines
*************************************************************************/

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
/*++

Routine Description:

    This is the initialization routine for this miniFilter driver.  This
    registers with FltMgr and initializes all global data structures.

Arguments:

    DriverObject - Pointer to driver object created by the system to
        represent this driver.

    RegistryPath - Unicode string identifying where the parameters for this
        driver are located in the registry.

Return Value:

    Returns the final status of this operation.

--*/
{
    DbgPrint("[SOTA3] DriverEntry: Begin");
    UNREFERENCED_PARAMETER(RegistryPath);
    NTSTATUS status = STATUS_SUCCESS;

    ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

    RtlZeroMemory(&Globals, sizeof(Globals));
    Globals.DriverObject = DriverObject;

    //
    //  Register with FltMgr to tell it our callback routines
    //
    status = FltRegisterFilter(DriverObject,
        &FilterRegistration,
        &Globals.Filter);
    if (!NT_SUCCESS(status)) {
        DbgPrint("[SOTA] DriverEntry: Failed to register minifilter - STATUS: 0x%x\n", status);
        return status;
    }

    //
    //  Start filtering I/O.
    //
    status = FltStartFiltering(Globals.Filter);
    if (!NT_SUCCESS(status)) {
        DbgPrint("[SOTA] DriverEntry: Failed to start filtering - STATUS: 0x%x\n", status);
        FltUnregisterFilter(Globals.Filter);
        return status;
    }

    status = PsSetCreateProcessNotifyRoutine(SOTAProcessCreateCallback, FALSE);
    if (!NT_SUCCESS(status)) {
        DbgPrint("[SOTA] DriverEntry: Failed to create new process notify routine. - STATUS: 0x%x\n", status);
        FltUnregisterFilter(Globals.Filter);
        return status;
    }

    Globals.PidTable = initialize_map();

    DbgPrint("[SOTA] DriverEntry: Success - STATUS: 0x%x\n", status);
    return status;
}

NTSTATUS
SotaUnload(
    _Unreferenced_parameter_ FLT_FILTER_UNLOAD_FLAGS Flags
)
/*++

Routine Description:

    This is the unload routine for this miniFilter driver. This is called
    when the minifilter is about to be unloaded. We can fail this unload
    request if this is not a mandatory unloaded indicated by the Flags
    parameter.

Arguments:

    Flags - Indicating if this is a mandatory unload.

Return Value:

    Returns the final status of this operation.

--*/
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(Flags);

    DbgPrint("[SOTA] SotaUnload: Begin");


    //FltCloseCommunicationPort(Globals.ScanServerPort);
    //Globals.ScanServerPort = NULL;
    //FltCloseCommunicationPort(Globals.AbortServerPort);
    //Globals.AbortServerPort = NULL;
    //FltCloseCommunicationPort(Globals.QueryServerPort);
    //Globals.QueryServerPort = NULL;
    FltUnregisterFilter(Globals.Filter);  // This will typically trigger instance tear down.
    Globals.Filter = NULL;

    PsSetCreateProcessNotifyRoutine(SOTAProcessCreateCallback, TRUE);

    free_map(Globals.PidTable);

    DbgPrint("[SOTA] SotaUnload: Finished");
    return STATUS_SUCCESS;
}

FLT_PREOP_CALLBACK_STATUS
SotaPreOperationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
/*++

Routine Description:

    This routine is the registered callback routine for filtering
    the "write" operation, i.e. the operations that have potentials
    to modify the file.

    This is non-pageable because it could be called on the paging path

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - If this callback routine returns FLT_PREOP_SUCCESS_WITH_CALLBACK or
        FLT_PREOP_SYNCHRONIZE, this parameter is an optional context pointer to be passed to
        the corresponding post-operation callback routine. Otherwise, it must be NULL.

Return Value:

    The return value is the status of the operation.

--*/
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext); 
    if (FltGetRequestorProcessId(Data) == 4) // PID 4 is the system process
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    if (!StringStartsWithAnyPrefix(&Data->Iopb->TargetFileObject->FileName, CanaryFiles, CANARYFILES_COUNT)) {
        //DbgPrint("[SOTA] SotaPreOperationCallback: Compare %s - %s or %s", &Data->Iopb->TargetFileObject->FileName, L"\\Users\\", L"\\Users\\User\\AppData");
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }
    DbgPrint("[SOTA] SotaPreOperationCallback: Begin");
    PUNICODE_STRING processName = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    status = GetProcessNameByPid((HANDLE)FltGetRequestorProcessId(Data), &processName);
    switch (Data->Iopb->MajorFunction) {
    case IRP_MJ_CREATE: {
        DbgPrint("[SOTA] SotaPreOperationCallback: MajorFunction: IRP_MJ_CREATE - PID: %i - ProcessName: %wZ - FileName: %wZ\n", FltGetRequestorProcessId(Data), processName, &Data->Iopb->TargetFileObject->FileName);
        break;
    }
    case IRP_MJ_READ: {
        DbgPrint("[SOTA] SotaPreOperationCallback: MajorFunction: IRP_MJ_READ - PID: %i - ProcessName: %wZ - FileName: %wZ\n", FltGetRequestorProcessId(Data), processName, &Data->Iopb->TargetFileObject->FileName);
        break;
    }
    case IRP_MJ_CLEANUP: {
        DbgPrint("[SOTA] SotaPreOperationCallback: MajorFunction: IRP_MJ_CLEANUP - PID: %i - ProcessName: %wZ - FileName: %wZ\n", FltGetRequestorProcessId(Data), processName, &Data->Iopb->TargetFileObject->FileName);
        break;
    }
    case IRP_MJ_WRITE: {
        DbgPrint("[SOTA-CRITICAL] SotaPreOperationCallback: Something tried to modify CanaryFile. MajorFunction: IRP_MJ_WRITE - PID: %i - ProcessName: %wZ - FileName: %wZ", FltGetRequestorProcessId(Data), processName, &Data->Iopb->TargetFileObject->FileName);
        status = TerminateProcessByPid((HANDLE)FltGetRequestorProcessId(Data));
        DbgPrint("[SOTA-KILL] SotaPreOperationCallback: Operation intercepted. STATUS: 0x%x - PID: %i\n", status, FltGetRequestorProcessId(Data));
        return FLT_PREOP_COMPLETE;
        break;
    }
    case IRP_MJ_SET_INFORMATION: {
        DbgPrint("[SOTA-CRITICAL] SotaPreOperationCallback: Something tried to modify CanaryFile. MajorFunction: IRP_MJ_SET_INFORMATION - PID: %i - ProcessName: %wZ - FileName: %wZ", FltGetRequestorProcessId(Data), processName, &Data->Iopb->TargetFileObject->FileName);
        status = TerminateProcessByPid((HANDLE)FltGetRequestorProcessId(Data));
        DbgPrint("[SOTA-KILL] SotaPreOperationCallback: Operation intercepted. STATUS: 0x%x - PID: %i\n", status, FltGetRequestorProcessId(Data));
        return FLT_PREOP_COMPLETE;
        break;
        }
    };

    if (processName) {
        ExFreePool(processName);
    }
        
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS
SotaPostOperationCallback(_Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
/*++

Routine Description:

    This routine is the post-create completion routine.
    In this routine, stream context and/or transaction context shall be
    created if not exits.

    Note that we only allocate and set the stream context to filter manager
    at post create.

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - The completion context set in the pre-create routine.

    Flags - Denotes whether the completion is successful or is being drained.

Return Value:

    The return value is the status of the operation.

--*/
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);
    DbgPrint("[SOTA] SotaPostOperationCallback: Begin");
    DbgPrint("[SOTA] SotaPostOperationCallback: IRP: %s - PID: %u\n", FltGetIrpName(Data->Iopb->MajorFunction), FltGetRequestorProcessId(Data));
    PAGED_CODE();

    //NTSTATUS status = Data->IoStatus.Status;


    return FLT_POSTOP_FINISHED_PROCESSING;
}

NTSTATUS SotaInstanceSetup(
    _In_ PCFLT_RELATED_OBJECTS  FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS  Flags,
    _In_ DEVICE_TYPE  VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE  VolumeFilesystemType)
{
    //
    // This is called to see if a filter would like to attach an instance to the given volume.
    //
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(VolumeDeviceType);
    UNREFERENCED_PARAMETER(VolumeFilesystemType);

    return STATUS_SUCCESS;
}

NTSTATUS SotaInstanceQueryTeardown(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
)
{
    //
    // This is called to see if the filter wants to detach from the given volume.
    //
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);

    return STATUS_SUCCESS;
}

NTSTATUS 
GetProcessNameByPid(HANDLE pid, PUNICODE_STRING* ProcessName)
{
    PEPROCESS eProcess = NULL;
    NTSTATUS status = PsLookupProcessByProcessId(pid, &eProcess);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    // Get the process image name
    status = SeLocateProcessImageName(eProcess, ProcessName);
    ObDereferenceObject(eProcess);  // Always dereference after using the EPROCESS object

    return status;
}

NTSTATUS TerminateProcessByPid(HANDLE pid)
{
    DbgPrint("[SOTA-KILL] TerminateProcessByPid: Killing Process with PID: %d\n", (ULONG)(ULONG_PTR)pid);
    PEPROCESS Process;
    NTSTATUS status = PsLookupProcessByProcessId(pid, &Process);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("[SOTA-KILL] TerminateProcessByPid: Failed in lookup process: %d\n", (ULONG)(ULONG_PTR)pid);
        return status;
    }

    // Get a handle to the process using ZwOpenProcess
    HANDLE ProcessHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    CLIENT_ID ClientId = { pid, 0 };

    InitializeObjectAttributes(&ObjectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
    status = ZwOpenProcess(&ProcessHandle, PROCESS_ALL_ACCESS, &ObjectAttributes, &ClientId);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("[SOTA-KILL] TerminateProcessByPid: Failed to open process handle: %d\n", (ULONG)(ULONG_PTR)pid);
        ObDereferenceObject(Process);
        return status;
    }

    // Use ZwTerminateProcess to terminate the process
    status = ZwTerminateProcess(ProcessHandle, STATUS_SUCCESS); // Here you should use a valid status code or just STATUS_SUCCESS
    ZwClose(ProcessHandle); // Close the handle
    ObDereferenceObject(Process);

    return status;
}

BOOLEAN 
StringStartsWithAnyPrefix(PUNICODE_STRING target, PWCHAR* PrefixList, int count)
{
    UNICODE_STRING temp;
    for (int i = 0; i < count; i++)
    {
        RtlInitUnicodeString(&temp, PrefixList[i]);

        if (RtlPrefixUnicodeString(&temp, target, TRUE))  // TRUE for case-insensitive
        {
            return TRUE;
        }
    }

    return FALSE;
}

VOID SOTAProcessCreateCallback(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create) {
    if (Create) {
        // New Process Created
        DbgPrint("[SOTA] SOTAProcessCreateCallback: Begin");
        NTSTATUS status = STATUS_SUCCESS;

        PUNICODE_STRING currentProcessName = NULL;
        status = GetProcessNameByPid(ParentId, &currentProcessName);
        if (!NT_SUCCESS(status)) {
            DbgPrint("[SOTA] SOTAProcessCreateCallback: Could not get parent process name. - PID: %d - STATUS: 0x%x\n", (ULONG)(ULONG_PTR)ProcessId, status);
            return;
        }

        PUNICODE_STRING parentProcessName = NULL;
        status = GetProcessNameByPid(ParentId, &parentProcessName);
        if (!NT_SUCCESS(status)) {
            DbgPrint("[SOTA] SOTAProcessCreateCallback: Could not get parent process name. - PID: %d - STATUS: 0x%x\n", (ULONG)(ULONG_PTR)ParentId, status);
            return;
        }

        if (StringStartsWithAnyPrefix(currentProcessName, SafeDirectories, SAFEDIRECTORIES_COUNT) && StringStartsWithAnyPrefix(parentProcessName, SafeDirectories, SAFEDIRECTORIES_COUNT)) {
            DbgPrint("[SOTA] SOTAProcessCreateCallback: New process, parent and current are safe. PID: %d - PROCESSNAME: %wZ - PARENTPID: %d - PARENTPROCESSNAME: %wZ\n", (ULONG)(ULONG_PTR)ProcessId, currentProcessName, (ULONG)(ULONG_PTR)ParentId, parentProcessName);
            if (currentProcessName) {
                ExFreePool(currentProcessName);
            }
            if (parentProcessName) {
                ExFreePool(parentProcessName);
            }
            return;
        }
        else {
            //TODO
            DbgPrint("[SOTA] SOTAProcessCreateCallback: New process created. PID: %d - PROCESSNAME: %wZ - PARENTPID: %d - PARENTPROCESSNAME: %wZ\n", (ULONG)(ULONG_PTR)ProcessId, currentProcessName, (ULONG)(ULONG_PTR)ParentId, parentProcessName);
            //status = insert(Globals.PidTable, (ULONG)ProcessId, (ULONG)ParentId);
            //return;
        }

    }
    else {
        // Old Process Killed, do nothing...
        DbgPrint("[SOTA] SOTAProcessCreateCallback: New process created. PID: %d - PROCESSNAME: %wZ - PARENTPID: %d - PARENTPROCESSNAME: %wZ\n", (ULONG)(ULONG_PTR)ProcessId, currentProcessName, (ULONG)(ULONG_PTR)ParentId, parentProcessName);
        NTSTATUS status = insert(Globals.PidTable, (ULONG)ProcessId, (ULONG)ParentId);
        return;
    }
}