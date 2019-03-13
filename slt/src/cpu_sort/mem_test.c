#include <zlib.h>
#include "fault.h"

#define DEVICE_PATH	"/mnt/mmc/"
#define DEBUG	0

#if (DEBUG)
#define DBG_MSG(msg...)	{ printf("[mmc test]: " msg); }
#else
#define DBG_MSG(msg...)	do{} while(0);
#endif
#define CPU_TEST_SIZE 10 * KBYTE * KBYTE

pthread_t	mem_mthread[THREAD_NUM];
int		result[THREAD_NUM];
int		mem_exit_thread;

static void *mem_test_thread(int id)
{
	int ret = 0;
	ret = system("/usr/local/memtester 5M 1 > /dev/null\n");
	if(ret)
		result[id]= SLT_RES_ERR;
}

int mem_test_run(void)
{
	int i = 0;
	mem_exit_thread = 0;
	for (i = 0; i < THREAD_NUM; i++) {
		if( pthread_create(&mem_mthread[i], NULL,
					mem_test_thread, i ) < 0 ) {
			printf(" mem test fail\n");
			return -1;
		}
	}
	return 0;
}

int mem_stop(void)
{
	int32_t i;
	system("pkill memtester");
	mem_exit_thread = 1;
	for( i = 0 ; i < THREAD_NUM; i++ )
	{
		pthread_join(mem_mthread[i], NULL);
	}
	return SLT_RES_OK;
}

int mem_status(void)
{
	int32_t i, count = 0;
	for( i = 0; i < THREAD_NUM; i++ )
	{
		if( result[i] < 0 )
			return SLT_RES_ERR;
		else if( SLT_RES_OK == result[i] )
			count ++;
	}

	if(count == THREAD_NUM) {
		return SLT_RES_OK;
	}
	else {
		printf("MEM ERR\n");
		system("pkill memtester\n");
		return SLT_RES_TESTING;
	}
}

