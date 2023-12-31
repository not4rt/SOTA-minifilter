#pragma once
#include "DriverHandler.h"

PCWSTR SafeOrigins[] = {
    //hv1
    L"\\Device\\HarddiskVolume1\\Windows\\",
    L"\\Device\\HarddiskVolume1\\Program Files(x86)\\",
    L"\\Device\\HarddiskVolume1\\Program Files (x86)\\",
    L"\\Device\\HarddiskVolume1\\ProgramData\\Microsoft\\Windows Defender\\",
    //hv2
    L"\\Device\\HarddiskVolume2\\Windows\\",
    L"\\Device\\HarddiskVolume2\\Program Files(x86)\\",
    L"\\Device\\HarddiskVolume2\\Program Files (x86)\\",
    L"\\Device\\HarddiskVolume2\\ProgramData\\Microsoft\\Windows Defender\\",
    //hv3
    L"\\Device\\HarddiskVolume3\\Windows\\",
    L"\\Device\\HarddiskVolume3\\Program Files(x86)\\",
    L"\\Device\\HarddiskVolume3\\Program Files (x86)\\",
    L"\\Device\\HarddiskVolume3\\ProgramData\\Microsoft\\Windows Defender\\",
    //hv4
    L"\\Device\\HarddiskVolume4\\Windows\\",
    L"\\Device\\HarddiskVolume4\\Program Files(x86)\\",
    L"\\Device\\HarddiskVolume4\\Program Files (x86)\\",
    L"\\Device\\HarddiskVolume4\\ProgramData\\Microsoft\\Windows Defender\\",
    //others
    L"\\Users\\WDKRemoteUser\\AppData\\Local\\Programs\\Microsoft VS Code\\"
};
#define SAFEORIGINS_COUNT (sizeof(SafeOrigins) / sizeof(PCWSTR))

//SOTA_DRIVER_DATA initialize_driverobj(PDRIVER_OBJECT DriverObject) {
//    SOTA_DRIVER_DATA map = (HashMap*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(HashMap), 'map1');`
//    if (!map) return NULL;
//
//    map->size = INITIAL_SIZE;
//    map->count = 0;
//    map->buckets = (Entry**)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(Entry*) * map->size, 'bckt');
//    if (!map->buckets) {
//        ExFreePoolWithTag(map, 'map1');
//        return NULL;
//    }
//
//    for (size_t i = 0; i < map->size; i++) {
//        map->buckets[i] = NULL;
//    }
//
//    return map;

BOOLEAN
StringStartsWithAnyPrefix(const PUNICODE_STRING target, const PCWSTR* PrefixList, const int count)
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

NTSTATUS
GetProcessNameByPid(HANDLE pid, PUNICODE_STRING* ProcessName) {
    // Check if its mapped
    struct SotaProcess* processStruct = find_process((int)(ULONG_PTR)pid);
    if (processStruct != NULL) {
        ProcessName = &processStruct->Pname;
        return STATUS_SUCCESS;
    }

    // Get name from system
    PEPROCESS eProcess = NULL;
    NTSTATUS status = PsLookupProcessByProcessId(pid, &eProcess);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // Get the process image name
    status = SeLocateProcessImageName(eProcess, ProcessName);
    ObDereferenceObject(eProcess);  // Always dereference after using the EPROCESS object

    return status;
}

void mapProcessToItself(_In_ HANDLE Process) { //DbgPrint("[SOTA] mapProcess: Mapping parent to itself in PidTable.\n");
    PUNICODE_STRING processName = NULL;
    NTSTATUS status = GetProcessNameByPid((HANDLE)(ULONG_PTR)Process, &processName);
    if (!NT_SUCCESS(status)) {
        DbgPrint("[SOTA] mapProcessToItself: Could not get process name. - PID: %d - STATUS: 0x%x\n", (ULONG)(ULONG_PTR)Process, status);
        return;
    }
    // Mapping parent it to itself
    add_pfamily((int)(ULONG_PTR)Process, (int)(ULONG_PTR)Process);
    add_process((int)(ULONG_PTR)Process, (int)(ULONG_PTR)Process, processName);
    DbgPrint("[SOTA] mapProcessToItself: New process mapped to itself. PID: %d - PROCESSNAME: %wZ\n", (ULONG)(ULONG_PTR)Process, processName);
}

NTSTATUS mapProcessToParent(
    _In_ HANDLE Parent,
    _In_ HANDLE Process
) {
    //DbgPrint("[SOTA] mapProcessToParent: Begin.\n");

    struct SotaProcess* parentStruct;
    struct SotaProcess* processStruct;
    struct SotaPFamily* parentPfamily;
    struct SotaPFamily* processPfamily;

    NTSTATUS status = STATUS_SUCCESS;

    //Check if parent is mapped
    parentStruct = find_process((int)(ULONG_PTR)Parent);

    // Parent Mapping
    if (!parentStruct) {
        mapProcessToItself(Parent);
        parentStruct = find_process((int)(ULONG_PTR)Parent);
    }

    if (parentStruct == NULL) {
        DbgPrint("[SOTA] mapProcessToParent: FATAL ERROR Could not find parent on PidTable. - PID: %d\n", (int)(ULONG_PTR)Parent);
        return STATUS_SUCCESS;
    }
    parentPfamily = find_pfamily(parentStruct->PFid);
    if (parentPfamily == NULL) {
        DbgPrint("[SOTA] mapProcessToParent: FATAL ERROR Could not find parent on PFidTable. - PARENTPID: %d - PFID: %d - PARENTNAME: %wZ\n", (int)(ULONG_PTR)Parent, parentStruct->PFid, parentStruct->Pname);
        return STATUS_SUCCESS;
    }

    // Process Mapping or updating
    PUNICODE_STRING processName = NULL;
    status = GetProcessNameByPid((HANDLE)(ULONG_PTR)Process, &processName);
    if (!NT_SUCCESS(status)) {
        DbgPrint("[SOTA] mapProcessToParent: Could not get current process name. - PID: %d - STATUS: 0x%x\n", (ULONG)(ULONG_PTR)Process, status);
        return STATUS_SUCCESS;
    }
    if (!StringStartsWithAnyPrefix(parentStruct->Pname, SafeOrigins, SAFEORIGINS_COUNT)) {
        add_pfamily(parentStruct->PFid, (int)(ULONG_PTR)Process);
        add_process((int)(ULONG_PTR)Process, parentStruct->PFid, processName);
    }
    else {
        add_pfamily((int)(ULONG_PTR)Process, (int)(ULONG_PTR)Process);
        add_process((int)(ULONG_PTR)Process, (int)(ULONG_PTR)Process, processName);
    }

    processStruct = find_process((int)(ULONG_PTR)Process);
    if (processStruct == NULL) {
        DbgPrint("[SOTA] mapProcessToParent: FATAL ERROR Could not find process on PidTable. - PID: %d\n", (int)(ULONG_PTR)Process);
        return STATUS_SUCCESS;
    }
    processPfamily = find_pfamily(processStruct->PFid);
    if (parentPfamily == NULL) {
        DbgPrint("[SOTA] mapProcessToParent: FATAL ERROR Could not find process on PFidTable. - PID: %d - PROCESSNAME: %wZ\n", (int)(ULONG_PTR)Process, processStruct->Pname);
        return STATUS_SUCCESS;
    }
    DbgPrint("[SOTA] mapProcessToParent: New process mapped. PID: %d - PROCESSNAME: %wZ - PFID: %d\n", processStruct->Pid, processStruct->Pname, processStruct->PFid);

    return STATUS_SUCCESS;
}

NTSTATUS unmapProcess(
    _In_ HANDLE Process
) {
    //DbgPrint("[SOTA] unmapProcess: Begin.\n");

    struct SotaProcess* processStruct;
    struct SotaPFamily* processPfamily;

    processStruct = find_process((int)(ULONG_PTR)Process);
    if (processStruct == NULL) {
        DbgPrint("[SOTA] unmapProcess: Process is not mapped on PidTable, returning success. - PID: %d\n", (ULONG)(ULONG_PTR)Process);
        return STATUS_SUCCESS;
    }
    else {
        processPfamily = find_pfamily(processStruct->PFid);
        delete_process(processStruct);

        if (processPfamily == NULL) {
            DbgPrint("[SOTA] unmapProcess: Process is not mapped on PFidTable, returning success. - PID: %d - PROCESSNAME: %wZ\n", (ULONG)(ULONG_PTR)Process, processStruct->Pname);
            return STATUS_SUCCESS;
        }
        else {
            delete_pfamily(processPfamily, (int)(ULONG_PTR)Process);
        }
    }

    DbgPrint("[SOTA] unmapProcess: Process Unmaped. - PID: %d - PFID: %d - PROCESSNAME: %wZ\n", processStruct->Pid, processStruct->PFid, processStruct->Pname);

    return STATUS_SUCCESS;
}

void clean_tables() {
    clean_pidtable();
    clean_pfidtable();
}

NTSTATUS kill_pfamily(int pid) {
    NTSTATUS status = STATUS_SUCCESS;
    struct SotaProcess* processStruct = NULL;
    struct SotaPFamily* processPfamily = NULL;
    processStruct = find_process(pid);
    if (processStruct == NULL) {
        DbgPrint("[SOTA] kill_pfamily: Could not find the pid specified. - PID: %d\n", pid);
        return status;
    }
    processPfamily = find_pfamily(processStruct->PFid);
    if (processPfamily == NULL) {
        DbgPrint("[SOTA] kill_pfamily: Could not find the pfid specified. - PFID: %d - PROCESSNAME: %wZ\n", processStruct->PFid, processStruct->Pname);
        return status;
    }

    PidList* current_pids;
    PidList* tmplist;

    DbgPrint("[SOTA-KILL] kill_pfamily: Killing Family PFID: %d\n", processPfamily->PFid);
    DL_FOREACH_SAFE(processPfamily->Pids, current_pids, tmplist) {
        //processStruct = find_process(current_pids->Pid);
        //DbgPrint("[SOTA-KILL] kill_pfamily: Killing Process with PID: %d - ProcessName: % - PROCESSNAME: %wZ\n", current_pids->Pid, processStruct->Pname);
        //if (StringStartsWithAnyPrefix(processStruct->Pname, SafeOrigins, SAFEORIGINS_COUNT)) {
        //    DbgPrint("[SOTA-KILL] kill_pfamily: Process on SafeList PID: %d - ProcessName: % - PROCESSNAME: %wZ\n", current_pids->Pid, processStruct->Pname);
        //    continue;
        //}

        // Get a handle to the process using ZwOpenProcess
        HANDLE ProcessHandle;
        OBJECT_ATTRIBUTES ObjectAttributes;
        CLIENT_ID ClientId = { (HANDLE)current_pids->Pid, 0 };

        InitializeObjectAttributes(&ObjectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
        status = ZwOpenProcess(&ProcessHandle, PROCESS_ALL_ACCESS, &ObjectAttributes, &ClientId);
        if (!NT_SUCCESS(status))
        {
            DbgPrint("[SOTA-KILL] kill_pfamily: Failed to open process handle: %d - PROCESSNAME: %wZ\n", current_pids->Pid, processStruct->Pname);
            return status;
        }

        // Use ZwTerminateProcess to terminate the process
        status = ZwTerminateProcess(ProcessHandle, STATUS_SUCCESS); // Here you should use a valid status code or just STATUS_SUCCESS
        ZwClose(ProcessHandle); // Close the handle
        if (NT_SUCCESS(status))
        {
            DbgPrint("[SOTA-KILL] kill_pfamily: Process killed. PID: %d - PROCESSNAME: %wZ\n", current_pids->Pid, processStruct->Pname);
        }
    }

    DbgPrint("[SOTA-KILL] kill_pfamily: End - PFID: %d\n", processPfamily->PFid);
    return status;
}

