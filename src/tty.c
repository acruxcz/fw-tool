/*
 * tty.c
 *
 *  Created on: Jul 9, 2019
 *      Author: petr
 */



#include "tty.h"

tty_t * tty_open(char * dev)
{
	tty_t * tty = malloc(sizeof(tty_t));
	tty->fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);

	if (tty->fd < 0) {
		free(tty);
		return NULL;
	}
	fcntl(tty->fd, F_SETFL, 0);
	//Get prarameters
	tcgetattr (tty->fd, &tty->tty_par);
	//Set BR
	cfsetospeed(&tty->tty_par, (speed_t)115200);
	cfsetispeed(&tty->tty_par, (speed_t)115200);

	cfmakeraw(&tty->tty_par);
//	tty->tty_par.c_cflag 		= CLOCAL | CREAD | PARENB | CS8; // 8b, read en, even parity
	tty->tty_par.c_cflag |= CLOCAL | CREAD;
	tty->tty_par.c_cflag &= ~PARENB;
	tty->tty_par.c_cflag &= ~CSTOPB;
	tty->tty_par.c_cflag &= ~CSIZE;
	tty->tty_par.c_cflag &= ~CRTSCTS;
	tty->tty_par.c_cflag |= CS8 | PARENB ;

	tty->tty_par.c_cc[VMIN]		= 0;//1; //NonBlocking Read
	tty->tty_par.c_cc[VTIME]	= 10; //100 ms read timeout



	tcsetattr(tty->fd, TCSANOW, &tty->tty_par);
	tcflush  (tty->fd, TCIFLUSH);


	return tty;
}

int tty_write(tty_t *  tty, char * buffer,int len)
{
	/*
	int i;
	int w = 0;
	for(i=0;i<len;i++)
	{
		w += write( tty->fd, &buffer[i], 1);
	  usleep(10000);
	}


	if(w == len )return 0;
	else return -1;
	*/
	 write( tty->fd, buffer, len);
	 tcdrain(tty->fd);
	 return 0;
}


int tty_read(tty_t *  tty, char * buffer,int len)
{
	int timeout = 10;
	int i;
	int r = 0;
	for(i=0;i<len;i++)
	{
		/*timeout = 100;
		while(timeout--)
		{*/
			r = read( tty->fd, &buffer[i], 1);
	/*		if(r != 0) break;
		}*/
	}
	return r;
}

int tty_close(tty_t *  tty)
{
	close(tty->fd);
	free(tty);
	return 0;
}
