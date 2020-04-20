#include <iostream>
#include <libusb-1.0/libusb.h>
#include <thread>
#include <chrono>
#define dven 4292
#define dprod 60001

using namespace std;

struct confdata {
    const int startsignal = 206;
    int datacount = 16000;
    int idk = 0;
    int freq = 0;
    int year = 2020;
    int alarm_low1 = 1086324736;
    int alarm_high1 = 1106247680;
    char month = 7;
    char day = 6;
    char hour = 10;
    char min = 32;
    char sec = 42;
    char yes = 0;
    char Q = 10;
    char confname[16] = "Brutus";
    char start = 1;
    int alarm_low2 = 1086324736;
    int alarm_high2 = 1106247680;
    const int endsignal = 206;
};
struct b3header{
    char id;
    short int dat;
};

int main(){
    libusb_device **devs;
    libusb_device_handle *dev_handle;
    libusb_context *ctx = NULL;
    if(libusb_init(&ctx) < 0){
        cout << "error 10" << endl;
        return 1;
    }
 //   libusb_set_option(ctx,3);
    int cnt = libusb_get_device_list(ctx, &devs);
    libusb_device_descriptor desc[cnt];
    for (int i = 0; i < cnt; i++){
        libusb_get_device_descriptor(devs[i], &desc[i]);
       // cout << desc[i].idVendor << " " << desc[i].idProduct << endl;
        //TODO
    }
    dev_handle = libusb_open_device_with_vid_pid(ctx,dven,dprod);
    if (dev_handle == NULL){
        cout << "null" << endl;
        return 1;
    } else {
        cout << "not null" << endl;
    }
    if (libusb_kernel_driver_active(dev_handle,0) == 1){
        cout << "Detaching kernel driver" << endl;
        if (libusb_detach_kernel_driver(dev_handle,0) == 0){
            cout << "Detached." << endl;
        }
    }
    if (libusb_claim_interface(dev_handle,0) < 0){
        cout << "cannot claim interface" << endl;
    }
    cout << "trasfering control" << endl;
    libusb_control_transfer(dev_handle,64,0,65535,0,NULL,0,0);
    libusb_control_transfer(dev_handle,64,2,2,0,NULL,0,0);
    cout << "finish" << endl;
    int written=0;
    confdata conf;
    unsigned char confsignal[3];
    confsignal[0]=0;
    confsignal[1]=16;
    confsignal[2]=1;
    libusb_bulk_transfer(dev_handle,2,confsignal,3,&written,0);
    libusb_bulk_transfer(dev_handle,130,confsignal,3,&written,0);
    cout << "num data header received: " << (int) confsignal[0] << " " << (int) confsignal[1] << " " << (int) confsignal[2] << " " << endl;
    confdata liveconfig;
    libusb_bulk_transfer(dev_handle,130,(unsigned char*)&liveconfig,64,&written,0);

    //std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    libusb_release_interface(dev_handle,0);
    libusb_close(dev_handle);
    libusb_exit(ctx);
    libusb_init(&ctx);
    dev_handle = libusb_open_device_with_vid_pid(ctx,dven,dprod);
        if (libusb_kernel_driver_active(dev_handle,0) == 1){
        cout << "Detaching kernel driver" << endl;
        if (libusb_detach_kernel_driver(dev_handle,0) == 0){
            cout << "Detached." << endl;
        }
    }
    if (libusb_claim_interface(dev_handle,0) < 0){
        cout << "cannot claim interface" << endl;
    }
    libusb_control_transfer(dev_handle,64,0,65535,0,NULL,0,0);
    libusb_control_transfer(dev_handle,64,2,2,0,NULL,0,0);
    confsignal[0]=0;
    confsignal[1]=0;
    confsignal[2]=1;
    libusb_bulk_transfer(dev_handle,2,confsignal,3,&written,0);
    short int rdt[32];
    libusb_bulk_transfer(dev_handle,130,confsignal,3,&written,0);
//    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    libusb_bulk_transfer(dev_handle,130,confsignal,3,&written,0);
    cout << "header received: " << (int) confsignal[0] << " " << (int) confsignal[1] << " " << (int) confsignal[2] << " " << endl;
    libusb_bulk_transfer(dev_handle,130,(unsigned char*) &rdt,64,&written,0);
    cout << written << " bytes were read:" << endl;
    for (int i = 0; i < written/2; i++){
        cout << rdt[i] << " ";
    }
    cout << endl;
    libusb_control_transfer(dev_handle,64,2,4,0,NULL,0,0);
    libusb_release_interface(dev_handle,0);
    libusb_close(dev_handle);
    libusb_exit(ctx);
}