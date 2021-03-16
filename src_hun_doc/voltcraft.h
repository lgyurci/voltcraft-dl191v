#pragma once
#include <libusb-1.0/libusb.h>
#include "datastructs.h"
#include <ctime>
#include <iostream> //string miatt kell
//Lázár györgy, 2020

class voltcraft{
    private:
        int v,p;
        const int usb_timeout = 6000;
    public:
        voltcraft(int v /*Vendor id*/, int p /*Product id*/);
        ~voltcraft(); //nincs rá szükség, a cpp fájlban van hozzá több indoklás hogy miért
        int configure (int datacount, int freq, std::tm *time, int ledmode, bool instant) const; //ezzel konfiguráljuk az eszközt
        int download(unsigned short int **results, confdata &cfdata) const; //ezzel töltjük le róla az adatokat
};
