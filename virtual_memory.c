#ifndef _virtual_memory_c_include_
#define _virtual_memory_c_include_

#if defined(TARGET_OS_LINUX) || defined(TARGET_OS_WINDOWS)
#include "virtual_memory.h"

MemoryBlock virtual_block_create(Size reserve){
	reserve = align_forward_size(reserve, VIRTUAL_PAGE_SIZE);
	MemoryBlock blk = {
		.ptr = virtual_reserve(reserve), 
		.commited = 0,
	};

	blk.reserved = blk.ptr != NULL ? reserve : 0;
	return blk;
}

void virtual_block_destroy(MemoryBlock* block){
	virtual_free(block->ptr, block->reserved);
}

void* virtual_block_push(MemoryBlock* block, Size count){
	count = align_forward_size(count, VIRTUAL_PAGE_SIZE);
	U8* old_ptr = block->ptr + block->commited;
	void* new_ptr = virtual_commit(old_ptr, count);
	if(new_ptr == NULL){
		return NULL; /* Memory error */
	}
	block->commited += count;
	return old_ptr;
}

void virtual_block_pop(MemoryBlock* block, Size len){
	len = clamp(0, len, block->commited);

	Uintptr base = (Uintptr)block->ptr;
	// Free pages *after* this location
	Uintptr free_after = base + (block->commited - len);
	free_after = align_forward_ptr(free_after, VIRTUAL_PAGE_SIZE);

	Size amount_to_free = (base + block->commited) - free_after;
	virtual_decommit((void*)free_after, amount_to_free);
	block->commited -= amount_to_free;
}

#endif
#endif /* Include guard */
