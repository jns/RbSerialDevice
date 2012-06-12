require 'm6812'

m = M6812.new(:device => "/dev/ttyS0", :baud => 57600, :hw_flow => false)
puts m.identity
