#pragma once

#include "SotaHash.h"


NTSTATUS kill_pfamily1(int pid) {
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
        DbgPrint("[SOTA-KILL] kill_pfamily: Killing Process with PID: %d - ProcessName: % - PROCESSNAME: %wZ\n", current_pids->Pid, processStruct->Pname);

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
//
// PIDTABLE
//
struct SotaProcess* PidTable3 = NULL;
void add_process(int pid, int pfid, PUNICODE_STRING pname) {
    struct SotaProcess *p;

    HASH_FIND_INT(PidTable3, &pid, p);  /* pid already in the hash? */

    if (p == NULL) {
        p = (struct SotaProcess*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(struct SotaProcess), 'hsh1');
        p->Pid = pid;
        HASH_ADD_INT(PidTable3, Pid, p);  /* Pid is the key field */
    }
    p->Pname = pname;
    p->PFid = pfid;
}

struct SotaProcess* find_process(int pid) {
    struct SotaProcess* p;

    HASH_FIND_INT(PidTable3, &pid, p);  /* p: output pointer */
    return p;
}

void delete_process(struct SotaProcess* process) {
    HASH_DEL(PidTable3, process);  /* process: pointer to delete */
    ExFreePool(process);
}

void clean_pidtable() {
    struct SotaProcess* current_p;
    struct SotaProcess* tmp;

    HASH_ITER(hh, PidTable3, current_p, tmp) {
        HASH_DEL(PidTable3, current_p);  /* delete it (PidTable advances to next) */
        ExFreePool(current_p);             /* free it */
    }
}

//
// PfidTable + PidList
//

struct SotaPFamily* PFidTable = NULL;
void add_pfamily(int pfid, int pid) {
    struct SotaPFamily* pf;

    HASH_FIND_INT(PFidTable, &pfid, pf);  /* pfid already in the hash? */
    if (pf == NULL) {
        pf = (struct SotaPFamily*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(struct SotaPFamily), 'hsh2');
        pf->PFid = pfid;
        HASH_ADD_INT(PFidTable, PFid, pf);  /* PFid is the key field */
        pf->Pids = NULL; /* Initialize PidList*/

        pf->COUNT_IRP_MJ_CREATE = 0;
        pf->UniqueDirsCreate = NULL; /* Initialize UniqueDirs HashMap*/

        pf->COUNT_IRP_MJ_READ = 0;
        pf->UniqueDirsREAD = NULL; /* Initialize UniqueDirs HashMap*/

        pf->COUNT_IRP_MJ_SET_INFORMATION = 0;
        pf->UniqueDirsSET_INFORMATION = NULL; /* Initialize UniqueDirs HashMap*/

        pf->COUNT_IRP_MJ_WRITE = 0;
        pf->UniqueDirsWRITE = NULL; /* Initialize UniqueDirs HashMap*/
    }
    PidList *pids = (PidList*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(*pids), 'lst2');
    if (pids != NULL) {
        pids->Pid = pid;
        //DbgPrint("[SOTA] add_pfamily: Appending entry. - PFID: %d - PID: %d\n", pf->PFid, pids->Pid);
        DL_APPEND(pf->Pids, pids);
    }
    else {
        DbgPrint("[SOTA] add_pfamily: Could not initialize list. - PFID: %d\n", pf->PFid);
    }
}
struct SotaPFamily* find_pfamily(int pfid) {
    struct SotaPFamily* pf;

    HASH_FIND_INT(PFidTable, &pfid, pf);  /* pf: output pointer */
    return pf;
}

void countIRP_pfamily(struct SotaPFamily* pfamily, PUNICODE_STRING file, ULONG irp_type) {
    struct UniqueDirectoriesAccessed* unidirs;

    switch (irp_type) {
        case 0: {
            //CREATE
            HASH_FIND_PTR(pfamily->UniqueDirsCreate, &file, unidirs);  /* pid already in the hash? */

            if (unidirs == NULL) {
                unidirs = (struct UniqueDirectoriesAccessed*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(struct UniqueDirectoriesAccessed), 'hsh3');
                unidirs->UniqueDirectory = file;
                HASH_ADD_PTR(pfamily->UniqueDirsCreate, UniqueDirectory, unidirs);  /* Pid is the key field */
                pfamily->COUNT_IRP_MJ_CREATE++;
                if (pfamily->COUNT_IRP_MJ_CREATE > 1000) {
                    //DbgPrint("[SOTA] countIRP_pfamily: Family made more than 100 unique COUNT_IRP_MJ_CREATE. - PFID: %d - FileName: %wZ\n", pfamily->PFid, file);
                    //pfamily->COUNT_IRP_MJ_READ = 0;
                    if (pfamily->COUNT_IRP_MJ_WRITE > 500 && pfamily->COUNT_IRP_MJ_SET_INFORMATION > 50) {
                        DbgPrint("[SOTA] Rate Limit Create %d. - PFID: %d - FileName: %wZ - READ: %d - CREATE: %d - WRITE: %d - SETINFORMATION: %d\n", pfamily->Pids->Pid, pfamily->PFid, file, pfamily->COUNT_IRP_MJ_READ, pfamily->COUNT_IRP_MJ_CREATE, pfamily->COUNT_IRP_MJ_WRITE, pfamily->COUNT_IRP_MJ_SET_INFORMATION);
                        kill_pfamily1(pfamily->Pids->Pid);
                    }
                }
            }
        }
        case 1: {
            //READ
            HASH_FIND_PTR(pfamily->UniqueDirsREAD, &file, unidirs);  /* pid already in the hash? */

            if (unidirs == NULL) {
                unidirs = (struct UniqueDirectoriesAccessed*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(struct UniqueDirectoriesAccessed), 'hsh3');
                unidirs->UniqueDirectory = file;
                HASH_ADD_PTR(pfamily->UniqueDirsREAD, UniqueDirectory, unidirs);  /* Pid is the key field */
                pfamily->COUNT_IRP_MJ_READ++;
                if (pfamily->COUNT_IRP_MJ_READ > 1000) {
                    //DbgPrint("[SOTA] countIRP_pfamily: Family made more than 100 unique COUNT_IRP_MJ_READ. - PFID: %d - FileName: %wZ\n", pfamily->PFid, file);
                    //pfamily->COUNT_IRP_MJ_READ = 0;
                    if (pfamily->COUNT_IRP_MJ_WRITE > 50 && pfamily->COUNT_IRP_MJ_SET_INFORMATION > 50) {
                        DbgPrint("[SOTA] Rate Limit READ %d. - PFID: %d - FileName: %wZ - READ: %d - CREATE: %d - WRITE: %d - SETINFORMATION: %d\n", pfamily->Pids->Pid, pfamily->PFid, file, pfamily->COUNT_IRP_MJ_READ, pfamily->COUNT_IRP_MJ_CREATE, pfamily->COUNT_IRP_MJ_WRITE, pfamily->COUNT_IRP_MJ_SET_INFORMATION);
                        kill_pfamily1(pfamily->Pids->Pid);
                    }
                }
            }
        }
        case 2: {
            //WRITE
            HASH_FIND_PTR(pfamily->UniqueDirsWRITE, &file, unidirs);  /* pid already in the hash? */

            if (unidirs == NULL) {
                unidirs = (struct UniqueDirectoriesAccessed*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(struct UniqueDirectoriesAccessed), 'hsh3');
                unidirs->UniqueDirectory = file;
                HASH_ADD_PTR(pfamily->UniqueDirsWRITE, UniqueDirectory, unidirs);  /* Pid is the key field */
                pfamily->COUNT_IRP_MJ_WRITE++;
                if (pfamily->COUNT_IRP_MJ_WRITE > 5) {
                    //DbgPrint("[SOTA] countIRP_pfamily: Family made more than 50 unique IRP_MJ_WRITE. - PFID: %d - FileName: %wZ\n", pfamily->PFid, file);
                    //pfamily->COUNT_IRP_MJ_WRITE = 0;
                    DbgPrint("[SOTA] Rate Limit WRITE PID %d. - PFID: %d - FileName: %wZ - READ: %d - CREATE: %d - WRITE: %d - SETINFORMATION: %d\n", pfamily->Pids->Pid, pfamily->PFid, file, pfamily->COUNT_IRP_MJ_READ, pfamily->COUNT_IRP_MJ_CREATE, pfamily->COUNT_IRP_MJ_WRITE, pfamily->COUNT_IRP_MJ_SET_INFORMATION);
                    kill_pfamily1(pfamily->Pids->Pid);
                }
            }
        }
        case 3: {
            //SET_INFORMATION
            HASH_FIND_PTR(pfamily->UniqueDirsSET_INFORMATION, &file, unidirs);  /* pid already in the hash? */

            if (unidirs == NULL) {
                unidirs = (struct UniqueDirectoriesAccessed*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(struct UniqueDirectoriesAccessed), 'hsh3');
                unidirs->UniqueDirectory = file;
                HASH_ADD_PTR(pfamily->UniqueDirsSET_INFORMATION, UniqueDirectory, unidirs);  /* Pid is the key field */
                pfamily->COUNT_IRP_MJ_SET_INFORMATION++;
                if (pfamily->COUNT_IRP_MJ_SET_INFORMATION > 10) {
                    //DbgPrint("[SOTA] countIRP_pfamily: Family made more than 50 unique COUNT_IRP_MJ_SET_INFORMATION. - PFID: %d - FileName: %wZ\n", pfamily->PFid, file);
                    //pfamily->COUNT_IRP_MJ_SET_INFORMATION = 0;
                    if (pfamily->COUNT_IRP_MJ_READ > 10 && pfamily->COUNT_IRP_MJ_WRITE > 10) {
                        DbgPrint("[SOTA] Rate Limit SET_INFORMATION PID %d. - PFID: %d - FileName: %wZ - READ: %d - CREATE: %d - WRITE: %d - SETINFORMATION: %d\n", pfamily->Pids->Pid, pfamily->PFid, file, pfamily->COUNT_IRP_MJ_READ, pfamily->COUNT_IRP_MJ_CREATE, pfamily->COUNT_IRP_MJ_WRITE, pfamily->COUNT_IRP_MJ_SET_INFORMATION);
                        kill_pfamily1(pfamily->Pids->Pid);
                    }
                }
            }
        }
    }
    return;
}

void delete_pfamily(struct SotaPFamily* pfamily, int pid) {
    PidList* current_pids;
    PidList* tmp;
    DL_FOREACH_SAFE(pfamily->Pids, current_pids, tmp) {
        if(current_pids->Pid == pid) {
            //DbgPrint("[SOTA] delete_pfamily: Deleting entry. - PFID: %d - PID: %d\n", pfamily->PFid, current_pids->Pid);
            DL_DELETE(pfamily->Pids, current_pids);
            ExFreePool(current_pids);
            break;
        }
    }

    int count = 0;
    DL_COUNT(pfamily->Pids, current_pids, count);
    if (count == 0) {
        //DbgPrint("[SOTA] delete_pfamily: Count reach 0, deleting PFidTableEntry. - PFID: %d\n", pfamily->PFid);

        UniqueDirectoriesAccessed* current;
        UniqueDirectoriesAccessed* tmpdir;
        HASH_ITER(hh, PFidTable->UniqueDirsCreate, current, tmpdir) {
            HASH_DEL(PFidTable->UniqueDirsCreate, current);
            ExFreePool(current);
        }
        HASH_ITER(hh, PFidTable->UniqueDirsREAD, current, tmpdir) {
            HASH_DEL(PFidTable->UniqueDirsREAD, current);
            ExFreePool(current);
        }
        HASH_ITER(hh, PFidTable->UniqueDirsSET_INFORMATION, current, tmpdir) {
            HASH_DEL(PFidTable->UniqueDirsSET_INFORMATION, current);
            ExFreePool(current);
        }
        HASH_ITER(hh, PFidTable->UniqueDirsWRITE, current, tmpdir) {
            HASH_DEL(PFidTable->UniqueDirsWRITE, current);
            ExFreePool(current);
        }

        HASH_DEL(PFidTable, pfamily);
        ExFreePool(pfamily);
    }
}

void clean_pfidtable() {
    struct SotaPFamily* current_pf;
    struct SotaPFamily* tmp;
    PidList* current_pids;
    PidList* tmplist;

    HASH_ITER(hh, PFidTable, current_pf, tmp) {
        DL_FOREACH_SAFE(current_pf->Pids, current_pids, tmplist) {
            DL_DELETE(current_pf->Pids, current_pids);
            ExFreePool(current_pids);
        }
        HASH_DEL(PFidTable, current_pf);  /* delete it (PidTable advances to next) */
        ExFreePool(current_pf);             /* free it */
    }
}

