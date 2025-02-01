#include "../base.h"
#include "test.h"
#include "../memory.h"
#include "../virtual_memory.h"
#include "../arena.h"

static inline
void arena_buf_test(){
    TEST_BEGIN("Arena (Buffer)");
    static U8 memory[256];
    Arena arena = {0};
    arena_init_buffer(&arena, memory, sizeof(memory));

    U32* n = arena_alloc(&arena, sizeof(U32), alignof(U32));
    Test(arena.offset == 4);
    Test(n != NULL);

    while(arena_alloc(&arena, 1, 1)){}
    Test(arena.offset == arena.data.reserved);
    Test(!arena_alloc(&arena, 1, 1));

    arena_free_all(&arena);
    n = arena_alloc(&arena, 20, 4);
    Test(n != NULL);
    n = arena_resize(&arena, n, 64);
    Test(n != NULL);
    arena_alloc(&arena, 1, 1);
    n = arena_resize(&arena, n, 64);
    Test(n == NULL);
    TEST_END;
}

static inline
void arena_virt_test(){
    TEST_BEGIN("Arena (Virtual)");

	Arena arena = {0};
	Test(arena_init_virtual(&arena, 1 * GiB));

	static const int count = 2049;
	U32* n = arena_alloc(&arena, sizeof(U32) * count, alignof(U32));
	Test(n != NULL);
	printf("Reserve: %ld Commit: %ld\n", arena.data.reserved, arena.data.commited);
	virtual_block_pop(&arena.data, 20);
	printf("Reserve: %ld Commit: %ld\n", arena.data.reserved, arena.data.commited);
	for(int i = 0; i < count; i ++){
		n[i] = i;
	}

    TEST_END;
}

#include <stdlib.h>
int main(){
	virtual_init();
    arena_buf_test();
    arena_virt_test();
}
