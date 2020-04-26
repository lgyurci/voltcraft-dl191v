#pragma once
#include "datastructs.h"
#include <fstream>
#include <iostream>
#include <iomanip> // dátum kiírásához kell, hogy mondjuk 4 helyett 04 legyen

using namespace std;

class logger{
    private:
        confdata *cfg;
        unsigned short int *data;
        int l;
        bool ts;
        bool hr;
        bool sd;
    public:
        logger(confdata &config, unsigned short int *dat,bool timestamps, bool header, bool standard);
        ~logger();
        friend ofstream &operator <<(ofstream &of,const logger &log); //ez írja ki fájlba, igazából ennek az osztálynak ez az egyetlen funkciója, kicsit bele van erőltetve hogy tudjak operatort overloadolni, mert követelmény
};