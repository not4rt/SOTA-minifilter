#include "DriverHandler.h"

//SOTA_DRIVER_DATA initialize_driverobj(PDRIVER_OBJECT DriverObject) {
//    SOTA_DRIVER_DATA map = (HashMap*)ExAllocatePool2(NonPagedPool, sizeof(HashMap), 'map1');
//    if (!map) return NULL;
//
//    map->size = INITIAL_SIZE;
//    map->count = 0;
//    map->buckets = (Entry**)ExAllocatePool2(NonPagedPool, sizeof(Entry*) * map->size, 'bckt');
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
//}