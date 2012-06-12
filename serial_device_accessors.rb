
# Functional base class for objects wishing to extend
# SerialDevice.  It adds accessor method to facilitate
# definition of SerialDevice communication.
#
# TODO: add validation to accessor methods.
class Base < SerialDevice
  
  #
  # Class SerialDev Accessor Methods 
  #
  #  This construct provides class methods sd_reader,sd_writer which 
  #  dynamically write instance methods for newly created instances.
  #  
  class << self
    
    # Class method to add a reader to the serial device.
    # dev_string is the message sent to the device.
    # id is the name of the defined ruby method.
    #
    # ex.
    #    sd_reader ":LASER:CURRENT?", :current
    # creates the method 
    #    current
    def sd_reader(dev_string, id)
      module_eval <<-"eod"
        def #{id.id2name}
          self.send_message("#{dev_string}")
        end
      eod
    end
  
    # define a writer method i.e. property=
    #
    # dev_string is a format string.
    # id is the name of the method.
    #
    # ex. 
    #    sd_writer ":LASER:CURRENT %d", :current
    # creates the method 
    #    current=(value)
    def sd_writer(dev_string, id, options = {})

      val_proc = false
      if options[:validate] # and self.public_method_defined? options[:validate]
        val_proc = true
      end 
      
      # Define an instance method which executes a validation routine
      # on the passed value and then executes the instruction
      module_eval <<-"eod"
        def #{id.id2name}=(value)
          
          # Perform validations 
          valid = true
          if #{val_proc}
            valid = self.send("#{options[:validate]}(" + value.to_s + ")")
          end
          
          # Execute 
          if valid
            self.send_message("#{dev_string}" % value)
          else
            false
          end
          
        end
      eod
    end
    
  end
    
end
