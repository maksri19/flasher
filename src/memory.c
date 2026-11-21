#include "common.h"

static uint8_t pool[MEMORY_POOL_CAPACITY];
static size_t offset = 0;


void* memory_alloc(size_t space) {
	// check if there available enough memory
	if (offset + space > MEMORY_POOL_CAPACITY) {
		log_exit("Offset exceed pool capacity: %zu > %zu\n", offset + space, MEMORY_POOL_CAPACITY);
	}

	//  calculate and return pointer
	void* ptr = (void*)(pool + offset);
	offset += space;

	return ptr;
}


size_t memory_getOffset() {
	return offset;
}
