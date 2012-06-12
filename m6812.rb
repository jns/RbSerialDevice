#
# m6812.rb
#
# copyright 2008, Joshua shapiro
#
# M6812 data acquisition subclass of SerialDevice
#
require 'RbSerialDevice'
require 'serial_device_accessors.rb'

class M6812 < Base

  sd_reader "IDN", :identity
  sd_reader "TIME", :time
#  sd_reader "POINTS", :num_sample_points
#  sd_reader "RLENGTH", :record_length

  # Return the number of sample points taken when 
  # the sample command is issued.  Only called once
  # and the result is cached.  
  def num_sample_points
    unless @num_sample_points
      @num_sample_points = self.send_message("POINTS").to_i
    end
    @num_sample_points
  end

  # return the length of a row of data in bytes
  # as returned by sample.  
  # For now this is hardcoded.  Should be setup to query
  # the board for this.
  def record_length
    unless @row_size
      @row_size = self.send_message("ROWSIZE").to_i
    end
    @row_size
  end

  # Instruct the board to sample
  # returns an array of bytes of length record_length*num_sample_points
  # Because there have a priori knowledge of the structure of that 
  # it is split into consituent data pieces and returned as a hash
  def sample
    points = self.num_sample_points
    size = points * self.record_length
    self.write("SAMPLE")
    arr = get_n_bytes(size)
    time = []
    quad = []
    ch0 = []
    ch1 = []
    begin
      # This reconstructs a two byte integer from two bytes
      time << (256*arr.shift.to_i + arr.shift.to_i)
      quad << arr.shift
      ch0 << (256*arr.shift.to_i + arr.shift.to_i)
      ch1 << (256*arr.shift.to_i + arr.shift.to_i)
#      ch2 << (256*arr.shift.to_i + arr.shift.to_i)
    end until arr.empty?
    return {:time => time, :quadrant => quad, :ch0 => ch0, :ch1 => ch1}
  end


  # Guarantee to read n bytes
  # raises an exception if it cant.
  def get_n_bytes(n)

    out = []
    tries = 0
    while (n > out.size)  
      data = self.read_bytes(n - out.size)
      if data
        out = out + data 
        tries = 0
      else
        tries = tries + 1
      end
      raise "get_n_bytes is stuck" if tries > 10
    end

    return out
  end

  # Return meta data for this serial device
  def meta
    {"BOARDID" => self.identity}
  end
end
