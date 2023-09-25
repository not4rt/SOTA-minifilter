//#pragma once
//
///*++
//
//Module Name:
//
//	HashMap.h
//
//Abstract:
//
//	Header file which contains the type definitions and global variables 
//	of the HashMap used by the Fpid-Pid map. Mainly used by SotaMain module.
//
//Environment:
//
//	Kernel mode
//
//--*/
//
//#ifndef HASHMAP_H
//#define HASHMAP_H
//
//#include <ntddk.h>
//
//#define INITIAL_SIZE 100
//
//
//// This structure will store a dynamic list of integers (Values).
//typedef struct {
//    ULONG* values;
//    size_t count;
//    size_t capacity;
//} PidList;
//
//PidList* PidList_initialize();
//void PidList_free(PidList* list);
//NTSTATUS PidList_insert(PidList* list, ULONG value);
//
////
////PFID HASHMAP
////
//typedef struct PFidEntry {
//    ULONG PFid;
//    PidList* Pids;
//    struct PFidEntry* next;
//} PFidEntry;
//
//struct PFidHashMap {
//    PFidEntry** buckets;
//    size_t size; // total size of the map
//    size_t count; // actual size of the map
//    KSPIN_LOCK lock;
//};  
//
//
//typedef struct PFidHashMap PFidHashMap;
//
//PFidHashMap* PFidMap_initialize(void);
//void PFidMap_free(
//    _In_ PFidHashMap* map
//);
//NTSTATUS PFidMap_insert_pfid(
//    _In_ PFidHashMap* map,
//    _In_ ULONG pfid);
//NTSTATUS PFidMap_remove_pid(
//    _In_ PFidHashMap* map,
//    _In_ ULONG pfid,
//    _In_ ULONG pid);
//PFidEntry* PFidMap_search(
//    _In_ PFidHashMap* map,
//    _In_ ULONG pfid);
//NTSTATUS PFidMap_insert_pid(
//    _In_ PFidHashMap* map,
//    _In_ ULONG pfid,
//    _In_ ULONG pid);
//NTSTATUS PFidMap_remove_pfid(
//    _In_ PFidHashMap* map,
//    _In_ ULONG pfid);
//NTSTATUS PFidMap_remove_pid(
//    _In_ PFidHashMap* map,
//    _In_ ULONG pfid,
//    _In_ ULONG pid);
//
////
////PID HASHMAP
////
//typedef struct PidEntry {
//    ULONG Pid;
//    ULONG PFid;
//    struct PidEntry* next;
//} PidEntry;
//
//struct PidHashMap {
//    PidEntry** buckets;
//    size_t size; // total size of the map
//    size_t count; // actual size of the map
//    KSPIN_LOCK lock;
//};
//
//typedef struct PidHashMap PidHashMap;
//
//PidHashMap* PidMap_initialize(void);
//void PidMap_free(PidHashMap* map);
//NTSTATUS PidMap_insert(PidHashMap* map, ULONG pid, ULONG pfid);
//NTSTATUS PidMap_remove(PidHashMap* map, ULONG pid);
//ULONG PidMap_search(PidHashMap* map, ULONG pid);
//
//
//#endif // HASHMAP_H
//
