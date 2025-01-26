#include "prog2.h"

#include <stdio.h>
#include <stdbool.h>

#define uintptr uintptr_t
#define isize   ptrdiff_t
#define byte    uint8_t
#define cstring char const *

static inline
void ensure(bool predicate, cstring msg){
	if(!predicate){
		fprintf(stderr, "Assertion fail: %s\n", msg);
		__builtin_trap();
	}
}

static inline
bool valid_alignment(uintptr align){
	return ((align & (align - 1)) == 0) && (align != 0);
}

static inline
uintptr align_forward_ptr(uintptr p, uintptr a){
	ensure(valid_alignment(a), "Invalid memory alignment");
	uintptr mod = p & (a - 1); // Fast modulo for powers of 2
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

static inline
uintptr arena_required_mem(uintptr cur, isize nbytes, isize align){
	ensure(valid_alignment(align), "Alignment must be a power of 2");
	uintptr aligned  = align_forward_ptr(cur, align);
	uintptr padding  = (uintptr)(aligned - cur);
	uintptr required = padding + nbytes;
	return required;
}

void* arena_alloc(Arena* ar, isize size, isize align){
	uintptr base = (uintptr)ar->data;
	uintptr current = base + (uintptr)ar->offset;

	uintptr available = (uintptr)ar->capacity - (current - base);
	uintptr required = arena_required_mem(current, size, align);

	// TODO: chain
	if(required > available){
		return NULL;
	}

	ar->offset += required;
	void* allocation = &ar->data[ar->offset - size];
	ar->last_allocation = (uintptr)allocation;
	return allocation;
}

void arena_free_all(Arena* ar){
	ar->offset = 0;
	// TODO: chain
}

void* arena_resize(Arena* ar, void* ptr, isize new_size){
	if((uintptr)ptr == ar->last_allocation){
		uintptr base = (uintptr)ar->data;
		uintptr current = base + (uintptr)ar->offset;
		uintptr limit = base + (uintptr)ar->capacity;
		isize last_allocation_size = current - ar->last_allocation;

		if((current - last_allocation_size + new_size) > limit){
			return NULL; /* No space left*/
		}

		ar->offset += new_size - last_allocation_size;
		return ptr;
	}

	return NULL;
}

