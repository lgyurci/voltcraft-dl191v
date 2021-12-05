# Management software for the Voltcraft DL-191V data logger
This is a small and open source software for Linux, to handle the Voltcraft DL-191V voltage meter.

This project definitely needs improvements, however, it's completely functional. If you have a similar logger and you'd like to help, I would be glad to hear about you.

## Dependencies
- libusb 1.0 (dev)
- g++

## Installation 
```javascript codeblock1
./configure && make
```
Optionally, you may install the program.
```javascript codeblock2
make install
```

## Usage: vdl191v COMMAND [OPTIONS]

Please be aware, that running this program might (probably will) require higher permissions, than those that a simple mortal user usually posesses. Make sure these are granted (so run it as root, or give the neccesary permissions).

## Available commands

setup - Sets up the datalogger with the given settings. Avaiable options: -c,-p,-l,-i,-f

download - Downloads data from the device. Available options: -s,-o,--no-header,--no-time-stamps

help - Without arguments: display this message. With argument: display help for the given command

### setup

Sets up device with the given config. It is possible, though not recommended to run without parameters.

After setup, the device will consume power until the measurement starts and ends. Set it up only before measurements.

#### setup options
```javascript codeblock3

   -p                Set the interval of measurement in seconds. Default: 2. Correct values are:
   
    0 (400Hz), 2, 5, 10, 30, 60, 300 (5min), 600 (10min), 1800 (30min), 3600 (1h),
                     
    7200 (2h), 10800 (3h), 21600 (6h), 43200 (12h), 86400 (24h). Use -f to override these.
                     
    Warning: Overriding these values may prevent any measurement from occuring, or it may
                     
    mess up timings without notice. Use this at your own risk, and always test it first.
                     

   -c                Set the maximum data to be recorded. Default: 32000. Correct interval: [500,32000]
   
    Note: Reaching this number won't power off the device, it will just cause it to stop recording.
                     
    Therefore, changing this value from 32000 is not recommended.
                     

   -l                Sets the green led's blinking interval in seconds. Default: 10. Correct values: 10,20,30.
   
    Use -f to override, and set any value (again, at your own risk)
                     

   -i                Instant - Start the measurement instantly after configuring the device. (Default: button press)
   

   -f                Force - Force bad values set by -l or -p. These may, or may not work.
```


### download

Downloads the recorded data from the device. By default, it prints it on the standard output.

Note: This will stop the measurement. This operation can take up to 10s, depending on how much data has to be downloaded. After 10s, you may forcibly close the application. After exiting this way, unplug the datalogger, and plug it back in to reset connection. This is very rarely neccesary. Please submit an issue if this happens, including your settings, which you have used the datalogger with.

#### download options
```javascript codeblock4

   -o                Redirect output to the specified file instead.
   
   --no-header       Disables header in output file (first line)
   
   --no-timestamps   Disables the time row completely
```
## Examples
- You'd like to setup a measurement with 2s sampling period:
```javascript codeblock5
vdl191v setup -p 2
```
- You'd like to download the measurement data into the `data.dsv` file:
```javascript codeblock5
vdl191v download -o data.dsv
```
## Limitations
- This program can only handle one device at a given time
- You cannot set alarms (yet?)
## Copying

This software is as free as it gets, licensed under the most permissive (sensible) open source license - the unlicense. In summary, you may use, sell, and modify the source code in any way you'd like. For more information, please read the LICENSE file.

Also please note, that while my code (and everyone's who contirbuted, but at the time of writing there aren't any contributors :( ) might come with a permissive license, the used dependencies (libusb) might not. Therefore, you may sell my modified code, but you probably can't do the same with the compiled binary (but idk, I'm far from a lawyer).

One more thing, which is more like a personal request. If you find my code useful in any way, please hit me up, I would be glad to hear that this is useful for someone else in the world.

This program was written by Lázár György, 2020, and it comes with absolutely no warranty. I'm not responsible for any damage done to any hardware.
