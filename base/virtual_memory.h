#ifndef _virtual_memory_h_include_
#define _virtual_memory_h_include_

#include "base.h"
#include "memory.h"

typedef struct MemoryBlock MemoryBlock;
enum MemoryProtection {
    MemoryProtection_Read  = (1 << 0),
    MemoryProtection_Write = (1 << 1),
    MemoryProtection_Exec  = (1 << 2),
};

// Region of virtual memory, can also be used as a generic buffer if virtual memory is not supported
struct MemoryBlock {
    void* ptr;
    Size commited;
    Size reserved;
};

#if defined(TARGET_OS_LINUX) || defined(TARGET_OS_WINDOWS)

#define VIRTUAL_PAGE_SIZE (4 * KiB)

// Should be called before any of the other virtual_* functions.
// This will verify very crucial assumptions about the environment.
// Returns success status
void virtual_init();

MemoryBlock virtual_block_create(Size reserve);

void virtual_block_destroy(MemoryBlock* block);

void* virtual_block_push(MemoryBlock* block, Size len);

void virtual_block_pop(MemoryBlock* block, Size len);

bool virtual_protect(void* ptr, Size len, U8 prot);

void* virtual_reserve(Size len);

void virtual_free(void* ptr, Size len);

void virtual_decommit(void* ptr, Size len);

void* virtual_commit(void* ptr, Size len);


static_assert(((VIRTUAL_PAGE_SIZE & (VIRTUAL_PAGE_SIZE - 1)) == 0) && (VIRTUAL_PAGE_SIZE > 0), "Page size must be a power of 2 greater than 0");
#endif


#endif /* Include guard */
