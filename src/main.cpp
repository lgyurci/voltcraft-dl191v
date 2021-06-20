#include <iostream>
#include "voltcraft.h" //this handles the device
#include "logger.h" //this writes the downloaded data to file
#include <ctime>
#include <string.h> //for strcmp
#include <fstream>
#include <sstream> //to make converting string to int/float easy
#define vid 4292 //IMPORTANT: VENDOR ID If you have a slightly different device, and you'd like to try this software with it, change this. Find this value using lsusb
#define pid 60001 //PRODUCT ID like the vendor id
//Lázár György, 2020
using namespace std;

string argmis(string opt){ //Missing argument error message
  return (string) "Error: missing argument for option '" + opt + "'";
}

string argbad(string opt){ //Bad argument error message
  return (string) "Error: bad argument for option '" + opt + "'";
}

bool isNum(string pn){ //Can a given string be interpreted as a number? Old fashioned way
  for (int i = 0; i < pn.length(); i++){
    if (pn[i] < 48 || pn[i] > 57){
      return false;
    }
  }
  return true;
}

void validateConf(int datacount, int freq, std::tm *time, int ledmode, bool force){ //This checks the configuration for errors
    if (datacount > 32000 || datacount < 500){
        string ex;
        if (force) ex = "Datacount out of range. Range: [500,32000], got: " + to_string(datacount) + " (could not force)"; //max 32000 datapoints can be recorded by the dl-191v
        else ex = "Datacount out of range. Range: [500,32000], got: " + to_string(datacount);
        throw ex;
    }
    if (freq < 0){ //Negative numbers are not accepted, even if the force option is given
        string ex;
        if (force) ex = "Measure interval out of range. Range: [0,...], got: " + to_string(freq) + " (could not force)";
        else ex = "Measuring interval: bad value, got: " + to_string(freq);
        throw ex;
    }
    if (freq != 0 && freq != 2 && freq != 5 && freq != 10 && freq != 30 && freq != 60 && freq != 300 && freq != 600 && freq != 1800 && freq != 3600 && freq != 7200 && freq != 10800 && freq != 21600 && freq != 43200 && freq != 86400 && !force){
        string ex; //Yeah, this is pretty ugly. These values definitely work, others may or may not.
        ex = "Measuring interval: bad value, got: " + to_string(freq) + " - use -f to override, and if you like living on the edge";
        throw ex;
    }
    if (ledmode < 0){ //Led's blinking period. The original software only lets you set 10, 20, or 30, so I'm doing the same. Can be overridden with -f
        string ex;
        if (force) ex = "Led blink interval out of range. Range: [1,...], got: " + to_string(ledmode) + " (could not force)";
        else ex = "Led blink interval: bad value, got: " + to_string(freq);
        throw ex;
    }
    if (ledmode != 10 && ledmode != 20 && ledmode != 30 && !force){
        string ex;
        ex = "Led blink interval: bad value, got: " + to_string(freq) + " - use -f to override, and if you like living on the edge";
        throw ex;
    }
}

int main(int argc, char *argv[]){

        string lend = "\n";
        string help =  //help menu
        lend +
        (string) "Usage: vdl191v COMMAND [OPTIONS]\n" + 
        lend + 
        (string) "A small program for managing the Voltcraft DL-191V voltage meter and data logger\n" +
        lend +
        (string) "Available commands:\n" +
        (string) "setup - Sets up the datalogger with the given settings. Avaiable options: -c,-p,-l,-i,-f\n" +
        (string) "download - Downloads data from the device. Avaiable options: -s,-o,--no-header,--no-time-stamps\n" +
        (string) "help - Without arguments: display this message. With argument: display help for the given command\n" +
        lend +
        (string) "See 'vdl191v help COMMAND' for help with command options" +
        lend +
        (string)"This program was written by Lázár György, 2020, and it comes with absolutely no warranty. I'm not responsible for any damage done to any hardware.";
        if (argc == 1){
          cout << help << endl;
          return 1;
        } else {
          if (strcmp(argv[1],"download") == 0){ //if the user wants to download the data
            bool standard = false;
            bool header = true;
            bool timestamps = true;
            string output = "data.dsv"; 
            for (int i = 2; i < argc; i++){ //interpreting the arguments
              if (strcmp(argv[i],"-s") == 0){
                standard = true;
              } else if (strcmp(argv[i],"-o") == 0){
                if (argc > i+1){
                  if (argv[i+1][0] != '-'){
                    output = argv[++i];
                  } else {
                    cerr << "Error: missing or bad argument for option '-o' (filename cannot start with '-')" << endl;
                    return 2;
                  }
                } else {
                  cerr << argmis("-o") << endl;
                  return 2;
                }
              } else if (strcmp(argv[i],"--no-header") == 0){
                header = false;
              } else if (strcmp(argv[i],"--no-time-stamps") == 0) {
                timestamps = false;
              } else {
                cerr << "Error: option '" << argv[i] << "' is unknown for command 'download'" << endl;
                return 2;
              }
            }
            voltcraft vdl191v(vid,pid); //if everything was alright
            confdata livec;
            unsigned short int *data;
            try {
              vdl191v.download(&data,livec); //try to download the data from the device
            } catch(string ex){
              cerr << ex << endl;
              return 3;
            }
            logger lg(livec,data,timestamps,header,standard);
            ofstream f;
            f.open(output);
            f << lg; //save the downloaded data to file
            f.close();
            return 0;
          } else if (strcmp(argv[1],"setup") == 0){ //setup
            if (argc == 2) {
              cout << "Warning: You did not specify any setup parameter. By continuing, the device will receive the default config. Are you sure? ";
              string inp;
              cin >> inp;
              if (inp != "y") return 0;
            }
            bool force = false;
            int dco = 32000; //defaults
            int inte = 2;
            time_t t = time(0);
            tm* now = localtime(&t);
            int ledmode = 10;
            bool instant = false;
            for (int i = 2; i < argc; i++){ // Reading arguments and interpreting them
              if (strcmp(argv[i],"-c") == 0){
                if (i+1 < argc){
                  if (isNum(argv[i+1])){
                    stringstream s(argv[++i]);
                    s >> dco;
                  } else {
                    cerr << argbad("-c") << endl;
                    return 2;
                  }
                } else {
                  cerr << argmis("-c") << endl;
                  return 2;
                }
              } else if (strcmp(argv[i],"-p") == 0){
                if (i+1 < argc){
                  if (isNum(argv[i+1])){
                    stringstream s(argv[++i]);
                    s >> inte;
                  } else {
                    cerr << argbad("-p") << endl;
                    return 2;
                  }
                } else {
                  cerr << argmis("-p") << endl;
                  return 2;
                }
              } else if (strcmp(argv[i],"-i") == 0){
                instant = true;
              } else if (strcmp(argv[i],"-f") == 0){
                force = true;
              } else if (strcmp(argv[i],"-l") == 0){
                if (i+1 < argc){
                  if (isNum(argv[i+1])){
                    stringstream s(argv[++i]);
                    s >> ledmode;
                  } else {
                    cerr << argbad("-l") << endl;
                    return 2;
                  }
                } else {
                  cerr << argmis("-l") << endl;
                  return 2;
                }
              } else {
                cerr << "Error: option '" << argv[i] << "' is unknown for command 'setup'" << endl;
                return 2;
              }
            }
            try{
              validateConf(dco,inte,now,ledmode,force);
            } catch(string ex){
              cerr << ex << endl;
              return 2;
            }
            voltcraft vdl191v(vid,pid);
            try{
              vdl191v.configure(dco,inte,now,ledmode,instant);
            } catch (string ex){
              cerr << ex << endl;
              return 3;
            }
            return 0;
          } else if (strcmp(argv[1],"help") == 0 || strcmp(argv[1],"-h") == 0 || strcmp(argv[1],"--help") == 0){ //help menu
            if (argc == 2){
              cout << help << endl;
              return 0;
            } else if (strcmp(argv[2],"download") == 0){
              string dhelp = lend + 
              "Usage: vdl191v download [OPTIONS]\n" + 
              lend +
              "Downloads the recorded data from the device.\n" +
              "Note: This will stop the measurement.\n" +
              lend +
              "Options:\n" +
              "   -s                Print output to stdout as well\n" +
              "   -o                Specify output file. Default: data.dsv\n" +
              "   --no-header       Disables header in output file (first line)\n" +
              "   --no-timestamps   Disables the time row completely";
              cout << dhelp << endl;
              return 0;
            } else if (strcmp(argv[2],"setup") == 0){
              string shelp = lend + 
              "Usage: vdl191v setup [OPTIONS]\n" + 
              lend +
              "Sets up device with the given config. It is possible, though not recommended to run without parameters.\n" +
              "After setup, the device will consume power until the measurement starts and ends. Set it up only before measurements.\n" +
              lend +
              "Options:\n" +
              "   -p                Set the interval of measurement in seconds. Default: 2. Correct values are:\n" +
              "                     0 (400Hz),2,5,10,30,60,300 (5min), 600 (10min), 1800 (30min), 3600 (1h),\n" +
              "                     7200 (2h), 10800 (3h), 21600 (6h), 43200 (12h), 86400 (24h). Use -f to override these.\n" +
              "                     Warning: Overriding these values may prevent any measurement from occuring, or it may\n" +
              "                     mess up timings without notice. Use this at your own risk, and always test it first.\n" +
              lend +
              "   -c                Set the maximum data to be recorded. Default: 32000. Correct interval: [500,32000]\n" +
              "                     Note: Reaching this number won't power off the device, it will just cause it to stop recording.\n" +
              "                     Therefore, changing this value from 32000 is not recommended.\n" +
              lend +
              "   -l                Sets the green led's blinking interval in seconds. Default: 10. Correct values: 10,20,30.\n" +
              "                     Use -f to override, and set any value (again, at your own risk)\n" +
              lend +
              "   -i                Instant - Start the measurement instantly after configuring the device. (Default: button press)\n" + 
              lend +
              "   -f                Force - Force bad values set by -l or -p. These may, or may not work.\n";
              cout << shelp << endl;
              return 0;
            } else if (strcmp(argv[2],"help") == 0){
              cout << "This is getting a bit recursive, don't you think?" << endl; //small easter egg
              return 420;
            } else {
              cerr << "'" << argv[2] << "' is not a vdl191v command, so there is no help for it here." << endl;
            }
          } else {
            cerr << argv[1] << " is not a valid command. See 'vdl191v help'." << endl;
            return 1;
          }
        }
}
