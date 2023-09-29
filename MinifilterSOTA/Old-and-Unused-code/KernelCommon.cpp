//sn99

#include "KernelCommon.h"

#define POOL_FLAG_NON_PAGED 0x0000000000000040UI64 // Non paged pool NX

void* __cdecl operator new(size_t size) {
    return ExAllocatePool2(POOL_FLAG_NON_PAGED, size, 'RW');
}

void __cdecl operator delete(void* data, size_t size) {
    UNREFERENCED_PARAMETER(size);
    if (data != NULL)
        ExFreePoolWithTag(data, 'RW');
}

void __cdecl operator delete(void* data) {
    if (data != NULL)
        ExFreePoolWithTag(data, 'RW');
}
