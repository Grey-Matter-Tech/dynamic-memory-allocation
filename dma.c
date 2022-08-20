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
 *         A very simple implementation of dynamic memory allocation APIs
 * \author
 *         Harsh Dave <HarshDave-Sithlord>
 */
#include "dma.h"
#include "utils.h"

void hheap_show(void);
void hheap_maintenance(void * free_ptr);
struct heap_memory *hheap = NULL;

/**
 * find_fit
 * ARGS:size(size of memory chunk to be allocated)
 * Return value: void *(returns starting address of memory chunk available)
 * Description: Traverse through hheap memory, looks for memory chunk which
 * is large enough to hold data of given size and finally returns its address.
 */
static void * find_fit(uint32_t size)
{
	uint8_t *start = (uint8_t *)HEAP_LOW_END;
	while(start < (uint8_t *)HEAP_HIGH_END)
	{
		if( ( (!(*(uint32_t *)start & 1)) && (*(uint32_t *)start > size) ) )
		{
			return (void *)start;
		}
		start += ((*(uint32_t *)start) & ~(1));
	}
	return NULL;
}

/**
 * hheap_init
 * ARGS:none
 * Return value: ret(OK,FAIL)
 * Description: Based on user configurable macro HEAP_SIZE,
 * allocates the memory chunk for our heap memory.
 * Sets up meta information regarding heap such as remaining memory,
 * total memory and the very first header in heap memory.
 */
bool_t hheap_init(void)
{
	bool_t ret = FAIL;
	/**
	 * Allocates the memory of size specified by macro HEAP_SIZE + heap meta data.
	 */
	hheap = (struct heap_memory *)malloc(sizeof(struct heap_memory) + HEAP_SIZE);
	if(hheap)
	{
		/**
		 * Initialize the data buffer with 0.
		 * Initialize remaining memory and total memory with HEAP_SIZE.
		 * Set the first heap header with value HEAP_SIZE.
		 */
		memset(hheap->heap, 0, HEAP_SIZE);
		hheap->rem_mem = HEAP_SIZE;
		hheap->total_mem = HEAP_SIZE;
		hheap->heap[0] = HEAP_SIZE;
#if DEBUG == HEAP_DEBUG_ALL
		printf("hheap memory is initialized[%d]\n", HEAP_SIZE);
#endif
		ret = OK;
	}
	return ret;
}

void *hheap_alloc(uint32_t size)
{
	void *header = NULL;
	uint32_t total_size = ALIGN(size + HEADER_SIZE);

#if (DEBUG == HEAP_MEM_SIZE_DEBUG) || (DEBUG == HEAP_DEBUG_ALL)
	printf("hmalloc :: total size needed is :: %d\n", total_size);
#endif
	header = find_fit(total_size);
	if(header)
	{
		*(uint32_t *)(header) = (uint32_t)( total_size | 1);
		/**
		 * update heap stats. Since a chunk is allocated
		 * we need the book keeping for available memory
		 */
		UPDATE_REM_MEM(total_size);
		*(uint32_t *)(header + total_size ) = (hheap->rem_mem);

#if (DEBUG == HEAP_ADDRESS_DEBUG) || (DEBUG == HEAP_DEBUG_ALL)
		printf("hmalloc :: Setting header %p[%d]\n", header, *(unsigned int*)header);
#endif
		header += HEADER_SIZE;
	}
	else
	{
		printf("hmalloc :: Error finding chunk\n");
	}

	return (void *)(header);
}

bool_t hheap_free(void *addr)
{
	bool_t ret = FAIL;
	if(addr)
	{
		uint32_t *header = (addr - HEADER_SIZE);

#if (DEBUG == HEAP_DEBUG_ALL) || (DEBUG == HEAP_ADDRESS_DEBUG)
		printf("hfree :: Checking address %p and %p -- [%p][%p]\n", header, addr, HEAP_LOW_END, HEAP_HIGH_END);
#endif
		if(VALIDATE_ADDRESS(header) == OK)
		{
#if (DEBUG == HEAP_DEBUG_ALL) || (DEBUG == HEAP_ADDRESS_DEBUG)
			printf("hfree :: Address is valid :: %p[%d]\n", header + HEADER_SIZE, *(unsigned int *)header);
			printf("hfree :: Freeing the memory\n");
#endif
			*header &= ~(1);
			UPDATE_REM_MEM(-*(header));
			ret = OK;
		}

		hheap_maintenance(header);
	}
	else
	{
		printf("NULL Address\n");
	}

	return ret;
}

void hheap_flush(void)
{
	memset(hheap->heap, 0, HEAP_SIZE);
	hheap->heap[0] = HEAP_SIZE;
}

void hheap_maintenance(void * free_ptr)
{
	hheap_show();
	uint32_t *ptr = (free_ptr + *(uint32_t *)(free_ptr));
	uint8_t *nptr = (uint8_t *)free_ptr;
	nptr += *((uint32_t *)free_ptr);
	uint8_t *src = free_ptr;
	uint64_t high_end = 0;
	uint32_t count = 0;
	while(ptr < HEAP_HIGH_END)
	{
		if ((*ptr & 1))
		{
			count++;
			ptr += (*ptr & ~(1))/sizeof(uint32_t);
		}
		else
		{
			break;
		}
	}

	while(count > 0)
	{
		high_end = (uint64_t) ((nptr + (*(uint32_t *)nptr & ~(1))) + HEADER_SIZE);
		while((uint64_t)nptr < high_end)
		{
			*src++ = *nptr;
			*nptr ^= *nptr;
			nptr++;
		}
		count--;
	}

	hheap_show();
}

/**
 * hheap_stats
 * ARGS:none
 * Return value: none
 * Description: Dumps hheap memory's active blocks
 * and its corresponding size.
 */
void hheap_stats(void)
{
	uint32_t *ptr = (uint32_t *)HEAP_LOW_END;
	while(ptr < (uint32_t *)HEAP_HIGH_END)
	{
		if ((*ptr & 1))
		{
			/**
			 * NOTE: Since the ptr is unsigned int type of
			 * pointer, it does memory jummp by multiple of
			 * sizeof(unsigned int). So to correct the offset
			 * it needs to be divided back by sizeof(unsigned int).
			 * If ptr were to be unsigned char or void pointer, no such
			 * division would have been necessary.
			 */
			ptr += (*ptr & ~(1))/sizeof(uint32_t);
		}
		else
		{
			break;
		}
	}
}

void hheap_show(void)
{
	uint32_t *header = HEAP_LOW_END;
	uint8_t *buffer = (uint8_t *)(header + 1);
	uint32_t upto = 0;
	while(header < HEAP_HIGH_END && ((*header & 1) == 1))
	{
		{
			printf("HEADER :: [%p][%d]\n", header, *header);
			printf("Buffer :: \n");
			upto = (((*header) & ~(1)) - HEADER_SIZE);
			buffer = (uint8_t *) (header + 1);
			if(upto < 0)
			{
				return;
			}

			for(int i =0; i< upto; i++)
			{
				if(i%100==0)
					printf("\n");
				printf("%c ", buffer[i]);
			}
			printf("\n");
		}
		buffer = (uint8_t *)header;
		buffer +=  (*header & ~(1));
		header = (uint32_t *)buffer;
	}
	printf("\n");
}


struct hheap_driver driver_beta = {
	.heap = &hheap,
	.init_heap = hheap_init,
	.heap_alloc = hheap_alloc,
	.heap_free = hheap_free,
	.heap_flush = hheap_flush,
	.heap_maintenance = hheap_maintenance,
	.heap_statistics = hheap_stats,
};
