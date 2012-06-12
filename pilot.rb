require 'RbSerialDevice'
require 'serial_device_accessors'

# Convenience class for communicating with pilot laser control
# supports a handful of common method
class Pilot < Base
  
  PIEZO_MIN = -13
  PIEZO_MAX = 13
  
  sd_reader "*idn?", :identity
  sd_writer ":System:echo %s", :echo

  sd_reader ":Piezo:Offset?", :piezo_offset
#  sd_writer ":Piezo:Offset %0.3f", :piezo_offset
  def piezo_offset=(val)
    unless val < -13.5 or val > 13.5
      self.send_message(":Piezo:Offset #{val}")
    end
  end  
  sd_reader ":Piezo:Frequency?", :piezo_frequency
  sd_writer ":Piezo:Frequency 0.2f Hz", :piezo_frequency

  sd_reader ":Piezo:Frequency:Generator?", :piezo_waveform
  sd_writer ":Piezo:Frequency:Generator %0.3s", :piezo_waveform # OFF|0, SIN|1, TRI|2
  
  sd_reader ":Piezo:Frequency:Amplitude?", :piezo_amplitude
  sd_writer ":Piezo:Frequency:Amplitude %0.3f", :piezo_amplitude

  sd_reader ":Piezo:Voltage?", :piezo_voltage
  
  sd_reader ":Laser:Current?", :laser_current
  sd_writer ":Laser:Current %0.3f", :laser_current

  sd_reader ":Laser:Status?", :laser_status

  sd_reader ":TEC:Temperature?", :laser_temperature
  
  sd_reader ":CCoupling:Enable?", :cc_enable
  sd_reader ":CCoupling:Gain?", :cc_gain
  sd_reader ":CCoupling:Prescale?", :cc_prescale
  sd_reader ":CCoupling:Direction?", :cc_direction

  # Limit current to 1.5 - 3.0 amps (negatitve voltage bias)
  def validate_current(current)
    (current < 0 and current > -3.0)
  end
  
  # Run the piezo from current -> min -> max -> 0
  def init_piezo
    
    piezo_amplitude = 0
    piezo_step = 0.5
    current_offset = piezo_offset.to_f
    
    puts "Piezo Rampup Routine..."
    
    current_offset.step(PIEZO_MIN, -piezo_step){|v|
      puts "PiezoOffset = #{v}"
      self.piezo_offset = v
    }
    
    PIEZO_MIN.step(PIEZO_MAX, piezo_step) {|v|
      puts "PiezoOffset = #{v}"
      self.piezo_offset = v
    }
    
    PIEZO_MAX.step(0, -piezo_step) {|v|
      puts "PiezoOffset = #{v}"
      self.piezo_offset = v
    }
    
    true
  end

  # Laser controller identity
  LASERID         = "LASERID"
  
  # Laser injection current
  LASER_CURRENT   = "LASERCUR"

  # Piezo waveform
  PIEZO_WAVEFORM  = "PIEZOWAV"

  # Piezo waveform frequency
  PIEZO_FREQUENCY = "PIEZOFRE"

  # Piezo waveform amplitude
  PIEZO_AMPLITUDE = "PIEZOAMP"

  # Piezo DC offset
  PIEZO_OFFSET    = "PIEZOOFF"

  # Actual piezo voltage (unreliable)
  PIEZO_VOLTAGE   = "PIEZOVOL"

  # Laser Diode temperature
  LASER_TEMP      = "LASERTEM"

  # Current Coupling Status
  CC_ENABLE       = "CCENABLE"

  # Return the meta data for the pilot laser controller as a hash
  def meta
    meta = {}
    set_meta(meta)
    return meta
  end

  # Takes an object which responds to []= and 
  # sets meta data for that object.
  def set_meta(data)
    
    data[LASERID] = self.identity.chomp
    data[LASER_CURRENT] = self.laser_current.chomp
    data[PIEZO_WAVEFORM] = self.piezo_waveform.chomp
    data[PIEZO_FREQUENCY] = self.piezo_frequency.chomp
    data[PIEZO_AMPLITUDE] = self.piezo_amplitude.chomp
    data[PIEZO_OFFSET] = self.piezo_offset.chomp
    data[PIEZO_VOLTAGE] = self.piezo_voltage.chomp
    data[LASER_TEMP] = self.laser_temperature.chomp
    data[CC_ENABLE] = self.cc_enable.chomp

  end
  
end
