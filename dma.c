/*
 * dma.c
 *
<<<<<<< HEAD
 *  Created on: **-***-20**
 *      Author: HarshDave
 */
#include "dma.h"
#include "utils.h"

//Heap memory HEAD. chop it off.
struct heap_memory *hheap = NULL;

unsigned char hheap_init(void)
{
	unsigned char ret = FAIL;
	hheap = (struct heap_memory *)malloc(sizeof(struct heap_memory) + HEAP_SIZE);
	if(hheap)
	{
		memset(hheap->heap, 0, HEAP_SIZE);
		hheap->rem_mem = HEAP_SIZE;
		hheap->total_mem = HEAP_SIZE;
#if DEBUG == HEAP_DEBUG_ALL
		printf("hheap memory is initialized\n");
#endif
		ret = OK;
	}
	return ret;
}

void hheap_stats(void)
{
	unsigned int *ptr = (unsigned int *)HEAP_LOW_END;
	while(ptr < (unsigned int *)HEAP_HIGH_END)
	{
		printf("%p and %d\n", ptr, (*(unsigned int *)ptr & ~(1)));
		if ((*ptr & 1))
		{
			printf("Memory chunk at :: %p [%d]\n", ptr, (*(unsigned int *)ptr & ~(1)));
			ptr += (*ptr & ~(1))/sizeof(unsigned int);
		}
		else
		{
			break;
		}
	}
	printf("Heap Total Size\t %d\nRemaining Memory\t %d\n",
			hheap->total_mem, hheap->rem_mem);
}

static void * find_fit(unsigned int size)
{
	unsigned int *start = (unsigned int *)HEAP_LOW_END;
	while(start < (unsigned int *)HEAP_HIGH_END)
	{
		if(( (!(*start & 1)) && (*start > size) ) || (!(*start & 1)))
		{
			return (void *)start;
		}
		//printf("######## FIND_FIT :: %d\n", *start, *start/4);
		start += ((*start) & ~(1))/sizeof(int);
	}
	return NULL;
}


void hheap_maintenance(void * free_ptr)
{
	unsigned int *ptr = (free_ptr + *(unsigned int *)(free_ptr));
	unsigned char *nptr = (unsigned char *)free_ptr + *(unsigned int *)(free_ptr);
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
		high_end = (unsigned long) (nptr + (*nptr & ~(1)));
		while((unsigned long)nptr < high_end)
		{
			*src++ = *nptr++;
		}
		count--;
	}
}

unsigned char hheap_free(void *addr)
{
	unsigned char ret = FAIL;
	if(addr)
	{
		unsigned int *header = (addr - HEADER_SIZE);

#if (DEBUG == HEAP_DEBUG_ALL) || (DEBUG == HEAP_ADDRESS_DEBUG)
		printf(KYEL"hfree :: Checking address %p and %p -- [%p][%p]\n"KNRM, header, addr, HEAP_LOW_END, HEAP_HIGH_END);
#endif
		if(VALIDATE_ADDRESS(header) == OK)
		{
#if (DEBUG == HEAP_DEBUG_ALL) || (DEBUG == HEAP_ADDRESS_DEBUG)
			printf(KYEL"hfree :: Address is valid :: %p[%d]\n"KNRM, header + HEADER_SIZE, *(unsigned int *)header);
			printf(KYEL"hfree :: Freeing the memory\n"KNRM);
#endif
			*header &= ~(1);
			UPDATE_REM_MEM(-*(header));
			ret = OK;
		}
		hheap_maintenance(header);
	}
	else
	{
		printf("Address is NULL\n");
	}
	return ret;
}


void *hheap_alloc(unsigned int size)
{
	void *header = NULL;
	unsigned int total_size = ALIGN(size + HEADER_SIZE);
#if (DEBUG == HEAP_MEM_SIZE_DEBUG) || (DEBUG == HEAP_DEBUG_ALL)
	printf(KGRN"hmalloc :: total size needed is :: %d\n"KNRM, total_size);
#endif
	header = find_fit(total_size);
	if(header)
	{
		*(unsigned int *)(header) = (unsigned int)(total_size | 1);
		*(unsigned int *)(header + total_size) = (HEAP_SIZE - total_size);
		/**
		 * update heap stats. Since a chunk is allocated
		 * we need the book keeping for available memory
		 */
		UPDATE_REM_MEM(total_size);
#if (DEBUG == HEAP_ADDRESS_DEBUG) || (DEBUG == HEAP_DEBUG_ALL)
		printf(KGRN"hmalloc :: Setting header %p[%d]\n"KNRM, header, *(unsigned int*)header);
#endif
	}
	else
	{
		printf(KRED"hmalloc :: Error finding chunk\n"KNRM);
	}
	return (void *)(header + HEADER_SIZE);
}

struct hheap_driver driver_beta = {
	.heap = &hheap,
	.init_heap = hheap_init,
	.heap_alloc = hheap_alloc,
	.heap_free = hheap_free,
	.heap_maintenance = hheap_maintenance,
	.heap_statistics = hheap_stats,
};
