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
static find_mem_block find_fit;
static heap_policy current_policy = heap_next_fit;
static uint8_t *mem_tracker = NULL;

/**
 * find_fit
 * ARGS:size(size of memory chunk to be allocated)
 * Return value: void *(returns starting address of memory chunk available)
 * Description: Traverse through hheap memory, looks for memory chunk which
 * is large enough to hold data of given size and finally returns its address.
 */
static void * first_fit(uint32_t size)
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
 * next_fit
 * ARGS:size(size of memory chunk to be allocated)
 * Return value: void *(returns starting address of memory chunk available)
 * Description: Traverse through hheap memory(starting from address pointed by
 * mem_tracker), looks for memory chunk which is large enough to hold data of
 * given size and finally returns its address.
 */
static void * next_fit(uint32_t size)
{
	uint8_t *start = mem_tracker;
	bool_t iterated_flag = 0U;
	while(!iterated_flag)
	{
		if( ( (!(*(uint32_t *)start & 1)) && (*(uint32_t *)start > size) ) )
		{
			mem_tracker = start;
			return start;
		}

		start += ((*(uint32_t *)start) & ~(1));

		if(start == (uint8_t *)HEAP_HIGH_END)
		{
			start = (uint8_t *)HEAP_LOW_END;
		}

		if(start == mem_tracker)
		{
			iterated_flag = 1U;
		}
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
		switch(current_policy)
		{
			case heap_first_fit:
			{
				find_fit = first_fit;
				break;
			}
			case heap_next_fit:
			{
				find_fit = next_fit;
				mem_tracker = (uint8_t *)HEAP_LOW_END;
				break;
			}
			default:
			{
				printf("Oops!\n");
				//find_fit = err_handler;
				break;
			}
		}

#if DEBUG == HEAP_DEBUG_ALL
		printf("hheap memory is initialized[%d]\n", HEAP_SIZE);
#endif
		ret = OK;
	}
	return ret;
}

/**
 * hheap_alloc
 * ARGS:size
 * Return value: address at which allocated buffer starts
 * Description: allocates the memory buffer of requested
 * size from heap memory. Rounds up size + HEADER_SIZE to append
 * a header to hold metaa information regarding allocated buffer,
 * such as size of buffer and status of it(avail or occupied).
 */
void *hheap_alloc(uint32_t size)
{
	void *header = NULL;
	uint32_t total_size = ALIGN(size + HEADER_SIZE);

#if (DEBUG == HEAP_MEM_SIZE_DEBUG) || (DEBUG == HEAP_DEBUG_ALL)
	printf("hmalloc :: total size needed is :: %d\n", total_size);
#endif
	/**
	 * find_fit is a function pointer which calls respective
	 * function based on current heap policy.
	 * As of now it is supporting first_fit and next_fit only.
	 */
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

/**
 * hheap_free
 * ARGS:address of buffer to be freed.
 * Return value: ret (OK, FAIL)
 * Description: checks the given address is valid or not(whether it
 * is within the range of heap memory). If address is valid, it will mark
 * its status as available memory block in its header.
 * Finally it calls hheap_maintenance to align all the occupied buffers at side
 * of heap and all the available buffers at the other side of heap.
 */
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
			printf("hfree :: Address is valid :: %p[%d]\n", header, *(unsigned int *)header);
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

bool_t hheap_realloc(void **addr, uint32_t size)
{
	bool_t ret = FAIL;

	if(*addr)
	{
		void *current_header = *addr - HEADER_SIZE;
		void *new_header = NULL;
		uint32_t total_size = ALIGN(size + HEADER_SIZE);
		int32_t diff = 0, current_size = (*(uint32_t *)current_header & ~(1));
		void *next_header = (*addr + current_size) - HEADER_SIZE;

		if((*(uint32_t *)next_header == hheap->rem_mem) && (hheap->rem_mem > size))
		{
			diff = size - current_size;
			current_size += diff;
			*(uint32_t *)current_header = current_size;
			ret = OK;
		}
		else
		{
			new_header = hheap_alloc(size);
			memset(new_header, '@', size);
			if(new_header)
			{
				for(uint32_t i = 0; i < current_size; i++)
				{
					*(uint8_t *)(new_header + i) = *(uint8_t *)(*addr + i);
				}
				hheap_free(*addr);
			}
			ret = OK;
		}
		*addr = new_header - current_size;
	}

	return ret;
}

/**
 * hheap_flush
 * ARGS:none
 * Return value: none
 * Description: Clears the content of entire heap memory except the only header.
 */
void hheap_flush(void)
{
	memset(hheap->heap, 0, HEAP_SIZE);
	hheap->heap[0] = HEAP_SIZE;
}

void hheap_maintenance(void * free_ptr)
{
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
		high_end = (uint64_t) ((nptr + (*(uint32_t *)nptr & ~(1))));
		if(count == 1)
			high_end += HEADER_SIZE;
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

void hheap_set_policy(heap_policy policy)
{
	current_policy = policy;
}

heap_policy hheap_get_policy(void)
{
	return current_policy;
}

struct hheap_driver driver_beta = {
	.heap = &hheap,
	.init_heap = hheap_init,
	.heap_alloc = hheap_alloc,
	.heap_realloc = hheap_realloc,
	.heap_free = hheap_free,
	.heap_flush = hheap_flush,
	.heap_maintenance = hheap_maintenance,
	.heap_statistics = hheap_stats,
	.set_heap_policy = hheap_set_policy,
	.get_heap_policy = hheap_get_policy,
};
