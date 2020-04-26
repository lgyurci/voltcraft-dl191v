 #include <iostream>
 #include <string.h>
 #include <sstream>
 #include <fstream>
 
 using namespace std;
 
 int main(){
     ofstream f;
     f.open("data.dsv");
     f << "asd" << endl;
     f.close();
 }