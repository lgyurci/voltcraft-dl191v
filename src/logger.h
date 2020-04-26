#pragma once
#include "datastructs.h"
#include <fstream>

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
        friend ofstream &operator <<(ofstream &of,const logger &log);
};