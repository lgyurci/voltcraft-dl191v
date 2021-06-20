#include <libusb-1.0/libusb.h>
#include "voltcraft.h"
#include "datastructs.h"
#include <ctime>
#include <iostream>
//Lázár György, 2020

using namespace std;

voltcraft::voltcraft(int vid/*vendor id*/, int pid/*product id*/){
    v = vid;
    p = pid;
}

int voltcraft::configure (int datacount, int freq, std::tm *time, int ledmode, bool instant) const{ //return value igazából nem is kéne, mert megoldja az exception handling
    libusb_device_handle *dev_handle = NULL; //device
    libusb_context *ctx; //libusb context
    libusb_init(&ctx);
    libusb_device **devs; //device list
    int cnt = libusb_get_device_list(ctx, &devs);
    bool found = false;
     for (int i = 0; i < cnt && !found; i++){
        libusb_device_descriptor desc;
        libusb_get_device_descriptor(devs[i], &desc);
        if (desc.idVendor == v && desc.idProduct == p) found = true; //is the device plugged in?
    }
    libusb_free_device_list(devs, 1);
    if (!found){
        string ex = "Device not found. Is it connected?"; //we complain to the user if it is not
        libusb_exit(ctx);
        throw ex;
    }
    dev_handle = libusb_open_device_with_vid_pid(ctx,v,p); //try to open the connection
    if (dev_handle == NULL){
        string ex = "Device is found, but opening it failed. Do you have the permission to do so?"; //with 99% probability it is a permission error
        libusb_exit(ctx);
        throw ex;
    }
    if (libusb_kernel_driver_active(dev_handle,0) == 1){ //if the kernel tried to use some driver
        if (libusb_detach_kernel_driver(dev_handle,0) != 0){ //then it should be detached
            string ex = "Could not detach kernel driver assigned to the device. This may be a permission issue."; //If you get this error, idk what to do. Maybe a selinux issue?
            libusb_close(dev_handle);
            libusb_exit(ctx);
            throw ex;
        }
    }
    if (libusb_claim_interface(dev_handle,0) != 0){ //claim the interface
        string ex = "Claiming USB interface failed. Check if you have the correct permissions to execute this task."; //In theory, this shouldn't happen, like never
        libusb_close(dev_handle);
        libusb_exit(ctx);
        throw ex;
    }
    int ret1,ret2 = -1; //since I do not check the validity of the return value, these are not really neccesary. In the future, this might change
    ret1 = libusb_control_transfer(dev_handle,64,0,65535,0,NULL,0,usb_timeout); // libusb control transfer. This was done by reverse engineering the windows version
    ret2 = libusb_control_transfer(dev_handle,64,2,2,0,NULL,0,usb_timeout); // (Also, I don't fully understand what this does, but whatever, it works)

    //If the program made it here, we have a live communication with the device. Yay
    confdata cfg; //creating config data
    cfg.datacount = datacount;
    cfg.freq = freq; //it is more like a period than frequency, but it sounds better
    cfg.year = time->tm_year + 1900;
    cfg.month = time->tm_mon + 1;
    cfg.day = time->tm_mday;
    cfg.hour = time->tm_hour;
    cfg.min = time->tm_min;
    cfg.sec = time->tm_sec;
    unsigned char ledchar = (unsigned char) ledmode; //period of the led's blinking
    unsigned char frmode; // I don't actually know what is this, but for different sampling rates, this is different. If it is wrong, it won't work.
    if (freq <= 4){
        frmode = 0;
    }
    if (freq > 4 && freq <= 6){
        frmode = 32;
    }
    if (freq > 6 && freq <= 10){
        frmode = 64;
    }
    if (freq > 10){
        frmode = 96;
    }
    bool alarm = false; //future function, for setting the alarm. Not yet implemented
    unsigned char alarmchar;
    if (alarm){
        alarmchar = 128;
    } else {
        alarmchar = 0;
    }
    unsigned char Q = ledchar | frmode | alarmchar; //combining the numbers defined above, we get the neccesary char
    cfg.Q = Q;
/*   char confname[16];
    bool stre = false;
    for (int i = 0; i < 16; i++){
        if (name[i] == '\0'){
            stre = true;
        }
        if (stre){
            confname[i] = 0;
        } else {
            confname[i] = name[i];
        }
    }
    cfg.confname = confname;*/
/*    char confname[16] = "Brutus";
    cfg.confname = confname;*/ //Because I suck at programming, this is not working, so the device's name is always a hardcoded "Brutus"
    unsigned char start; // should the measurement start right after writing the config?
    if (freq == 0){ // At 400Hz these bits are different for some reason, no idea why
        if (instant){
            start = 18;
        } else {
            start = 17;
        }
    } else {
        if (instant){
            start = 2;
        } else {
            start = 1;
        }
    }
    cfg.start = start;




    int written=0; // bytes written to device
    b3header confsignal; // This signals the device that we will be sending configuration to it
    confsignal.id = 1;
    confsignal.d1 = 64;
    confsignal.d2 = 0;
    unsigned char returncode = 2;
    int r1 = libusb_bulk_transfer(dev_handle,2,(unsigned char*)&confsignal,3,&written,usb_timeout); //We send the header
    int r2 = libusb_bulk_transfer(dev_handle,2,(unsigned char*)(&cfg),64,&written,usb_timeout); //and also the 64 byte config struct
    if (r1 != 0 | r2 != 0){
        string ex = "Bulk write failed"; //This means that I probably messed up something in the code
        libusb_release_interface(dev_handle,0);
        libusb_close(dev_handle);
        libusb_exit(ctx);
        throw ex;
    }
    if (libusb_bulk_transfer(dev_handle, 130, &returncode, 1, &written, usb_timeout) != 0){ //if everything is good, the device answers an OK (not exactly, but it is what it is)
        string ex = "Bulk read failed"; //like with the write
        libusb_release_interface(dev_handle,0);
        libusb_close(dev_handle);
        libusb_exit(ctx);
        throw ex;
    } else {
        libusb_release_interface(dev_handle,0);
        libusb_close(dev_handle);
        libusb_exit(ctx);
        return 0;
    }
}


int voltcraft::download(unsigned short int **results, confdata &cfdata) const{ //retur value is the downloaded data's length
    libusb_device_handle *dev_handle = NULL; //device
    libusb_context *ctx; //libusb context
    libusb_init(&ctx);
    libusb_device **devs; //device list
    int cnt = libusb_get_device_list(ctx, &devs); //just like during the setup
    bool found = false;
     for (int i = 0; i < cnt && !found; i++){
        libusb_device_descriptor desc;
        libusb_get_device_descriptor(devs[i], &desc);
        if (desc.idVendor == v && desc.idProduct == p) found = true;
    }
    libusb_free_device_list(devs, 1);
    if (!found){
        string ex = "Device not found. Is it connected?";
        libusb_exit(ctx);
        throw ex;
    }
    dev_handle = libusb_open_device_with_vid_pid(ctx,v,p);
    if (dev_handle == NULL){
        string ex = "Device is found, but opening it failed. Do you have the permission to do so?";
        libusb_exit(ctx);
        throw ex;
    }
    if (libusb_kernel_driver_active(dev_handle,0) == 1){
        if (libusb_detach_kernel_driver(dev_handle,0) != 0){
            string ex = "Could not detach kernel driver assigned to the device. This may be a permission issue.";
            libusb_close(dev_handle);
            libusb_exit(ctx);
            throw ex;
        }
    }
    if (libusb_claim_interface(dev_handle,0) != 0){
        string ex = "Claiming USB interface failed. Check if you have the correct permissions to execute this task.";
        libusb_close(dev_handle);
        libusb_exit(ctx);
        throw ex;
    }
    int ret1,ret2 = -1;
    ret1 = libusb_control_transfer(dev_handle,64,0,65535,0,NULL,0,usb_timeout);
    ret2 = libusb_control_transfer(dev_handle,64,2,2,0,NULL,0,usb_timeout);


    //this is the point where things differ
    int dread=0; //bytes read
    confdata conf;
    b3header confsignal; //This signals, that we'd like to read the configuration options. This also stops the measurement, and there is no other way to stop it
    b2header downloadable;
    confsignal.id = 0;
    confsignal.d1 = 16;
    confsignal.d2 = 1;
    libusb_bulk_transfer(dev_handle,2,(unsigned char*)&confsignal,3,&dread,usb_timeout); //Sending the request
    libusb_bulk_transfer(dev_handle,130,(unsigned char*)&downloadable,3,&dread,usb_timeout); //sends back an OK header
    libusb_bulk_transfer(dev_handle,130,(unsigned char*)&cfdata,64,&dread,usb_timeout); //Sends back the configuration options
    libusb_bulk_transfer(dev_handle,130,NULL,0,NULL,usb_timeout); //after downloading the config, the device sends an empty packet for some reason, we should receive that

//    downloadable.dat = 64000;
//    cfdata.idk = 32000;
    downloadable.dat = cfdata.idk*2; //for some reason, this is a more reliable source of the recorded data's length, so I'll be using this
    int requestcount = downloadable.dat/4096; //How many times we request data from it (with a request it can send one block of data - so 4096)
    int blokkremain = downloadable.dat % 4096; //how much data is out of the 4kB block
    int rems = blokkremain/64; // how many times do we need to request 64 datapoints in the last block
    int datain = 0;
    b2header response;
    if (blokkremain % 64 != 0 && blokkremain != 0){
        rems++; //if it is not dividable, this should be increased by one
    }
    int total64; //how much 64 byte packets are neccesary
    if (downloadable.dat%64 != 0){
        total64 = downloadable.dat/64 + 1;
    } else {
        total64 = downloadable.dat;
    }
    unsigned short int *ddata = new unsigned short int [total64*32]; //creating the array with the right size
    for (int i = 0; i <= requestcount; i++){ //how many requests should we make?
        confsignal.id=0; //this is always 0
        confsignal.d1=i; //This is the current block number
        if (i == requestcount){ //if we are in the block with the last datapoints
            confsignal.d2 = rems; //we need this many times more 64 byte packets
            int db = blokkremain/512; //this is how much packets will the device send
            int last = (blokkremain%512)/64; //in the last block how many times 64 byte packets are neccesary
            if ((blokkremain%512)%64 != 0){
                last++;
            }
            libusb_bulk_transfer(dev_handle,2,(unsigned char*)&confsignal,3,&dread,usb_timeout); //this tells the device, that we'd like to download the data
            libusb_bulk_transfer(dev_handle,130,(unsigned char*)&response,3,NULL,usb_timeout); //sends an OK (02 00 00 header)
            for (int j = 0; j < db; j++){
                dread = 0;
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&(ddata[datain]),512,&dread,usb_timeout); //here the other packets are read too
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&response,3,NULL,usb_timeout);
                datain += dread/2;
            }
            if (last != 0){
                dread = 0;
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&(ddata[datain]),last*64,&dread,usb_timeout);//if there is still a last packet, we have to read that too
                libusb_bulk_transfer(dev_handle,130,NULL,0,NULL,usb_timeout); 
                datain += dread/2;
            }
        } else {
            confsignal.d2 = 64;
            libusb_bulk_transfer(dev_handle,2,(unsigned char*)&confsignal,3,&dread,usb_timeout); //The same thing happens like before, just here it is guaranteed, that 8 times 512 byte packets will come
            libusb_bulk_transfer(dev_handle,130,(unsigned char*)&response,3,NULL,usb_timeout); //answer
            for (int j = 0; j < 8; j++){
                dread = 0;
                int bread = 0;
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&(ddata[datain]),512,&dread,usb_timeout); //data reading
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&response,3,&bread,usb_timeout); // empty packet
                datain += dread/2;
            }
        }
    }
    libusb_control_transfer(dev_handle,64,2,4,0,NULL,0,usb_timeout); //cleanup
    libusb_release_interface(dev_handle,0); //clenaup
    libusb_close(dev_handle); //cleanup
    libusb_exit(ctx);//cleanup
    *results = ddata; //recorded data
    return downloadable.dat/2; //returns the read data's length. Not really neccesary anymore, I do it differently now.
}
