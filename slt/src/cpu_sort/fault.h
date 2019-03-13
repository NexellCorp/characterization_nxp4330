#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sched.h>		/* schedule */
#include <linux/sched.h>	/* SCHED_NORMAL, SCHED_FIFO, SCHED_RR, SCHED_BATCH */
#include <fcntl.h>
#include <string.h>
#include <errno.h>		/* error */
#include <mntent.h>
#include <sys/mount.h>
#include <pthread.h>
#include <sys/vfs.h>
#include <sys/stat.h> /* stat */ 
#include "gpio.h"

#define RETRYCNT	200;
#define KBYTE		(1024)
#define MBYTE		(1024 * KBYTE)
#define GBYTE		(1024 * MBYTE)

#define	NUM_OF_CORE	4
#define THREAD_NUM	NUM_OF_CORE 
#define TEST_SIZE	(128 * KBYTE)

#define HOST_PWR_ON_PIN 17 /* GPIOA 17 */
typedef enum{
	SLT_RES_ERR     = -1,
	SLT_RES_OK      = 0,
	SLT_RES_TESTING = 1
} SLT_RESULT;

