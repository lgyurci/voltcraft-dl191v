#pragma once
#include <libusb-1.0/libusb.h>
#include <ctime>
#include "datastructs.h"
#include <string.h>
#include <iostream>

class voltcraft{
    private:
        int v,p;
    public:
        voltcraft(int v /*Vendor id*/, int p /*Product id*/);
        ~voltcraft();
        int configure (int datacount, int freq, std::tm *time, int ledmode, bool instant);
        int download(unsigned short int **results, confdata &cfdata);
};