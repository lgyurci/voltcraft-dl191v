 #include <iostream>
 #include "datastructs.h"
 
 using namespace std;
 
 int main(){   
	int dread=0;
    confdata conf;
    b3header confsignal;
    b2header downloadable;
    confsignal.id = 0;
    confsignal.d1 = 16;
    confsignal.d2 = 1;


    confsignal.id=0;
    confsignal.d1=0;
    confsignal.d2=1;

	downloadable.dat = 16000;

    int requestcount = downloadable.dat/4096;
    int blokkremain = downloadable.dat % 4096;
    int rems = blokkremain/64;
    int datain = 0;
    if (blokkremain % 64 != 0 && blokkremain != 0){
        rems++;
    }
    int total64;
    if (downloadable.dat%64 != 0){
        total64 = downloadable.dat/64 + 1;
    } else {
        total64 = downloadable.dat;
    }
    short int *ddata = new short int [total64*32];
    for (int i = 0; i <= requestcount; i++){
        confsignal.id=0;
        confsignal.d1=i;
        if (i == requestcount){
            confsignal.d2 = rems;
            int db = blokkremain/512;
            int last = rems-db;
            cout << (int)confsignal.id << " " << (int)confsignal.d1 << " " << (int)confsignal.d2 << endl;
            for (int j = 0; j < db; j++){
                dread = 0;
                datain += dread;
            }
            if (last != 0){
                dread = 0;
                datain += dread;
            }
        } else {
            confsignal.d2 = 64;
            cout << (int)confsignal.id << " " << (int)confsignal.d1 << " " << (int)confsignal.d2 << endl;
            for (int j = 0; j < 8; j++){
                dread = 0;
                datain += dread;
            }
        }
    }
 }