#include "arena.h"
#include "memory.h"
#include "virtual_memory.h"

bool arena_init_buffer(Arena* a, U8* data, Size len){
	if(len <= 0){ return false; }
	mem_set(a, 0, sizeof(*a));
	a->data = (MemoryBlock){
		.commited = len,
		.reserved = len,
		.ptr = data,
	};
	a->kind = ArenaKind_Buffer;
	return true;
}

Uintptr arena_required_mem(Uintptr cur, Size count, Size align){
	ensure(mem_valid_alignment(align), "Alignment must be a power of 2");
	Uintptr aligned  = align_forward_ptr(cur, align);
	Uintptr padding  = (Uintptr)(aligned - cur);
	Uintptr required = padding + count;
	return required;
}

bool arena_init_virtual(Arena* a, Size reserve){
	if(reserve <= 0){ return false; }
	mem_set(a, 0, sizeof(*a));
	MemoryBlock data = virtual_block_create(reserve);
	if(data.ptr == NULL){ return false; }
	a->data = data;
	a->kind = ArenaKind_Virtual;
	return true;
}

void *arena_alloc(Arena* a, Size size, Size align){
	Uintptr base = (Uintptr)a->data.ptr;
	Uintptr current = (Uintptr)base + (Uintptr)a->offset;

	Size available = a->data.commited - (current - base);
	Size required  = arena_required_mem(current, size, align);

	/* Retry */ while(1){
		if(hint_unlikely(required > available)){
			Size in_reserve = a->data.reserved - a->data.commited;
			Size diff = required - available;
			if(diff > in_reserve){
				return NULL; /* Out of memory */
			}
			else if(a->kind == ArenaKind_Virtual){
				if(virtual_block_push(&a->data, required) == NULL){
					return NULL; /* Memory Error */
				}
			}
		}

		break;
	}

	a->offset += required;
	void* allocation = a->data.ptr + (a->offset - size);
	a->last_allocation = (Uintptr)allocation;
	return allocation;
}

void arena_free_all(Arena* a){
	a->offset = 0;
}

void* arena_resize(Arena* a, void* ptr, Size new_size){
	retry:
	if((Uintptr)ptr == a->last_allocation){
		Uintptr base = (Uintptr)a->data.ptr;
		Uintptr current = base + (Uintptr)a->offset;
		Uintptr limit = base + (Uintptr)a->data.commited;
		Size last_allocation_size = current - a->last_allocation;

		if((current - last_allocation_size + new_size) > limit){
			if(a->kind == ArenaKind_Virtual){
				Size to_commit = (current - last_allocation_size + new_size) - limit;
				if(virtual_block_push(&a->data, to_commit) != NULL){
					goto retry;
				}
			}

			return NULL; /* No space left*/
		}

		a->offset += new_size - last_allocation_size;
		return ptr;
	}

	return NULL;
}

void* arena_realloc(Arena* a, void* ptr, Size old_size, Size new_size, Size align){
	if(ptr == NULL){
		return arena_alloc(a, new_size, align);
	}
	void* new_ptr = arena_resize(a, ptr, new_size);
	if(new_ptr == NULL){
		new_ptr = arena_alloc(a, new_size, align);
		if(new_ptr != NULL){
			mem_copy_no_overlap(new_ptr, ptr, min(old_size, new_size));
		}
	}
	return new_ptr;
}

void arena_destroy(Arena* a){
	arena_free_all(a);
	if(a->kind == ArenaKind_Virtual){
		virtual_block_destroy(&a->data);
	}
}
