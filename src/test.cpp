 #include <iostream>
 #include <string.h>
 #include <sstream>
 
 using namespace std;
 
 int main(){
     string st = "a42a";
     stringstream s (st);
     int a;
     s >> a;
     cout << a << endl;

 }