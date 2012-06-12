
#include "serial_device.h"
#include <stdio.h>

int main(int argc, char **argv) {

  SERIAL_DEVICE sd;
  char *message = "RD";
  int err;
  char buf[10];
  printf("Initing...\n");
  sd = sd_init("/dev/ttyS0");

  sd_send_message(sd, message);
/*   fprintf(stdout, "Writing...\n"); */
/*   err =  sd_write(sd, message); */
/*   if ( SERIAL_DEVICE_OK == err) { */
/*     printf("Reading...\n"); */
/*     err = sd_read(sd); */
/*   } else { */
/*     printf("Error...\n"); */
/*   } */

  printf("%s\n", sd->last_response);
  printf("Cleanup..\n");
  sd_destroy(sd);
  return 0;
}
