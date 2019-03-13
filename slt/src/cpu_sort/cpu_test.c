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

pthread_t	cpu_mthread[THREAD_NUM];
int		result[THREAD_NUM];
int		cpu_exit_thread;

static void *cpu_test_thread(int id)
{
        uint8_t *pInBuf = (uint8_t *)malloc(CPU_TEST_SIZE);
        uint8_t *pZipBuf = (uint8_t *)malloc(CPU_TEST_SIZE);
        uint8_t *pUnzipBuf = (uint8_t *)malloc(CPU_TEST_SIZE);
        uint32_t zipSize, unzipSize;
	int i = 0;

        if( !pInBuf || !pZipBuf || !pUnzipBuf )
        {
                printf("Not enought memory(id=%d)\n", id);
        }

        //      Make Input Pattern
        for(i = 0; i < CPU_TEST_SIZE; i++ )
        {
                pInBuf[i] = i%256;
        }

        while( !cpu_exit_thread )
        {
                {
                        zipSize = CPU_TEST_SIZE;
                        if( Z_OK != compress( pZipBuf, (uLongf*)&zipSize, pInBuf, CPU_TEST_SIZE ) )
                        {
                                goto ErrorExit;
                        }
                        unzipSize = CPU_TEST_SIZE;
                        if( Z_OK != uncompress( pUnzipBuf, (uLongf*)&unzipSize, pZipBuf, zipSize ) )
                        {
                                goto ErrorExit;
                        }
                        // Compare
                        for( i=0 ; i<CPU_TEST_SIZE ; i++ )
                        {
                                if( pUnzipBuf[i] != (i%256) )
                                        goto ErrorExit;
                        }
                }
        }

        result[id] = SLT_RES_OK;
        free(pInBuf);
        free(pZipBuf);
        free(pUnzipBuf);
        return;
ErrorExit:
        printf("unzip Error (id=%d)\n", id);
        result[id] = SLT_RES_ERR;
        free(pInBuf);
        free(pZipBuf);
        free(pUnzipBuf);
        return;
}

int cpu_test_run(void)
{
	int i = 0;
	cpu_exit_thread = 0;
	for (i = 0; i < THREAD_NUM; i++) {
		if( pthread_create(&cpu_mthread[i], NULL,
					cpu_test_thread, i ) < 0 ) {
			printf(" cpu test fail\n");
			return -1;
		}
	}
	return 0;
}

int cpu_stop(void)
{
	int32_t i;
	cpu_exit_thread = 1;
	for( i = 0 ; i < THREAD_NUM; i++ )
	{
		pthread_join(cpu_mthread[i], NULL);
	}
	return SLT_RES_OK;
}

int cpu_status(void)
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
		printf("ERR\n");
		return SLT_RES_TESTING;
	}
}

