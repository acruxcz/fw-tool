/*
 * tty.h
 *
 *  Created on: Jul 9, 2019
 *      Author: petr
 */

#ifndef TTY_H_
#define TTY_H_

#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>


typedef struct {
	int fd;
	struct termios tty_par;
}tty_t;


tty_t * tty_open(char * dev);
int 	tty_write(tty_t *  tty, char * buffer,int len);
int 	tty_read(tty_t *  tty, char * buffer,int len);
int 	tty_close(tty_t *  tty);

#endif /* TTY_H_ */
