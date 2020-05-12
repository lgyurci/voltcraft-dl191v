#include "logger.h"
#include "datastructs.h"
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

logger::logger(confdata &config, unsigned short int *dat,bool timestamps, bool header, bool standard){
    cfg = &config;
    data = dat;
    ts = timestamps;
    hr = header;
    sd = standard;
}
logger::~logger(){ // Ezt megtehetjük, mert úgyis csak new-val létrehozott tömböt kap
    delete[] data;
}
ofstream &operator <<(ofstream &of, const logger &log){
    double inte; //intervallum
    if (log.cfg->freq == 0) inte = 0.0025; else inte = log.cfg->freq; //0 -> 400Hz-t jelent
    if (log.hr){ //ez igazából csak mechanikus szenvedés, és egyértelmű, szóval nem dokumentálom
        of << "#Measure_start:'" << log.cfg->year << "-" << setfill('0') << setw(2) << (int) log.cfg->month << "-" << setfill('0') << setw(2) << (int) log.cfg->day << " "<< setfill('0') << setw(2)  << (int) log.cfg->hour << ":" << setfill('0') << setw(2) << (int) log.cfg->min << ":" << setfill('0') << setw(2) << (int) log.cfg->sec << "'" 
        ",Interval(s):"<< inte << ",Recorded_data:" << log.cfg->idk << endl;
        if (log.sd) {
        cout << "#Measure_start:'" << log.cfg->year << "-" << setfill('0') << setw(2) << (int) log.cfg->month << "-" << setfill('0') << setw(2) << (int) log.cfg->day << " "<< setfill('0') << setw(2)  << (int) log.cfg->hour << ":" << setfill('0') << setw(2) << (int) log.cfg->min << ":" << setfill('0') << setw(2) << (int) log.cfg->sec << "'" 
        ",Interval(s):"<< inte << ",Recorded_data:" << log.cfg->idk << endl;
        }
    }
    if (log.ts) {
        of << "Time(s)" << (char)9 << "Voltage(mV)" << endl;
        if (log.sd) {
            cout << "Time(s)" << (char)9 << "Voltage(mV)" << endl;
        }
    } else {
        of << "Voltage(mV)" << endl;
        if (log.sd) {
            cout << "Voltage(mV)" << endl;
        }
    }
    if(log.ts){
        for (int i = 0; i < log.cfg->idk; i++){
            of << inte*i << (char)9 << log.data[i] << endl;
            if (log.sd) {
              cout << inte*i << (char)9 << log.data[i] << endl;
            }
        }
    } else {
        for (int i = 0; i < log.cfg->idk; i++){
            of << log.data[i] << endl;
            if (log.sd) {
                cout << log.data[i] << endl;
            }
        }
    }
    return of;
}