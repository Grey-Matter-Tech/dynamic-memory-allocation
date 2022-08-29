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
 *         A very simple application to validate basic functionality of hheap memory.
 * \author
 *         Harsh Dave <HarshDave-Sithlord>
 */

#include "dma.h"
#include "utils.h"

void main(void)
{
	if(HEAP.init_heap() == OK)
	{
		int i = 0, size = 0;
		unsigned int *ptr = NULL, *sptr = NULL;
		unsigned char catch = 3;
		while(i < 100)
		{
			/*if( i % 3 == 0 && i > 0)
			{
				//HEAP.heap_free(sptr);
				//HEAP.heap_flush();
			}
			else */if(i % 4 == 0 && i > 0)
			{
				printf("About to go and realloc memory\n");
				HEAP.heap_realloc((void *)&sptr, 60);
				memset(sptr, '$', 60);
			}
			else
			{
				size = rand() % 15;
				ptr = (unsigned int *)HEAP.heap_alloc(size * sizeof(int));
				if(ptr)
				{
					if(i % 2 == 0)
					{
						sptr = ptr;
					}
#if (DEBUG==APP_DEBUG) || (DEBUG==HEAP_DEBUG_ALL)
					char data = ('A' + (rand() % 26));
					printf("Writing data %c for %ld times\n\n\n", data, size * sizeof(int));
					memset(ptr, data, size * sizeof(int));
					printf("Allocated buffer at address :: %p[%d]\n", ptr, size);
#endif
				}
				else
				{
#if (DEBUG==APP_DEBUG) || (DEBUG==HEAP_DEBUG_ALL)
					printf("Failed!!\n");
#endif
				}
			}
			i++;
		}
	}
}
