//#include "MyHashMap.h"
//#include <ntddk.h>
//
//// Simple hash function.
////static unsigned int hash(int key) {
////    return key % INITIAL_SIZE;
////}
//
////
//// Keylist functions
////
//PidList* PidList_initialize() {
//    DbgPrint("[SOTA] PidList_initialize.\n");
//    PidList* list = (PidList*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PidList), 'vlst');
//    if (!list) {
//        DbgPrint("[SOTA] PidList_initialize: Failed to allocate list memory.\n");
//        return NULL;
//    }
//
//    list->count = 0;
//    list->capacity = 10;  // Initial capacity
//    list->values = (ULONG*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(ULONG) * list->capacity, 'varr');
//
//    if (!list->values) {
//        DbgPrint("[SOTA] PidList_initialize: Failed to allocate values memory.\n");
//        ExFreePoolWithTag(list, 'vlst');
//        return NULL;
//    }
//
//    return list;
//}
//
//void PidList_free(PidList* list) {
//    DbgPrint("[SOTA] PidList_free.\n");
//    if (list) {
//        if (list->values) {
//            ExFreePoolWithTag(list->values, 'varr');
//        }
//        ExFreePoolWithTag(list, 'vlst');
//    }
//}
//
//NTSTATUS PidList_insert(PidList* list, ULONG value) {
//    DbgPrint("[SOTA] PidList_insert. - pid: %d\n", value);
//    UNREFERENCED_PARAMETER(list);
//    //if (list->count == list->capacity) {
//    //    size_t new_capacity = list->capacity * 2;
//    //    ULONG* new_values = (ULONG*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(ULONG) * new_capacity, 'varr');
//    //    if (!new_values) {
//    //        DbgPrint("[SOTA] PidList_insert: Failed to allocate new list memory- pid: %d\n", value);
//    //        return STATUS_INSUFFICIENT_RESOURCES;
//    //    }
//
//    //    RtlCopyMemory(new_values, list->values, sizeof(ULONG) * list->count);
//    //    ExFreePoolWithTag(list->values, 'varr');
//    //    list->values = new_values;
//    //    list->capacity = new_capacity;
//    //}
//
//    //list->values[list->count++] = value;
//    return STATUS_SUCCESS;
//}
//
//NTSTATUS PidList_remove(PidList* list, ULONG pid) {
//    DbgPrint("[SOTA] PidList_remove. - pid: %d\n", pid);
//    for (size_t i = 0; i < list->count; i++) {
//        if (list->values[i] == pid) {
//            list->values[i] = list->values[list->count - 1];
//            list->count--;
//        }
//    }
//
//    return STATUS_SUCCESS;
//}
//
//
//
//// PFID Functions
//PFidHashMap* PFidMap_initialize() {
//    DbgPrint("[SOTA] PFidMap_initialize.\n");
//    PFidHashMap* map = (PFidHashMap*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PFidHashMap), 'map1');
//    if (!map) {
//        DbgPrint("[SOTA] PFidMap_initialize: Failed to allocate memory for HashMap.\n");
//        return NULL;
//    }
//
//    map->size = INITIAL_SIZE;
//    map->count = 0;
//    map->buckets = (PFidEntry**)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PFidEntry*) * map->size, 'bckt');
//    if (!map->buckets) {
//        DbgPrint("[SOTA] PFidMap_initialize: Failed to allocate memory for buckets.\n");
//        ExFreePoolWithTag(map, 'map1');
//        return NULL;
//    }
//
//    for (size_t i = 0; i < map->size; i++) {
//        map->buckets[i] = NULL;
//    }
//
//    KeInitializeSpinLock(&map->lock);
//
//    return map;
//}
//
//void PFidMap_free(PFidHashMap* map) {
//    DbgPrint("[SOTA] PFidMap_free.\n");
//    for (size_t i = 0; i < map->size; i++) {
//        PFidEntry* current = map->buckets[i];
//        while (current) {
//            PFidEntry* temp = current;
//            current = current->next;
//            ExFreePoolWithTag(temp, 'pfi1');
//        }
//    }
//    ExFreePoolWithTag(map->buckets, 'bckt');
//    ExFreePoolWithTag(map, 'map1');
//}
//
//NTSTATUS PFidMap_insert_pfid(PFidHashMap* map, ULONG pfid) {
//    DbgPrint("[SOTA] PFidMap_insert_pfid: - pfid: %d \n", pfid);
//    KIRQL irql = KeGetCurrentIrql();
//    KeAcquireSpinLock(&map->lock, &irql);
//    PFidEntry* new_entry = (PFidEntry*)ExAllocatePool2(POOL_FLAG_NON_PAGED_EXECUTE, sizeof(PFidEntry), 'pfi1');
//    if (!new_entry) {
//        DbgPrint("[SOTA] PFidMap_insert_pfid: Failed to allocate memory for new entry. - pfid: %d \n", pfid);
//        KeReleaseSpinLock(&map->lock, irql);
//        return STATUS_UNSUCCESSFUL;
//    }
//    new_entry->PFid = pfid;
//    new_entry->Pids = PidList_initialize();
//    new_entry->next = NULL;
//
//    if (!map->buckets[map->count]) {
//        map->buckets[map->count] = new_entry;
//    }
//    else {
//        PFidEntry* current = map->buckets[map->count];
//        while (current->next) {
//            current = current->next;
//        }
//        current->next = new_entry;
//
//        map->count++;
//    }
//
//
//    //// Resize logic
//    //if ((double)map->count / map->size > LOAD_FACTOR) {
//    //    size_t new_size = map->size * 2;
//    //    Entry** new_buckets = (Entry**)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(Entry*) * new_size, 'bckt');
//    //    if (!new_buckets) return STATUS_UNSUCCESSFUL;
//
//    //    for (size_t i = 0; i < new_size; i++) {
//    //        new_buckets[i] = NULL;
//    //    }
//
//    //    for (size_t i = 0; i < map->size; i++) {
//    //        Entry* current = map->buckets[i];
//    //        while (current) {
//    //            unsigned int new_idx = current->key % new_size;
//    //            Entry* next = current->next;
//    //            current->next = new_buckets[new_idx];
//    //            new_buckets[new_idx] = current;
//    //            current = next;
//    //        }
//    //    }
//
//    //    ExFreePoolWithTag(map->buckets, 'bckt');
//    //    map->buckets = new_buckets;
//    //    map->size = new_size;
//    //}
//
//    KeReleaseSpinLock(&map->lock, irql);
//    return STATUS_SUCCESS;
//}
//
//NTSTATUS PFidMap_remove_pfid(PFidHashMap* map, ULONG pfid) {
//    DbgPrint("[SOTA] PFidMap_remove_pfid. - pfid: %d\n", pfid);
//    KIRQL irql = KeGetCurrentIrql();
//    KeAcquireSpinLock(&map->lock, &irql);
//    PFidEntry* current = map->buckets[0];
//    PFidEntry* prev = NULL;
//    while (current) {
//        if (current->PFid == pfid) {
//            if (prev == NULL) {
//                map->buckets[0] = current->next;
//            }
//            else {
//                prev->next = current->next;
//            }
//            ExFreePoolWithTag(current, 'pfi1');
//            map->count--;
//            KeReleaseSpinLock(&map->lock, irql);
//            DbgPrint("[SOTA] PFidMap_remove_pfid: Found and removed. pfid: %d\n", pfid);
//            return STATUS_SUCCESS; // Removed
//        }
//        prev = current;
//        current = current->next;
//    }
//
//    KeReleaseSpinLock(&map->lock, irql);
//
//    DbgPrint("[SOTA] PFidMap_remove_pfid: PFID not found. - PFID: %d\n", pfid);
//    return STATUS_UNSUCCESSFUL;  // Not found
//}
//
//NTSTATUS PFidMap_remove_pid(PFidHashMap* map, ULONG pfid, ULONG pid) {
//    DbgPrint("[SOTA] PFidMap_remove_pid: - pfid: %d - pid: %d\n", pfid, pid);
//    KIRQL irql = KeGetCurrentIrql();
//    KeAcquireSpinLock(&map->lock, &irql);
//    PFidEntry* current = map->buckets[0];
//    PFidEntry* prev = NULL;
//    while (current) {
//        if (current->PFid == pfid) {
//            PidList_remove(current->Pids, pid);
//            if (current->Pids->count == 0) {
//                DbgPrint("[SOTA] PFidMap_remove_pid: PidList is empty, removing family. pfid: %d - pid: %d\n", pfid, pid);
//                PFidMap_remove_pfid(map, pfid);
//            }
//            KeReleaseSpinLock(&map->lock, irql);
//            DbgPrint("[SOTA] PFidMap_remove_pid: Found and removed. - pfid: %d - pid: %d\n", pfid, pid);
//            return STATUS_SUCCESS; // Removed
//        }
//        prev = current;
//        current = current->next;
//    }
//
//    KeReleaseSpinLock(&map->lock, irql);
//    DbgPrint("[SOTA] PFidMap_delete: PFid not found- pfid: %d\n", pfid);
//    return STATUS_UNSUCCESSFUL;  // Not found
//}
//
//PFidEntry* PFidMap_search(PFidHashMap* map, ULONG pfid) {
//    DbgPrint("[SOTA] PFidMap_search. - pfid: %d\n", pfid);
//    PFidEntry* current = map->buckets[0];
//    while (current) {
//        if (current->PFid == pfid) {
//            return current;
//        }
//        current = current->next;
//    }
//    return NULL;  // Not found
//}
//
//NTSTATUS PFidMap_insert_pid(PFidHashMap* map, ULONG pfid, ULONG pid) {
//    DbgPrint("[SOTA] PFidMap_insert_pid. - pfid: %d - pid: %d\n", pfid, pid);
//    NTSTATUS status;
//    status = PidList_insert(PFidMap_search(map, pfid)->Pids, pid);
//    return status;
//}
//
////
//// PID Functions
////
//PidHashMap* PidMap_initialize() {
//    DbgPrint("[SOTA] PidMap_initialize.\n");
//    PidHashMap* map = (PidHashMap*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PidHashMap), 'map1');
//    if (!map) {
//        DbgPrint("[SOTA] initialize_map: Failed to allocate memory for HashMap.\n");
//        return NULL;
//    }
//
//    map->size = INITIAL_SIZE;
//    map->count = 0;
//    map->buckets = (PidEntry**)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PidEntry*) * map->size, 'bckt');
//    if (!map->buckets) {
//        DbgPrint("[SOTA] initialize_map: Failed to allocate memory for buckets.\n");
//        ExFreePoolWithTag(map, 'map1');
//        return NULL;
//    }
//
//    for (size_t i = 0; i < map->size; i++) {
//        map->buckets[i] = NULL;
//    }
//
//    KeInitializeSpinLock(&map->lock);
//
//    return map;
//}
//
//void PidMap_free(PidHashMap* map) {
//    DbgPrint("[SOTA] PidMap_free.\n");
//    for (size_t i = 0; i < map->size; i++) {
//        PidEntry* current = map->buckets[i];
//        while (current) {
//            PidEntry* temp = current;
//            current = current->next;
//            ExFreePoolWithTag(temp, 'pid1');
//        }
//    }
//    ExFreePoolWithTag(map->buckets, 'bckt');
//    ExFreePoolWithTag(map, 'map1');
//}
//
//
//NTSTATUS PidMap_insert(PidHashMap* map, ULONG pid, ULONG pfid) {
//    DbgPrint("[SOTA] PidMap_insert: pid: %d - pfid: %d\n", pid, pfid);
//    KIRQL irql = KeGetCurrentIrql();
//    KeAcquireSpinLock(&map->lock, &irql);
//    PidEntry* new_entry = (PidEntry*)ExAllocatePool2(POOL_FLAG_NON_PAGED_EXECUTE, sizeof(PidEntry), 'pid1');
//    if (!new_entry) {
//        DbgPrint("[SOTA] PidMap_insert: Failed to allocate memory for new_entry. - pid: %d - pfid: %d\n", pid, pfid);
//        KeReleaseSpinLock(&map->lock, irql);
//        return STATUS_UNSUCCESSFUL;
//    }
//    new_entry->Pid = pid;
//    new_entry->PFid = pfid;
//    new_entry->next = NULL;
//
//    if (!map->buckets[pid]) {
//        map->buckets[map->count] = new_entry;
//    }
//    else {
//        PidEntry* current = map->buckets[pid];
//        while (current->next) {
//            current = current->next;
//        }
//        current->next = new_entry;
//    }
//
//    map->count++;
//
//    //// Resize logic
//    //if ((double)map->count / map->size > LOAD_FACTOR) {
//    //    size_t new_size = map->size * 2;
//    //    Entry** new_buckets = (Entry**)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(Entry*) * new_size, 'bckt');
//    //    if (!new_buckets) return STATUS_UNSUCCESSFUL;
//
//    //    for (size_t i = 0; i < new_size; i++) {
//    //        new_buckets[i] = NULL;
//    //    }
//
//    //    for (size_t i = 0; i < map->size; i++) {
//    //        Entry* current = map->buckets[i];
//    //        while (current) {
//    //            unsigned int new_idx = current->key % new_size;
//    //            Entry* next = current->next;
//    //            current->next = new_buckets[new_idx];
//    //            new_buckets[new_idx] = current;
//    //            current = next;
//    //        }
//    //    }
//
//    //    ExFreePoolWithTag(map->buckets, 'bckt');
//    //    map->buckets = new_buckets;
//    //    map->size = new_size;
//    //}
//
//    KeReleaseSpinLock(&map->lock, irql);
//    return STATUS_SUCCESS;
//}
//
//NTSTATUS PidMap_remove(PidHashMap* map, ULONG pid) {
//    DbgPrint("[SOTA] PidMap_remove: pid: %d\n", pid);
//    KIRQL irql = KeGetCurrentIrql();
//    KeAcquireSpinLock(&map->lock, &irql);
//    PidEntry* current = map->buckets[0];
//    PidEntry* prev = NULL;
//    while (current) {
//        if (current->Pid == pid) {
//            if (prev == NULL) {
//                map->buckets[0] = current->next;
//            }
//            else {
//                prev->next = current->next;
//            }
//            ExFreePoolWithTag(current, 'pid1');
//            map->count--;
//            KeReleaseSpinLock(&map->lock, irql);
//            DbgPrint("[SOTA] PidMap_remove: Found and removed. pid: %d\n", pid);
//            return STATUS_SUCCESS; // Removed
//        }
//        prev = current;
//        current = current->next;
//    }
//
//    KeReleaseSpinLock(&map->lock, irql);
//
//    DbgPrint("[SOTA] PidMap_remove: PID not found. - pid: %d\n", pid);
//    return STATUS_UNSUCCESSFUL;  // Not found
//}
//
//ULONG PidMap_search(PidHashMap* map, ULONG pid) {
//    DbgPrint("[SOTA] PidMap_search: pid: %d\n", pid);
//    PidEntry* current = map->buckets[pid];
//    while (current) {
//        DbgPrint("[SOTA] PidMap_search: current: %d\n", current->Pid);
//        if (current->Pid == pid) {
//            return current->PFid;
//        }
//        current = current->next;
//    }
//    return 0;  // Not found
//}
