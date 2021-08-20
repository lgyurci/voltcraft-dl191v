//Lázár György, 2020

#include "logger.h"
#include "datastructs.h"
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

logger::logger(confdata &config, unsigned short int *dat,bool timestamps, bool header){
    cfg = &config;
    data = dat;
    ts = timestamps;
    hr = header;
}
logger::~logger(){ // We can do this, but only if we don't give it an array as a param NOT created by new
    delete[] data;
}

ostream &operator <<(ostream &of, const logger &log){
    double inte; //interval
    if (log.cfg->freq == 0) inte = 0.0025; else inte = log.cfg->freq; //0 equals 400Hz
    if (log.hr){ //this is just disgusting suffering, I won't comment on this
        of << "#Measure_start:'" << log.cfg->year << "-" << setfill('0') << setw(2) << (int) log.cfg->month << "-" << setfill('0') << setw(2) << (int) log.cfg->day << " "<< setfill('0') << setw(2)  << (int) log.cfg->hour << ":" << setfill('0') << setw(2) << (int) log.cfg->min << ":" << setfill('0') << setw(2) << (int) log.cfg->sec << "'" 
        ",Interval(s):"<< inte << ",Recorded_data:" << log.cfg->idk << endl;
    }
    if (log.ts) {
        of << "Time(s)" << (char)9 << "Voltage(mV)" << endl;
    } else {
        of << "Voltage(mV)" << endl;
    }
    if(log.ts){
        for (int i = 0; i < log.cfg->idk; i++){
            of << inte*i << (char)9 << log.data[i] << endl;
        }
    } else {
        for (int i = 0; i < log.cfg->idk; i++){
            of << log.data[i] << endl;
        }
    }
    return of;
}

ofstream &operator <<(ofstream &of, const logger &log){
    double inte; //interval
    if (log.cfg->freq == 0) inte = 0.0025; else inte = log.cfg->freq; //0 equals 400Hz
    if (log.hr){ //this is just disgusting suffering, I won't comment on this
        of << "#Measure_start:'" << log.cfg->year << "-" << setfill('0') << setw(2) << (int) log.cfg->month << "-" << setfill('0') << setw(2) << (int) log.cfg->day << " "<< setfill('0') << setw(2)  << (int) log.cfg->hour << ":" << setfill('0') << setw(2) << (int) log.cfg->min << ":" << setfill('0') << setw(2) << (int) log.cfg->sec << "'" 
        ",Interval(s):"<< inte << ",Recorded_data:" << log.cfg->idk << endl;
    }
    if (log.ts) {
        of << "Time(s)" << (char)9 << "Voltage(mV)" << endl;
    } else {
        of << "Voltage(mV)" << endl;
    }
    if(log.ts){
        for (int i = 0; i < log.cfg->idk; i++){
            of << inte*i << (char)9 << log.data[i] << endl;
        }
    } else {
        for (int i = 0; i < log.cfg->idk; i++){
            of << log.data[i] << endl;
        }
    }
    return of;
}
