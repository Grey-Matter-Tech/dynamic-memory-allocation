/*
 * dma.c
 *
 *  Created on: 27-Jul-2022
 *      Author: DaveH
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * For some fancy color prints.
 * ref taken from https://stackoverflow.com/questions/3585846/color-text-in-terminal-applications-in-unix
 */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define ALIGNMENT 4U
#define HEADER_SIZE 4U
#define ALIGN(size)	size + (ALIGNMENT - 1) & ~(ALIGNMENT - 1)
#define HEAP_SIZE 64*4*64
#define APP_DEBUG 1
#define HEAP_DEBUG_ALL 2
#define HEAP_ADDRESS_DEBUG 3
#define HEAP_MEM_SIZE_DEBUG 4
#define HEAP_3_4_COMBINE 5

#define DEBUG HEAP_DEBUG_ALL


#define OK 0U
#define FAIL 1U

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

/*do not why the FAIL someone would need to align chunk*/
#define DMA_SIZE ALIGN(HEAP_SIZE)

struct heap_memory{
	unsigned int total_mem;
	unsigned int rem_mem;
	unsigned int heap[];
};

//Heap memory HEAD. chop it off.
struct heap_memory *hheap = NULL;

unsigned char init_hheap(void)
{
	unsigned char ret = FAIL;
	hheap = (struct heap_memory *)malloc(sizeof(struct heap_memory) + (2*HEAP_SIZE));
	if(hheap)
	{
		memset(hheap->heap, 0, (2*HEAP_SIZE));
		hheap->rem_mem = HEAP_SIZE;
		hheap->total_mem = HEAP_SIZE;
#if DEBUG == HEAP_DEBUG_ALL
		printf("H-heap memory is initialized\n");
#endif
		ret = OK;
	}
	return ret;
}
#if 1
void heap_stats(void)
{
	unsigned int *ptr = hheap->heap;
	while(ptr < HEAP_HIGH_END)
	{
		printf("%p and %d\n", ptr, (*ptr & ~(1)));
		if ((*ptr & 1))
		{
			printf("Memory chunk at :: %p [%d]\n", ptr, (*ptr & ~(1)));
			ptr += (*ptr & ~(1));
		}
		else
		{
			break;
		}
	}
	printf("Heap Total Size\t %d\nRemaining Memory\t %d\n",
			hheap->total_mem, hheap->rem_mem);
}
#endif
void * find_fit(unsigned int size)
{
	unsigned int *start = HEAP_LOW_END;
	while(start < HEAP_HIGH_END)
	{
		if(( (!(*start & 1)) && (*start > size) ) || (!(*start & 1)))
		{
			return (void *)start;
		}
		start += ((*start) & ~(1));
	}
	return NULL;
}

void hheap_maintenance(void)
{
	unsigned int *heap_ptr = HEAP_LOW_END;
	unsigned long *mem_track = HEAP_HIGH_END;
	unsigned int index = 0;

	while(heap_ptr < HEAP_HIGH_END)
	{
		if ( (printf("hheap_maintenance :: %p %d %d\n", heap_ptr, *heap_ptr, *heap_ptr & 1)) && (*heap_ptr & 1)  )
		{
			mem_track[index++] = heap_ptr;
			heap_ptr += (*heap_ptr & ~(1));
		}
		else
		{
			break;
		}
	}
	//mem_track = HEAP_HIGH_END;
	for(unsigned int i = 0; i < index; i++)
	{
		printf("memory track[%d] :: %p\n", i, mem_track[i]);
	}
	printf("\n");
}

unsigned char hfree(void *addr)
{
	unsigned char ret = FAIL;
	void *header = (addr - HEADER_SIZE);
#if (DEBUG == HEAP_DEBUG_ALL) || (DEBUG == HEAP_ADDRESS_DEBUG)
	printf(KYEL"hfree :: Checking address %p and %p -- [%p][%p]\n"KNRM, header, addr, HEAP_LOW_END, HEAP_HIGH_END);

#endif
	if(VALIDATE_ADDRESS(header) == OK)
	{
#if (DEBUG == HEAP_DEBUG_ALL) || (DEBUG == HEAP_ADDRESS_DEBUG)
		printf(KYEL"hfree :: Address is valid :: %p[%d]\n"KNRM, header + HEADER_SIZE, *(unsigned int *)header);
		printf(KYEL"hfree :: Freeing the memory\n"KNRM);
#endif
		*(unsigned int *)header &= ~(1);
		UPDATE_REM_MEM(-*(unsigned int *)(header));
		ret = OK;
	}

	//hheap_maintenance();

	return ret;
}

void *hmalloc(unsigned int size)
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

void main(void)
{
	if(init_hheap() == OK)
	{
		/**
		 * Basic functionality test for our custom heap memory.
		 * As of 30-07-2022 13:54 supports hmalloc and hfree API.
		 */
		int i = 0, size = 0;
		unsigned int *ptr = NULL;
		while(i < 10)
		{
			if(i % 3 == 0 && i > 0)
			{
				hfree(ptr);
			}
			else
			{
				size = rand() % 69;
				ptr = (unsigned int *)hmalloc(size * sizeof(int));
				if(ptr)
				{
#if (DEBUG==APP_DEBUG) || (DEBUG==HEAP_DEBUG_ALL)
					printf("Allocated buffer at address :: %p[%d]\n", ptr, size);
#endif
				}
				else
				{
#if (DEBUG==APP_DEBUG) || (DEBUG==HEAP_DEBUG_ALL)
					printf("It is FAILed!!\n");
#endif
				}
			}
			i++;
		}
		printf("DUMPING STATS###########################\n");
		heap_stats();
		hheap_maintenance();
	}
}
