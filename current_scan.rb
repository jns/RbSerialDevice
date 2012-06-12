#!/usr/bin/ruby

require 'RbSerialDevice'
require 'pilot.rb'

pilot = SerialDevice.new(:device => "/dev/ttyUSB0", :baud => 57600)
board = SerialDevice.new(:device => "/dev/ttyS0", :baud => 57600)

puts pilot.send_message("*IDN?")
puts board.send_message("IDN")


istart = -1.9
istop  = -2.4
istep  = -0.002


# The current incarnation of the data is 
# each byte is separated by a white space
def save(arr, fname)
  f = File.open(fname, 'w') 
  begin
   arr.shift
   x = arr.shift
   arr.shift
   y = arr.shift
   f.puts "#{x} #{y}"
  end until arr.empty?
  f.close
end

# Guarantee to read n bytes
def read_bytes(sd, n)

  out = []

  while (n > out.size)  
    data = sd.read_bytes(n - out.size)
    break unless data
    out = out + data
  end

  return out
end

# step through current
istart.step(istop, istep) do |i|
  puts "Current #{i}"
  pilot.send_message(":Laser:Current #{i}")
  board.write("SAMPLE")
  a = read_bytes(board, 1024)
  save(a, "data3/scan_#{i.abs}.txt")

end