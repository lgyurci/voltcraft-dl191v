#pragma once
#include <libusb-1.0/libusb.h>
#include <ctime>
#include "datastructs.h"

class voltcraft{
    private:
        int v,p;
    public:
        voltcraft(int v /*Vendor id*/, int p /*Product id*/);
        ~voltcraft();
        int configure (int datacount, int freq, std::tm *time, int ledmode, bool instant = false);
        int download(unsigned short int **results, confdata &cfdata);
        static void validateConf(int datacount, int freq, std::tm *time, int ledmode, bool instant = false);
};