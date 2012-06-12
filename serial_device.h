
#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

#include <sys/types.h>

#define BAUDRATE B57600
#define SERIAL_DEVICE_OK 0
#define SERIAL_DEVICE_ERR_NULL -1
#define SERIAL_DEVICE_ERR_SELECT -2
#define SERIAL_DEVICE_ERR_WRITE -3
#define SERIAL_DEVICE_ERR_READ -4
#define SERIAL_DEVICE_ERR_NOTOK -5

#define SERIAL_DEVICE_ERR_WRITE_EAGAIN -6
#define SERIAL_DEVICE_ERR_WRITE_EBADF -7
#define SERIAL_DEVICE_ERR_WRITE_EFAULT -8
#define SERIAL_DEVICE_ERR_WRITE_EFBIG -9
#define SERIAL_DEVICE_ERR_WRITE_EINTR -10
#define SERIAL_DEVICE_ERR_WRITE_EINVAL -11
#define SERIAL_DEVICE_ERR_WRITE_EIO -12
#define SERIAL_DEVICE_ERR_WRITE_ENOSPC -13
#define SERIAL_DEVICE_ERR_WRITE_EPIPE -14


#define SERIAL_DEVICE_PARITY_EVEN 2
#define SERIAL_DEVICE_PARITY_ODD 1
#define SERIAL_DEVICE_PARITY_NONE 0

// The SERIAL_DEVICE Data Type
typedef struct {
	int fd;
	struct termios* oldtio;
	struct termios* tio;
	char *last_response;
} SERIAL_DEVICE_T;

// Pointer to the data type
typedef SERIAL_DEVICE_T* SERIAL_DEVICE;

// Define a string type for readability
typedef char* string_t;

/** Return the error string for the given error */
char *sd_errstring(int err);

/** Lookup the baudrate control integer for the given rate */
int sd_baud_lookup(int baudrate);

/**
 * Query the device. Resopnse is inserted in sd->last_response
 * @param sd The SERIAL_DEVICE type as returned by sd_init
 * @param query The string_t message
 * @returns SERIAL_DEVICE_OK upon success.  Check sd->last_response for response value
 */
int sd_send_message(SERIAL_DEVICE sd, string_t message);

/**
 * Read a response from the device into the sd->last_response field.
 * @param sd The device  as returned by sd_init
 * @returns SERIAL_DEVICE_OK on success, 
 */ 
int sd_read(SERIAL_DEVICE sd);

/**
 * Read up to n bytes from the serial device into the buffer.
 * @return the number of bytes actually read
 */
int sd_read_nbytes(SERIAL_DEVICE sd, int n, char *buf);

/**
 * Write a command to the pilot
 * @param sd The SERIAL_DEVICE as returned by sd_init
 * @param command The command to send
 * @returns 0 on success, errno on error
*/
int sd_write(SERIAL_DEVICE sd, const string_t command);

/**
 * Open the communication port to the pilot.  Returns NULL upon error.
 *
 * @param device The device to which the pilot is connected i.e. /dev/ttyS0, /dev/ttyUSB0
 * @returns the file descriptor to be passed to other functions in this library 
 */
SERIAL_DEVICE sd_init(char *device);

SERIAL_DEVICE sd_initWithDevice_baudrate_dataBits_stopBits_parity_flowControl(char *device, int baudrate, int dataBits, int stopBits, int parity, int flow_control);

/**
 * Close the communication port with the pilot
 * @param pilot The pilot type as returned by pilot_init
 */
void sd_destroy(SERIAL_DEVICE sd);

#endif
