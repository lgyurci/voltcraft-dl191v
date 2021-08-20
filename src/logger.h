#pragma once
#include "datastructs.h"
#include <fstream>
#include <iostream>
#include <iomanip> // It is neccesary to print the dates, like 4 as 04
//Lázár György, 2020

using namespace std;

class logger{
    private:
        confdata *cfg;
        unsigned short int *data;
        int l;
        bool ts;
        bool hr;
    public:
        logger(confdata &config, unsigned short int *dat,bool timestamps, bool header);
        ~logger();
        friend ofstream &operator <<(ofstream &of,const logger &log); //This overloaded operator writes the data to the file.
	friend ostream &operator <<(ostream &of,const logger &log); 
};
