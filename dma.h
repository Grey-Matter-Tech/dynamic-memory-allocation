/*
 * Copyright (c) 2022, Harsh Dave.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 */

/**
 * \file
 *         declarations of driver structure and prototypes of dynamic memory allocation APIs
 * \author
 *         Harsh Dave <HarshDave-Sithlord>
 */

#ifndef DYNAMIC_MEMORY_ALLOCATION_DMA_H_
#define DYNAMIC_MEMORY_ALLOCATION_DMA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef HHEAP_TYPEDEF
#include "typedef.h"
#endif

/**
 * hheap memory configuration.
 * alignment	: rounds off memory chunk.
 * header_size	: holds meta information about memory.
 *                stores the size and status(free or occupied) of memory.
 * heap_size	: total size of hheap memory
 * debug		: WIP(later pal)
 */
/**
 * Memory alignment macros.
 * Currently we align the memory to be the multiple of word(4 - bytes)
 * Header holds the size of allocated buffer and status of buffer(avail or occupied)
 */
#define ALIGNMENT 4U
#define HEADER_SIZE 4U
#define ALIGN(size)	(( (size) + (ALIGNMENT - 1) ) & ~(ALIGNMENT - 1))

/**
 * Total heap size.
 */
#define HEAP_SIZE ALIGN(1024*4*1024)

/**
 * Debug configuration macros.
 * WIP
 */
#define APP_DEBUG 1
#define HEAP_DEBUG_ALL 2
#define HEAP_ADDRESS_DEBUG 3
#define HEAP_MEM_SIZE_DEBUG 4
#define HEAP_3_4_COMBINE 5
#define DEBUG HEAP_DEBUG_ALL

/**
 * Heap memory policy decides in what manner the memory to be found
 * for a given size.
 * User can opt for first fit, next fit and best fit policy.
 * WIP
 */
#define FIRST_FIT 0U
#define NEXT_FIT 1U
#define BEST_FIT 2U

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
		hheap->heap + HEAP_SIZE/sizeof(unsigned int); \
	})

#define HEAP_LOW_END \
	({\
		hheap->heap;\
	})

/**
 * Once memory chunk is allocated, meta-data of heap needs to be
 * updated since it keeps current status information about heap.
 */
#define UPDATE_REM_MEM(x) \
	({\
		hheap->rem_mem -= x;\
		if(hheap->rem_mem <= 0)\
		{\
			printf("Critical Error\n");\
		}\
	})

/**
 * Utility routine to check whether the address falls within
 * valid range of heap memory or not.
 */
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
typedef enum{
	heap_first_fit = 0,
	heap_next_fit,
	heap_best_fit,
}heap_policy;

/**
 * WIP
 */
struct hheap_driver{
	struct heap_memory **heap; /*WIP-#1*/
	unsigned char (*init_heap)(void);
	void * (*heap_alloc)(unsigned int size);
	bool_t (*heap_realloc)(void **addr, unsigned int size);
	unsigned char (*heap_free)(void *addr);
	void (*heap_flush)(void);
	void (*heap_maintenance)(void * free_ptr);
	void (*heap_statistics)(void);
	void (*set_heap_policy)(heap_policy policy);
	heap_policy (*get_heap_policy)(void);
};

typedef void *(*find_mem_block)(uint32_t size);

struct hheap_driver driver_beta;

#define HEAP driver_beta

#endif /* DYNAMIC_MEMORY_ALLOCATION_DMA_H_ */
