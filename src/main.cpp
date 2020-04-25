#include <iostream>
#include "voltcraft.h"
#include <ctime>
#include <string.h>
#include <fstream>
#include <iomanip>
#define vid 4292
#define pid 60001

using namespace std;

string argmis(string opt){
  return (string) "Error: missing argument for option '" + opt + "'";
}

int main(int argc, char *argv[]){
/*        voltcraft vdl191v(vid,pid);
        std::time_t t = std::time(0);
        std::tm* now = std::localtime(&t);
      //  vdl191v.configure(32000,2,now,10,false);
        confdata livec;
        unsigned short int *data;
        int cunt = vdl191v.download(&data,livec);
        cout << cunt << " data downloaded:" << endl;
        for (int i = 0; i < cunt; i++){
            cout << data[i] << " ";
        }
        cout << endl;
        delete[] data;*/
        string lend = "\n";
        string help = 
        lend +
        (string) "Usage: vdl191v COMMAND [OPTIONS]\n" + 
        lend + 
        (string) "A small program for managing the Voltcraft DL-191V voltage meter and data logger\n" +
        lend +
        (string) "Avaiable commands:\n" +
        (string) "setup - Sets up the datalogger with the given settings. Avaiable options: -c,-p,-t,-l,-i,-f\n" +
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
          if (strcmp(argv[1],"download") == 0){
            bool standard = false;
            bool header = true;
            bool timestamps = true;
            string output = "data.dsv"; 
            for (int i = 2; i < argc; i++){
              if (strcmp(argv[i],"-s") == 0){
                standard = true;
              } else if (strcmp(argv[i],"-o") == 0){
                if (argc > i+1){
                  if (argv[i+1][0] != '-'){
                    output = argv[++i];
                  } else {
                    cout << "Error: missing or bad argument for option '-o' (filename cannot start with '-')" << endl;
                    return 2;
                  }
                } else {
                  cout << argmis("-o") << endl;
                  return 2;
                }
              } else if (strcmp(argv[i],"--no-header") == 0){
                header = false;
              } else if (strcmp(argv[i],"--no-time-stamps") == 0) {
                timestamps = false;
              } else {
                cout << "Error: option '" << argv[i] << "' is unknown for command 'download'" << endl;
                return 2;
              }
            }
            voltcraft vdl191v(vid,pid);
            confdata livec;
            unsigned short int *data;
            int count;
            try {
              count = vdl191v.download(&data,livec);
            } catch(string ex){
              cout << ex;
              return 3;
            }
            ofstream f;
            f.open(output);
            double inte;
            if (livec.freq == 0) inte = 0.0025; else inte = livec.freq;
            if (header){
              f << "Measure_start:'" << livec.year << "-" << setfill('0') << setw(2) << (int) livec.month << "-" << setfill('0') << setw(2) << (int) livec.day << " "<< setfill('0') << setw(2)  << (int) livec.hour << ":" << setfill('0') << setw(2) << (int) livec.min << ":" << setfill('0') << setw(2) << (int) livec.sec << "'" 
              ",Interval(s):"<< inte << ",Recorded_data:" << count << endl;
              if (standard) {
                cout << "Measure_start:'" << livec.year << "-" << setfill('0') << setw(2) << (int) livec.month << "-" << setfill('0') << setw(2) << (int) livec.day << " "<< setfill('0') << setw(2)  << (int) livec.hour << ":" << setfill('0') << setw(2) << (int) livec.min << ":" << setfill('0') << setw(2) << (int) livec.sec << "'" 
              ",Interval(s):"<< inte << ",Recorded_data:" << count << endl;
              }
            }
            if (timestamps) {
              f << "Time(s)" << "  " << "Voltage(mV)" << endl;
              if (standard) {
                cout << "Time(s)" << "  " << "Voltage(mV)" << endl;
              }
            } else {
              f << "Voltage(mV)" << endl;
              if (standard) {
                cout << "Voltage(mV)" << endl;
              }
            }
            if(timestamps){
              for (int i = 0; i < count; i++){
                f << inte*i << "  " << data[i] << endl;
                if (standard) {
                  cout << inte*i << "  " << data[i] << endl;
                }
              }
            } else {
              for (int i = 0; i < count; i++){
                f << "  " << data[i] << endl;
                if (standard) {
                  cout << "  " << data[i] << endl;
                }
              }
            }
            delete[] data;
            f.close();
            return 0;
          } else if (strcmp(argv[1],"setup") == 0){
            if (argc == 2) {
              cout << "Warning: You did not specify any setup parameter. By continuing, the device will receive the default config. Are you sure? (y) ";
              string inp;
              cin >> inp;
              if (inp != "y") return 0;
            }
            bool force = false;
            int dco = 32000;
            int inte = 2;
            time_t t = time(0);
            tm* now = localtime(&t);
            int ledmode = 10;
            bool instant = false;
            for (int i = 2; i < argc; i++){
              if (strcmp(argv[i],"-c") == 0){
                if (i+1 < argc){
                  stringstream s(argv[++i]);
                  s >> dco;
                } else {
                  cout << argmis("-c") << endl;
                  return 1;
                }
              } else if (strcmp(argv[i],"-p") == 0){
                if (i+1 < argc){
                  stringstream s(argv[++i]);
                  s >> inte;
                } else {
                  cout << argmis("-p") << endl;
                  return 1;
                }
              } else if (strcmp(argv[i],"-i") == 0){
                instant = true;
              }
              
            }
            voltcraft vdl191v(vid,pid);
          } else {
            cout << argv[1] << " is not a valid command. See 'vdl191v help'." << endl;
            return 1;
          }
        }
}