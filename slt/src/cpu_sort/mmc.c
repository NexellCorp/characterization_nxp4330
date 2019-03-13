#include "fault.h"

#define DEVICE_PATH	"/mnt/mmc"
#define CH_NUM	3
#define DEBUG	0

#if (DEBUG)
#define DBG_MSG(msg...)	{ printf("[mmc test]: " msg); }
#else
#define DBG_MSG(msg...)	do{} while(0);
#endif

pthread_t	mmc_mthread[THREAD_NUM];
int		result[THREAD_NUM];
int		mmc_exit_thread;

int mount_mmc(void)
{
	int ret = 0;
	int i = 0;
	unsigned char buf[128];
	unsigned char c_buf[128];

	for (i = 0; i < CH_NUM	; i++) {
		sprintf(buf, "/mnt/mmc%d", i);
		mkdir(buf, 0755);

		sprintf(c_buf, "mount /dev/mmcblk%dp1 %s", i, buf);
		ret = system(c_buf);
		if(ret)
			return ret;
	}
	return ret;
}

int unmount_mmc(void)
{
	int ret = 0;
	int i = 0;
	unsigned char buf[128];
	unsigned char c_buf[128];

	for (i = 0; i < CH_NUM; i++) {
		sprintf(c_buf, "umount /dev/mmcblk%dp1 /mnt/mmc%d", i, i);
		ret = system(c_buf);
		if(ret) {
			printf("MMC mount Fail\n");
			return ret;
		}
	}
	return ret;
}
static int mmc_rw_test(unsigned char *w, unsigned char *r, int size, int index )
{
	int ret, id;
	int fd;
	char filepath[256];

	if (index >= CH_NUM)
		id = index % (CH_NUM);
	else 
		id = index;

	sprintf( filepath, "%s%d/%s%d.dat", "/mnt/mmc", id, "rw_test", index);
	fd = open( filepath, O_CREAT|O_RDWR|O_SYNC );
	if(fd <0)
	{
		printf("open err (for write)\n ");
		return -1;
	}
	lseek(fd,512,0);
	ret = write(fd,w,size);
	close(fd);

	fd = open( filepath, O_CREAT|O_RDWR|O_SYNC );
	if( fd < 0 )
	{
		printf("open err (for read)\n");
		return -1;
	}
	lseek(fd,512,0);
	ret = read(fd,r,size);
	ret = memcmp(w,r,size);
	close(fd);
	return ret;
}

static void *mmc_test_thread(int id)
{
	//int id = (void *)num;
	int size = TEST_SIZE;
	int i = 0, ret = -1 ;
	unsigned char *wbuf = NULL;
	unsigned char *rbuf = NULL;

	wbuf = malloc(TEST_SIZE);
	rbuf = malloc(TEST_SIZE);

	if(wbuf == NULL  || rbuf== NULL)
	{
		printf("Alloc buffer error\n");
		if(wbuf)
			free(wbuf);
		if(rbuf)
			free(rbuf);
		return (int *)-1;
	}

	srand(time(NULL));
	for(i=0;i<size;i++)
	{
		wbuf[i] = rand()%0xff;
#if (DEBUG)
		if(i% 16 ==0)
			printf("\n");
		printf("%2x ",wbuf[i] );
#endif
	}

	while (!mmc_exit_thread)
	{
		memset(rbuf , 0, TEST_SIZE);      //      Clear Read Buffer
		ret = mmc_rw_test(wbuf, rbuf, TEST_SIZE , id);
		if(ret != 0)
		{
			printf("mmc test fail\n");
			result[id] = SLT_RES_ERR;
			goto out;
		}

	}
	printf("MMC: end of thread (%d)\n", id);

	return 0;
out:
	free(wbuf);
	free(rbuf);
	return (int *)-1;
}

int mmc_test_run(void)
{
	int i = 0, ret = 0;
	mmc_exit_thread = 0;
	ret = mount_mmc();
	if (ret) {
		printf("mmc mount fail\n");
		return -1;
	}

	for (i = 0; i < THREAD_NUM; i++) {
		if( pthread_create(&mmc_mthread[i], NULL,
					mmc_test_thread, i ) < 0 ) {
			printf(" mmc test fail\n");
			return -1;
		}
	}
	return 0;
}

int mmc_stop(void)
{
	int32_t i;
	mmc_exit_thread = 1;
	for( i = 0 ; i < THREAD_NUM; i++ )
	{
		pthread_join(mmc_mthread[i], NULL);
	}
	return SLT_RES_OK;
}



int mmc_status(void)
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

