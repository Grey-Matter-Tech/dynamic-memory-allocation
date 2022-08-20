/*
 * sample_application.c
 *
 *  Created on: **-***-20**
 *      Author: HarshDave
 */

#include "dma.h"
#include "utils.h"

void main(void)
{
	if(driver_beta.init_heap() == OK)
	{
		int i = 0, size = 0;
		unsigned int *ptr = NULL, *sptr = NULL;
		unsigned char catch = rand() % 9 + 1;
		while(i < 100)
		{
			if( i % 4 == 0)
			{
				driver_beta.heap_free(sptr);
			}
			else
			{
				size = rand() % 569;
				ptr = (unsigned int *)driver_beta.heap_alloc(size * sizeof(int));
				if(ptr)
				{
					if(i % catch == 0)
					{
						sptr = ptr;
					}
#if (DEBUG==APP_DEBUG) || (DEBUG==HEAP_DEBUG_ALL)
					memset(ptr, ('A' + (rand() % 26)),size * sizeof(int));
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
		driver_beta.heap_statistics();
	}
}
