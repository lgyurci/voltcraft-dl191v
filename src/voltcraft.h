#pragma once
#include <libusb-1.0/libusb.h>
#include "datastructs.h"
#include <ctime>
#include <iostream> //because of string
//Lázár györgy, 2020

class voltcraft{
    private:
        int v,p;
        const int usb_timeout = 6000;
    public:
        voltcraft(int v /*Vendor id*/, int p /*Product id*/);
        int configure (int datacount, int freq, std::tm *time, int ledmode, bool instant) const; //this configures the device
        int download(unsigned short int **results, confdata &cfdata) const; //this downloads the data
};
