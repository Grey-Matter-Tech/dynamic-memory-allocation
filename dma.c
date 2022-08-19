/*
 * dma.c
 *
 *  Created on: **-***-20**
 *      Author: HarshDave
 */
#include "dma.h"
#include "utils.h"

//Heap memory HEAD. chop it off.
struct heap_memory *hheap = NULL;

void hheap_show(void)
{
	unsigned int *header = HEAP_LOW_END;
	unsigned char *buffer = (unsigned char *)(header + 1);
	unsigned int upto = 0;
	while(header < HEAP_HIGH_END && ((*header & 1) == 1))
	{
		{
			printf("HEADER :: [%p][%d]\n", header, *header);
			printf("Buffer :: \n");
			upto = (((*header) & ~(1)) - HEADER_SIZE);
			buffer = (unsigned char *) (header + 1);
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
		buffer = (unsigned char *)header;
		buffer +=  (*header & ~(1));
		header = (unsigned int *)buffer;
	}
	printf("\n");
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
unsigned char hheap_init(void)
{
	unsigned char ret = FAIL;
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

/**
 * hheap_stats
 * ARGS:none
 * Return value: none
 * Description: Dumps hheap memory's active blocks
 * and its corresponding size.
 */
void hheap_stats(void)
{
	unsigned int *ptr = (unsigned int *)HEAP_LOW_END;
	while(ptr < (unsigned int *)HEAP_HIGH_END)
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
			ptr += (*ptr & ~(1))/sizeof(unsigned int);
		}
		else
		{
			break;
		}
	}
}

/**
 * find_fit
 * ARGS:size(size of memory chunk to be allocated)
 * Return value: void *(returns starting address of memory chunk available)
 * Description: Traverse through hheap memory, looks for memory chunk which
 * is large enough to hold data of given size and finally returns its address.
 */
static void * find_fit(unsigned int size)
{
	unsigned char *start = (unsigned char *)HEAP_LOW_END;
	while(start < (unsigned char *)HEAP_HIGH_END)
	{
		if(( (!(*(unsigned int *)start & 1)) && (*(unsigned int *)start > size) ) /*|| (!(*start & 1))*/)
		{
			return (void *)start;
		}
		start += ((*(unsigned int *)start) & ~(1));
	}
	return NULL;
}


void hheap_maintenance(void * free_ptr)
{
	hheap_show();
	unsigned int *ptr = (free_ptr + *(unsigned int *)(free_ptr));
	unsigned char *nptr = (unsigned char *)free_ptr;
	nptr += *((unsigned int *)free_ptr);
	unsigned char *src = free_ptr;
	unsigned long high_end = 0;
	unsigned int count = 0;
	while(ptr < HEAP_HIGH_END)
	{
		if ((*ptr & 1))
		{
			count++;
			ptr += (*ptr & ~(1))/sizeof(unsigned int);
		}
		else
		{
			break;
		}
	}

	while(count > 0)
	{
		high_end = (unsigned long) ((nptr + (*(unsigned int *)nptr & ~(1))) + HEADER_SIZE);
		while((unsigned long)nptr < high_end)
		{
			*src++ = *nptr;
			*nptr ^= *nptr;
			nptr++;
		}
		count--;
	}

	hheap_show();
}

unsigned char hheap_free(void *addr)
{
	unsigned char ret = FAIL;
	if(addr)
	{
		unsigned int *header = (addr - HEADER_SIZE);

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


void *hheap_alloc(unsigned int size)
{
	void *header = NULL;
	unsigned int total_size = ALIGN(size + HEADER_SIZE);

#if (DEBUG == HEAP_MEM_SIZE_DEBUG) || (DEBUG == HEAP_DEBUG_ALL)
	printf("hmalloc :: total size needed is :: %d\n", total_size);
#endif
	header = find_fit(total_size);
	if(header)
	{
		*(unsigned int *)(header) = (unsigned int)( total_size | 1);
		/**
		 * update heap stats. Since a chunk is allocated
		 * we need the book keeping for available memory
		 */
		UPDATE_REM_MEM(total_size);
		*(unsigned int *)(header + total_size ) = (hheap->rem_mem);

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

struct hheap_driver driver_beta = {
	.heap = &hheap,
	.init_heap = hheap_init,
	.heap_alloc = hheap_alloc,
	.heap_free = hheap_free,
	.heap_maintenance = hheap_maintenance,
	.heap_statistics = hheap_stats,
};
