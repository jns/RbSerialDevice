#include "ruby.h"
#include "serial_device.h"


VALUE cSerialDevice;

VALUE DEVICE_SYMBOL;
VALUE BAUDRATE_SYMBOL;
VALUE DATA_BITS_SYMBOL;
VALUE STOP_BITS_SYMBOL;
VALUE PARITY_SYMBOL;
VALUE NONE_SYMBOL;
VALUE ODD_SYMBOL;
VALUE EVEN_SYMBOL;
VALUE HARDWARE_FLOW_CONTROL_SYMBOL;

/**
 * Write a message to the serial device and
 * return the response.
 */
VALUE rsd_send_message(VALUE self, VALUE message)
{
	SERIAL_DEVICE sd;
	Data_Get_Struct(self, SERIAL_DEVICE_T, sd);
	int err = sd_send_message(sd, (string_t)STR2CSTR(message));
	if ( SERIAL_DEVICE_OK == err) {
		return rb_str_new2(sd->last_response);
	} else {
		rb_raise(rb_eException, sd_errstring(err));
	}	
}

/**
 * Read up to a given number of bytes from the SerialDevice.
 */
VALUE rsd_read_nbytes(VALUE self, VALUE fixnum_bytes) 
{
  SERIAL_DEVICE sd;
  Data_Get_Struct(self, SERIAL_DEVICE_T, sd);
  
  int nbytes = NUM2INT(fixnum_bytes);
  unsigned char buf[nbytes];
  VALUE array = rb_ary_new();

  nbytes = sd_read_nbytes(sd, nbytes, buf);

  if (0 == nbytes) {
    return Qnil;
  } else while (nbytes-- > 0) {
    rb_ary_unshift(array, INT2FIX(buf[nbytes]));
  }
  
  return array;
}

/**
 * Read bytes from the serial device.  
 * Number of bytes read is limited by serial_device.c
 */
VALUE rsd_read(VALUE self)
{
  SERIAL_DEVICE sd;
  Data_Get_Struct(self, SERIAL_DEVICE_T, sd);
  int err = sd_read(sd);
  if ( SERIAL_DEVICE_OK == err ) {
    return rb_str_new2(sd->last_response);
  } else {
    rb_raise(rb_eException, sd_errstring(err));
  }
}

/**
 * Write a string to the serial device.
 * String is automatically terminated with ASCII 13 (CR)
 */
VALUE rsd_write(VALUE self, VALUE string)
{
  SERIAL_DEVICE sd;
  Data_Get_Struct(self, SERIAL_DEVICE_T, sd);
  int err = sd_write(sd, STR2CSTR(string));
  if ( SERIAL_DEVICE_OK == err ) {
    return Qnil;
  } else {
    rb_raise(rb_eException, sd_errstring(err));
  }
}

VALUE rsd_init(VALUE self, VALUE device) 
{
	return self;
}

/**
 * Issue command to close the underlying file
 * Resources are not freed until it is garbage collected
 */
VALUE rsd_close(VALUE self)
{
  SERIAL_DEVICE sd;
  Data_Get_Struct(self, SERIAL_DEVICE_T, sd);
  sd_close(sd);

  return Qnil;
}

/**
 * Create a new Serial Device.
 * Takes an options hash which must recognizes the following keys
 *   :device,  required. 
 *   :baud,    default=9600
 *   :parity,  one of :none,:even,:odd. Default=:none
 *   :stop_bits, 1 or 2, default=1
 *   :data_bits, 5,6,7,8, default=8
 *   :hw_flow,   true or false default = false
 */
 VALUE rsd_new(VALUE sdClass,  VALUE options) 
{

	VALUE options_value;

	VALUE argv[1];
	char *device;
	int baud_default = 9600;
	VALUE parity_default = NONE_SYMBOL;
	int data_default = 8;
	int stop_default = 1;
	int flow_control_default = 0;

	int baudrate = baud_default;
	VALUE parity_value = parity_default;
	int data_bits = data_default;
	int stop_bits = stop_default;
	int parity;
	int flow_control = flow_control_default;
	
	  Check_Type(options, T_HASH);

	  options_value = rb_hash_aref(options, DEVICE_SYMBOL);
	  if (RTEST(options_value)) {
	    Check_Type(options_value, T_STRING);
	    argv[0] = options_value;
	    device = STR2CSTR(options_value);
	  } else {
	    rb_raise(rb_eException, ":device must be specified");
	  }

	  options_value = rb_hash_aref(options, BAUDRATE_SYMBOL);
	  if (RTEST(options_value)) {
	    baudrate = sd_baud_lookup(NUM2INT(options_value));
	  } else {
	    baudrate = sd_baud_lookup(baud_default);
	  }

	  options_value = rb_hash_aref(options, DATA_BITS_SYMBOL);
	  if (RTEST(options_value)) {
	    data_bits = NUM2INT(options_value);
	  } else {
	    data_bits = data_default;
	  }

	  options_value = rb_hash_aref(options, PARITY_SYMBOL);
	  if (RTEST(options_value)) {
	    parity_value = options_value;
	  } else {
	    parity_value = parity_default;
	  }

	  options_value = rb_hash_aref(options, STOP_BITS_SYMBOL);
	  if (RTEST(options_value)) {
	    stop_bits = NUM2INT(options_value);
	  } else {
	    stop_bits = stop_default;
	  }

	  options_value = rb_hash_aref(options, HARDWARE_FLOW_CONTROL_SYMBOL);
	  if (RTEST(options_value)) {
	    if (options_value == Qtrue) {
	      flow_control = 1;
	    } else {
	      flow_control = 0;
	    } 
	  } else {
	    flow_control = 0;
	  }

	if (data_bits < 5 || data_bits > 8) {
	  rb_raise(rb_eException, "Data bits must be between 5 and 8");
	}

	if (stop_bits != 1 && stop_bits != 2) {
	  rb_raise(rb_eException, "Stop bits must be either 1 or 2");
	}

	if (data_bits < 8 && parity_value == NONE_SYMBOL) {
	  rb_raise(rb_eException, "Parity must be :odd or :even");
	}

	if (data_bits == 8) {
	  parity = NONE_SYMBOL;
	}

	if (parity_value == NONE_SYMBOL) {
	  parity = SERIAL_DEVICE_PARITY_NONE;
	} else if (parity_value == ODD_SYMBOL) {
	  parity = SERIAL_DEVICE_PARITY_ODD;
	} else if (parity == EVEN_SYMBOL) {
	  parity = SERIAL_DEVICE_PARITY_EVEN;
	} else {
	  rb_raise(rb_eException, "Parity must be :odd, :even, or :none");
	}

	SERIAL_DEVICE sd = 
	  sd_initWithDevice_baudrate_dataBits_stopBits_parity_flowControl(device, 
							      baudrate, 
							      data_bits, 
							      stop_bits, 
							      parity,
							      flow_control);
	
	if ( NULL == sd ) {
		// Throw
		rb_raise(rb_eException, "Error initializing device");
		return Qnil;
	} else {
		// Wrap pilot in a ruby object
		// Pass in free routine for garbage collector
		VALUE tdata = Data_Wrap_Struct(sdClass, 0, sd_destroy, sd); 
		rb_obj_call_init(tdata, 1, argv);
		return tdata;
	}
}

void Init_RbSerialDevice() 
{
    cSerialDevice = rb_define_class("SerialDevice", rb_cObject);
    rb_define_singleton_method(cSerialDevice, "new", rsd_new, 1);
    rb_define_method(cSerialDevice, "initialize", rsd_init, 1);
    rb_define_method(cSerialDevice, "send_message", rsd_send_message, 1);
    rb_define_method(cSerialDevice, "read", rsd_read, 0);
    rb_define_method(cSerialDevice, "write", rsd_write, 1);
    rb_define_method(cSerialDevice, "read_bytes", rsd_read_nbytes, 1);
    rb_define_method(cSerialDevice, "close", rsd_close, 0);

    DEVICE_SYMBOL = ID2SYM(rb_intern("device"));
    BAUDRATE_SYMBOL = ID2SYM(rb_intern("baud"));
    PARITY_SYMBOL = ID2SYM(rb_intern("parity"));
    DATA_BITS_SYMBOL = ID2SYM(rb_intern("data_bits"));
    STOP_BITS_SYMBOL = ID2SYM(rb_intern("stop_bits"));
    NONE_SYMBOL = ID2SYM(rb_intern("none"));
    ODD_SYMBOL = ID2SYM(rb_intern("odd"));
    EVEN_SYMBOL = ID2SYM(rb_intern("even"));
    HARDWARE_FLOW_CONTROL_SYMBOL = ID2SYM(rb_intern("hw_flow"));
}
