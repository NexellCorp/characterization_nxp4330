#include "uart_test.h"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <errno.h>
#include <time.h>
#include <linux/kernel.h>

struct termios newtio;
struct termios oldtio;

static struct sig_param s_par;
static struct sig_param *par = &s_par;

static int get_baudrate(int op_baudrate)
{
	int speed;

	switch (op_baudrate) {
		case 0    	: speed =       B0; break;
		case 50   	: speed =      B50; break;
		case 75   	: speed =      B75; break;
		case 110  	: speed =     B110; break;
		case 134  	: speed =     B134; break;
		case 150  	: speed =     B150; break;
		case 200  	: speed =     B200; break;
		case 300  	: speed =     B300; break;
		case 600  	: speed =     B600; break;
		case 1200 	: speed =    B1200; break;
		case 1800 	: speed =    B1800; break;
		case 2400 	: speed =    B2400; break;
		case 4800 	: speed =    B4800; break;
		case 9600 	: speed =    B9600; break;
		case 19200	: speed =   B19200; break;
		case 38400	: speed =   B38400; break;
	  	case 57600  : speed =   B57600; break;
  		case 115200 : speed =  B115200; break;
  		case 230400 : speed =  B230400; break;
  		case 460800 : speed =  B460800; break;
  		case 500000 : speed =  B500000; break;
  		case 576000 : speed =  B576000; break;
  		case 921600 : speed =  B921600; break;
  		case 1000000: speed = B1000000; break;
  		case 1152000: speed = B1152000; break;
  		case 1500000: speed = B1500000; break;
  		case 2000000: speed = B2000000; break;
  		case 2500000: speed = B2500000; break;
  		case 3000000: speed = B3000000; break;
  		case 3500000: speed = B3500000; break;
  		case 4000000: speed = B4000000; break;
		default:
			printf("Fail, not support op_baudrate, %d\n", op_baudrate);
			return -1;
	}
	return speed;
}

int uart_init(void)
{
	int fd;
	int opt;
	speed_t speed;

	char ttypath[64] = TTY_NAME;
	int op_baudrate = TTY_BAUDRATE;
	int op_canoical = NON_CANOICAL;				//Canonical mode Default non
	//int op_test = TEST_MASTER;
	int op_charnum  = TTY_NC_CHARNUM;
	int op_timeout  = TTY_NC_TIMEOUT;
	int op_flow	= 0;
	int op_stop = 0;
	int op_parity = 0;
	int op_odd =0;
	int op_data = 8;
	int ret=0;

	fd = open(ttypath, O_RDWR| O_NOCTTY);	// open TTY Port
	if (0 > fd) {
		printf("Fail, open '%s', %s\n", ttypath, strerror(errno));
		return -1;
		//exit(1);
	}

	/*
	 * save current termios
	 */
	tcgetattr(fd, &oldtio);

	par->fd = fd;
	memcpy(&par->tio, &oldtio, sizeof(struct termios));

 	speed = get_baudrate( op_baudrate);

	if(!speed)
		goto _exit;
	memcpy(&newtio, &oldtio, sizeof(struct termios));

	newtio.c_cflag &= ~CBAUD;	// baudrate mask
	newtio.c_iflag 	&= ~ICRNL;
	newtio.c_cflag |=  speed; 

	newtio.c_cflag &=   ~CS8;
	switch (op_data)
	{	
		case 5:
			newtio.c_cflag |= CS5; 
			break;
		case 6:
			newtio.c_cflag |= CS6;
			break;
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
		default :
			printf("Not Support  %d data len, Setting 8bit Data len. \n",op_data);
			newtio.c_cflag |= CS8;
			break;
	}

	newtio.c_iflag 	|= IGNBRK|IXOFF;

	newtio.c_cflag 	&= ~HUPCL;
	if(op_flow)
		newtio.c_cflag |= CRTSCTS;   /* h/w flow control */
	else
	newtio.c_cflag &= ~CRTSCTS;  /* no flow control */
  
	newtio.c_iflag 	|= 0;	// IGNPAR;
	newtio.c_oflag 	|= 0;	// ONLCR = LF -> CR+LF
	newtio.c_oflag 	&= ~OPOST;

	newtio.c_lflag 	= 0;
	newtio.c_lflag 	= op_canoical ? newtio.c_lflag | ICANON : newtio.c_lflag & ~ICANON;	// ICANON (0x02)

	newtio.c_cflag	= op_stop ? newtio.c_cflag | CSTOPB : newtio.c_cflag & ~CSTOPB;	//Stop bit 0 : 1 stop bit 1 : 2 stop bit

	newtio.c_cflag	= op_parity ? newtio.c_cflag | PARENB : newtio.c_cflag & ~PARENB; //0: No parity bit, 1: Parity bit Enb , 
	newtio.c_cflag	= op_odd ? newtio.c_cflag | PARODD : newtio.c_cflag & ~PARODD; //0: No parity bit, 1: Parity bit Enb , 


	if (!(ICANON & newtio.c_lflag)) {
	 		newtio.c_cc[VTIME] 	= op_timeout; 	// time-out °ªÀ¸·Î »ç¿ëµÈ´Ù. time-out = TIME* 100 ms,  0 = ¹«ÇÑ
	 		newtio.c_cc[VMIN] 	= op_charnum; 	// MINÀº read°¡ ¸®ÅÏµÇ±â À§ÇÑ ÃÖ¼ÒÇÑÀÇ ¹®ÀÚ °³¼ö.
	}

	tcflush  (fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	return fd;
_exit:
	close(fd);
	return -1;
}


