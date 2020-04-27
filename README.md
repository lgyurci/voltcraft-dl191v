# vdl191v
A small (but probably the only) implementation for Voltcraft DL-191V on linux

Installation: ./configure && make && make install . It's possible to run the program without installing it (actually, there is no strong reason to do the final step...).

**Usage:** vdl191v COMMAND [OPTIONS]

Please be aware, that running this program might (probably will) require higher permissions, than those that a simple mortal user usually posesses. Make sure these are granted (so run it as root, or give the neccesary permissions).

**Available commands:**

setup - Sets up the datalogger with the given settings. Avaiable options: -c,-p,-l,-i,-f

download - Downloads data from the device. Avaiable options: -s,-o,--no-header,--no-time-stamps

help - Without arguments: display this message. With argument: display help for the given command

**setup:**

Sets up device with the given config. It is possible, though not recommended to run without parameters.

After setup, the device will consume power until the measurement starts and ends. Set it up only before measurements.

**Options:**
   -p                Set the interval of measurement in seconds. Default: 2. Correct values are:
   
    0 (400Hz), 2, 5, 10, 30, 60, 300 (5min), 600 (10min), 1800 (30min), 3600 (1h),
                     
    7200 (2h), 10800 (3h), 21600 (6h), 43200 (12h), 86400 (24h). Use -f to override these.
                     
    Warning: Overriding these values may prevent any measurement from occuring, or it may
                     
    mess up timings without notice. Use this at your own risk, and always test it first.
                     

   -c                Set the maximum data to be recorded. Default: 32000. Correct interval: [50,32000]
   
    Note: Reaching this number won't power off the device, it will just cause it to stop recording.
                     
    Therefore, changing this value from 32000 is not recommended.
                     

   -l                Sets the green led's blinking interval in seconds. Default: 10. Correct values: 10,20,30.
   
    Use -f to override, and set any value (again, at your own risk)
                     

   -i                Instant - Start the measurement instantly after configuring the device. (Default: button press)
   

   -f                Force - Force bad values set by -l or -p. These may, or may not work.


**download:**

Downloads the recorded data from the device.

Note: This will stop the measurement.


**Options:**

   -s                Print output to stdout as well
   
   -o                Specify output file. Default: data.dsv
   
   --no-header       Disables header in output file (first line)
   
   --no-timestamps   Disables the time row completely
   

This program was written by Lázár György, 2020, and it comes with absolutely no warranty. I'm not responsible for any damage done to any hardware.
