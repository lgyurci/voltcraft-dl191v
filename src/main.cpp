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
        vdl191v.configure(16000,2,now,10,false);
    } catch (char *ex){
        cout << ex << endl;
    }
}