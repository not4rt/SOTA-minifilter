#pragma once

#include "SotaHash.h"

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
