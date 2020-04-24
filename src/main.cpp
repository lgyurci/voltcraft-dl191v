#include <iostream>
#include "voltcraft.h"
#include <ctime>
#define vid 4292
#define pid 60001

using namespace std;

int main(){
    try{
        voltcraft vdl191v(vid,pid);
        std::time_t t = std::time(0);
        std::tm* now = std::localtime(&t);
     //   vdl191v.configure(8000,0,now,10,false);
        confdata livec;
        unsigned short int *data;
        int cunt = vdl191v.download(&data,livec);
        cout << cunt << " data downloaded:" << endl;
        for (int i = 0; i < cunt; i++){
            cout << data[i] << " ";
        }
        cout << endl;
        delete[] data;
    } catch (char *ex){
        cout << ex << endl;
    }
}