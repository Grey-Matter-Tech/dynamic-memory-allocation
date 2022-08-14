/*
 * dma.h
 *
 *  Created on: **-***-20**
 *      Author: HarshDave
 */

#ifndef DYNAMIC_MEMORY_ALLOCATION_DMA_H_
#define DYNAMIC_MEMORY_ALLOCATION_DMA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//TODO: dummy commits

/**
 * hheap memory configuration.
 * alignment	: rounds off memory chunk.
 * header_size	: holds meta information about memory.
 *                stores the size and status(free or occupied) of memory.
 * heap_size	: total size of hheap memory
 * debug		: WIP(later pal)
 */
#define ALIGNMENT 4U
#define HEADER_SIZE 4U
#define ALIGN(size)	size + (ALIGNMENT - 1) & ~(ALIGNMENT - 1)
#define HEAP_SIZE 1024*4*1024
#define APP_DEBUG 1
#define HEAP_DEBUG_ALL 2
#define HEAP_ADDRESS_DEBUG 3
#define HEAP_MEM_SIZE_DEBUG 4
#define HEAP_3_4_COMBINE 5
#define DEBUG HEAP_DEBUG_ALL

/**
 * hheap APIs return status
 */
#define OK 0U
#define FAIL 1U

/**
 * hheap utilities.
 * high_end marks end of hheap memory
 * low_end marks beginning of hheap memory
 */
#define HEAP_HIGH_END \
	({\
		hheap->heap + HEAP_SIZE; \
	})

#define HEAP_LOW_END \
	({\
		hheap->heap;\
	})

#define UPDATE_REM_MEM(x) \
	({\
		hheap->rem_mem -= x;\
	})

#define VALIDATE_ADDRESS(x) \
	({\
		unsigned char ret = FAIL;\
		if(x >= hheap->heap && x < HEAP_HIGH_END) \
		{\
			ret = OK;\
		}\
		ret;\
	})


#define DMA_SIZE ALIGN(HEAP_SIZE)

struct heap_memory{
	unsigned int total_mem;
	unsigned int rem_mem;
	unsigned int heap[];
};

/**
 * WIP
 */
struct hheap_driver{
	struct heap_memory **heap; /*WIP-#1*/
	unsigned char (*init_heap)(void);
	void * (*heap_alloc)(unsigned int size);
	unsigned char (*heap_free)(void *addr);
	void (*heap_maintenance)(void * free_ptr);
	void (*heap_statistics)(void);
};

struct hheap_driver driver_beta;

#endif /* DYNAMIC_MEMORY_ALLOCATION_DMA_H_ */
