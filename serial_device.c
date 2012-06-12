/* 
 * Defines functions for communicating with a generic Serial Device
 * copyright 2008 Joshua Shapiro 
 */
#include <stdlib.h>
//#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "serial_device.h"

#define BUFSIZE 255


/** Return the error string for the given error */
char *sd_errstring(int err)
{
  char *retval;
  switch(err) {
  case SERIAL_DEVICE_ERR_NULL: retval = "SERIAL_DEVICE structure is null."; break;
  case SERIAL_DEVICE_ERR_SELECT: retval = "select() returned error while reading device response."; break;
  case SERIAL_DEVICE_ERR_WRITE: retval = "Error writing data to device"; break;
  case SERIAL_DEVICE_ERR_READ: retval = "Error reading data from device"; break;
  case SERIAL_DEVICE_ERR_WRITE_EAGAIN: 
    retval = "Error writing: non-blocking device will block"; break;
  case SERIAL_DEVICE_ERR_WRITE_EBADF:
    retval = "Error writing: invalid file descriptor"; break;
  case SERIAL_DEVICE_ERR_WRITE_EFAULT:
    retval = "Error writing: buffer not addressable"; break;
  case SERIAL_DEVICE_ERR_WRITE_EFBIG:
    retval = "Error writing: file size exceeded"; break;
  case SERIAL_DEVICE_ERR_WRITE_EINTR:
    retval = "Error writing: call interrupted"; break;
  case SERIAL_DEVICE_ERR_WRITE_EINVAL:
    retval = "Error writing: unsuitable for writing"; break;
  case SERIAL_DEVICE_ERR_WRITE_EIO:
    retval = "Error writing: error modifying inode"; break;
  case SERIAL_DEVICE_ERR_WRITE_ENOSPC:
    retval = "Error writing: no room for data"; break;
  case SERIAL_DEVICE_ERR_WRITE_EPIPE:
    retval = "Error writing: receiving end cannot read"; break;

  default:
    retval = "Unknown error.";
  }
  return retval;
}

/**
 * Lookup the appropriate control integer to specify the 
 * given baud rate.  
 */
int sd_baud_lookup(int baudrate) {
  int retval;
  switch(baudrate) {
  case 0:     retval = B0; break;
  case 50:    retval = B50; break;
  case 75:    retval = B75; break;
  case 110:   retval = B110; break;
  case 134:   retval = B134; break;
  case 150:   retval = B150; break;
  case 200:   retval = B200; break;
  case 300:   retval = B300; break;
  case 600:   retval = B600; break;
  case 1200:  retval = B1200; break;
  case 1800:  retval = B1800; break;
  case 2400:  retval = B2400; break;
  case 4800:  retval = B4800; break;
  case 9600:  retval = B9600; break;
  case 19200: retval = B19200; break;
  case 38400: retval = B38400; break;
  case 57600: retval = B57600; break;
  case 115200: retval = B115200; break;
  case 230400: retval = B230400; break;
  default:
    retval = B0;
  }
  return retval;
}


/**
 * Send a message to the serial device
 * @param sd The serial device
 * @param message The message to send
 * @return error condition
 * Serial device response is stored in sd->last_response
 */
int sd_send_message(SERIAL_DEVICE sd, string_t message) 
{
	
  int err = SERIAL_DEVICE_OK;
	
  err =  sd_write(sd, message);
  if ( SERIAL_DEVICE_OK == err) {
    err = sd_read(sd) ;
  }
	
  return err;
}


/**
 * Read up to n bytes into data from the serial device
 * return the number of bytes actually read.
 * Warning.  This buffer will not be null terminated.
 * No error tracking either.
 */
int sd_read_nbytes(SERIAL_DEVICE sd, int n_bytes, char *data) 
{
  if (NULL == sd) {
    return -1;
  }

  int fd = sd->fd;
  int n;
  int ready = 1;
  fd_set fds;
  struct timeval timeout;


  // Setup fd_set
  FD_ZERO(&fds);
  FD_SET(fd, &fds);

  // Set 10 milli-second timeout
  timeout.tv_sec = 0;
  timeout.tv_usec = 10 * 1000;
  
  /* Wait for data to be ready */
  ready = select(fd+1, &fds, NULL, NULL, &timeout) ;
  
  if ( 0 < ready) {
    n =  read(fd, data, n_bytes);
  } else {
    n = 0;
  }
		
  return n;

}

/**
 * Issue a command to the pilot.
 * Return errno or 0
 */
int sd_read(SERIAL_DEVICE sd) 
{
  int fd = sd->fd;
  int err = SERIAL_DEVICE_OK;
  int n;
  int n_tot = 0;
  int ready = 1;
  int maxtimes = 100;
  fd_set fds;
  struct timeval timeout;
  char buf[BUFSIZE];

  if (NULL == sd) {
    err = SERIAL_DEVICE_ERR_NULL;
  }
	
  // Repeatedly read data until select times out
  while (0 < ready && SERIAL_DEVICE_OK == err && 0 < maxtimes-- )  {

    // Setup fd_set
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    // Set 100 milli-second timeout
    timeout.tv_sec = 0;
    timeout.tv_usec = 100 * 1000;
		
    /* Wait for data to be ready */
    ready = select(fd+1, &fds, NULL, NULL, &timeout) ;

    if ( 0 < ready) {
      n =  read(fd, buf, BUFSIZE);
      if ( 0 > n) {
	// Error in read
	err = SERIAL_DEVICE_ERR_READ;
      } else if (0 < n && ((n_tot + n) < BUFSIZE)) {
	// Append n bytes to return buffer, watch out for overflow
	memcpy(sd->last_response + n_tot, buf, n);
	n_tot += n;
      }
    }
  }

  if ( 0 > ready ) {
    // Select returned an error instead of a timeout
    err = SERIAL_DEVICE_ERR_SELECT;
  }

  /* Terminate String */
  sd->last_response[n_tot] = '\0';	 
	
  /* Replace newlines with space*/
  for (n=0; n < n_tot; n++) {
    if ( sd->last_response[n] == '\n' ) {
      sd->last_response[n] = ' ';
    }
  }
	
  /*   Strip leading and trailing space */
  n = 0;
  while (n < n_tot && ' ' == sd->last_response[n]) { n++; }
  strcpy(buf, sd->last_response + n);
	
  n = strlen(buf) - 1;
  while (n > 0 && ' ' == buf[n]) {n--;}
  buf[n+1] = '\0';
  strcpy(sd->last_response, buf);
	
  // Ideally this is SERIAL_DEVICE_OK
  return err;
}

/**
 * Send a command to the pilot
 * This function will automatically append the <cr>
 */
int sd_write(SERIAL_DEVICE sd, const  string_t command) 
{
  int fd = sd->fd;
  int err = SERIAL_DEVICE_OK;
  int n, i;
  char c;
  int size = strlen(command);
  fd_set fds;
  struct timeval timeout;
  int ready;

  if (NULL == sd) {
    err = SERIAL_DEVICE_ERR_NULL;
  }

/*   // Setup fd_set */
/*   FD_ZERO(&fds); */
/*   FD_SET(fd, &fds); */
  
/*   // Set 100 milli-second timeout */
/*   timeout.tv_sec = 0; */
/*   timeout.tv_usec = 100 * 1000; */
  
/*   /\* Wait for data to be ready *\/ */
/*   ready = select(fd+1, NULL, &fds, NULL, &timeout) ; */
	
/*   if (0 < ready) { */
/*       err = SERIAL_DEVICE_ERR_WRITE; */
/*   } */
  
  for (i = 0; i < size && SERIAL_DEVICE_OK == err; i++) {
    c = command[i];
    n =  write(fd, &c, 1);
    if (1  != n) {
      err = SERIAL_DEVICE_ERR_WRITE;
    }
  }
  if (SERIAL_DEVICE_OK == err) {
    c = 13;
    if (1 != write(fd, &c, 1)) {
      err = SERIAL_DEVICE_ERR_WRITE;
    }
  }


  // Figure out exactly what happened
  if (SERIAL_DEVICE_ERR_WRITE == err) {
    int errsv = errno;
    switch (errsv) {
    case EAGAIN:
      err = SERIAL_DEVICE_ERR_WRITE_EAGAIN; break;
    case EBADF:
      err = SERIAL_DEVICE_ERR_WRITE_EBADF; break;
    case EFAULT:
      err = SERIAL_DEVICE_ERR_WRITE_EFAULT; break;
    case EFBIG:
      err = SERIAL_DEVICE_ERR_WRITE_EFBIG; break;
    case EINTR:
      err = SERIAL_DEVICE_ERR_WRITE_EINTR; break;
    case EINVAL:
      err = SERIAL_DEVICE_ERR_WRITE_EINVAL; break;
    case EIO:
      err = SERIAL_DEVICE_ERR_WRITE_EIO; break;
    case ENOSPC:
      err = SERIAL_DEVICE_ERR_WRITE_ENOSPC; break;
    case EPIPE:
      err = SERIAL_DEVICE_ERR_WRITE_EPIPE; break;
    default:
      err = SERIAL_DEVICE_ERR_WRITE; break;
    }
  }

  return err;
}


SERIAL_DEVICE sd_init(char *device)
{
  return sd_initWithDevice_baudrate_dataBits_stopBits_parity_flowControl(device, B9600, 8, 1, SERIAL_DEVICE_PARITY_NONE, 0);
}
/**
 * Initialize a serial device for communication
 * Returns a SERIAL_DEVICE handle which can be used
 * for reading and writing to the device
 */
SERIAL_DEVICE sd_initWithDevice_baudrate_dataBits_stopBits_parity_flowControl(char *device, int baudrate, int data, int stop, int parity, int flow_control) 
{

  int cflags = CREAD;
  int oflags = 0;
  int iflags = IGNPAR;

  /** Valid data bits are 5 - 8 */
  switch(data) {
  case 5: cflags |= CS5; break;
  case 6: cflags |= CS6; break;
  case 7: cflags |= CS7; break;
  case 8: cflags |= CS8; break;
  default:
    return NULL;
  }

  /** Valid stop bits are 1,2 */
  switch(stop) {
  case 1: break;
  case 2: cflags |= CSTOPB; break;
  default:
    return NULL;
  }

  /** Valid parity odd,even, none*/
  switch(parity) {
  case SERIAL_DEVICE_PARITY_NONE: break;
  case SERIAL_DEVICE_PARITY_EVEN: cflags |= PARENB; break;
  case SERIAL_DEVICE_PARITY_ODD: cflags |= (PARENB | PARODD); break;
  default:
    return NULL;
  }

  switch(flow_control) {
  case 1: cflags |= CRTSCTS; break;
  }

  SERIAL_DEVICE sd = (SERIAL_DEVICE)malloc(sizeof(SERIAL_DEVICE_T));

  int fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (0 < fd) {

    sd->fd = fd;
    sd->oldtio = malloc(sizeof(struct termios));
    sd->tio = malloc(sizeof(struct termios));
    sd->last_response = (char *)malloc(sizeof(char)*BUFSIZE);
		
    tcgetattr(fd, sd->oldtio);
		
    /* 
       BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
       CRTSCTS : output hardware flow control (only used if the cable has
       all necessary lines. See sect. 7 of Serial-HOWTO)
       CS8     : 8n1 (8bit,no parity,1 stopbit)
       CLOCAL  : local connection, no modem contol
       CREAD   : enable receiving characters
       IGNPAR  : ignore bytes with parity errors
       IGNCR : Ignore CR on input
       A read will return when at least 1 byte of data is available or after 0.1*VTIME seconds expire
    */
    cfmakeraw(sd->tio);
    cfsetspeed(sd->tio, baudrate);
    sd->tio->c_cflag = cflags;
    sd->tio->c_oflag = oflags;
    sd->tio->c_iflag = iflags;
    sd->tio->c_cc[VMIN] = 0;
    sd->tio->c_cc[VTIME] = 0;
		 
    /* 
       now clean the modem line and activate the settings for the port
    */
    tcflush(fd, TCIOFLUSH);
    if ( 0 > tcsetattr(fd, TCSANOW, sd->tio) ) {
      sd_destroy(sd);
      return NULL;
    }
		
    /*
      Was having some issues with Mac vs. linux as to when to set the baudrate, so i just do it twice :)
    */
    cfsetspeed(sd->tio, baudrate);
    if ( 0 > tcsetattr(fd, TCSANOW, sd->tio) ) {
      sd_destroy(sd);
      return NULL;
    }
		
  } else {
    free(sd);
    return NULL;
  }
	
  return sd;
}

/**
 * Close file descripter and reset attributes
 * doesn't free memory
 */
void sd_close(SERIAL_DEVICE sd) 
{
  if (NULL != sd && 0 < sd->fd) {
    tcflush(sd->fd, TCIOFLUSH);
    tcsetattr(sd->fd, TCSANOW, sd->oldtio);
    close(sd->fd);
    sd->fd = 0;
  }
}

/**
 * Free resources consumed by a SERIAL_DEVICE 
 */
void sd_destroy(SERIAL_DEVICE sd) 
{
  if (NULL != sd) { 
    sd_close(sd);
    free(sd->oldtio);
    free(sd->tio);
    free(sd->last_response);
    free(sd);
  }
}
