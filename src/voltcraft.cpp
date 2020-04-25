#include <libusb-1.0/libusb.h>
#include <ctime>
#include "voltcraft.h"
#include "datastructs.h"
#include <string.h>
#include <iostream>

using namespace std;

voltcraft::voltcraft(int vid/*vendor id*/, int pid/*product id*/){
    v = vid;
    p = pid;
}

voltcraft::~voltcraft(){
    //Eredetileg úgy működött volna az objektum, hogy a contructor rácsatlakozik egyből az eszközre, és elküldi neki a control packeteket is, azonban ez a libusb-nek
    //nem nagyon tetszett, valamiért ha nem volt egy metódusban a kommunikáció nyitása és folytatása lehalt az egész, nem teljesen tudom hogy miért (egyáltalán nem tudom)
    //Szóval maradt ez a megoldás, hogy mindig amikor egy metódusban kommunikálok az eszközzel, akkor újra nyitom/zárom a kommunikációt, ideális esetben ez így sem történik
    //meg többször mint egyébként, de valóban, az objektumorientált logikába sokkal jobban illeszkedett volna, ha private változóként eltárolom még a libusb_context-et
    //és a dev_handle-t, és ezeken keresztül kommunikálok. De ez elméletileg csak a kód mennyiségét befolyásolja, a működésnek így is meg kell egyeznie tökéletesen,
    //hiszen jó esetben az eszközt felkonfiguráljuk (1 nyitás-zárás), kihúzzuk, mérünk vele, visszadugjuk, letöltjük róla az adatokat(még egy nyitás-zárás), tehát nem szükséges a kommunikációt életben tartani
}

int voltcraft::configure (int datacount, int freq, std::tm *time, int ledmode, bool instant){ //return value igazából nem is kéne, mert megoldja az exception handling
    libusb_device_handle *dev_handle = NULL; //eszköz
    libusb_context *ctx; //kontextus
    libusb_init(&ctx);
    libusb_device **devs; //eszközlista
    int cnt = libusb_get_device_list(ctx, &devs);
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
    ret1 = libusb_control_transfer(dev_handle,64,0,65535,0,NULL,0,0);
    ret2 = libusb_control_transfer(dev_handle,64,2,2,0,NULL,0,0);


    confdata cfg;
    cfg.datacount = datacount;
    cfg.freq = freq;
    cfg.year = time->tm_year + 1900;
    cfg.month = time->tm_mon + 1;
    cfg.day = time->tm_mday;
    cfg.hour = time->tm_hour;
    cfg.min = time->tm_min;
    cfg.sec = time->tm_sec;
    unsigned char ledchar = (unsigned char) ledmode;
    unsigned char frmode;
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
    bool alarm = false; //future function
    unsigned char alarmchar;
    if (alarm){
        alarmchar = 128;
    } else {
        alarmchar = 0;
    }
    unsigned char Q = ledchar | frmode | alarmchar;
    cfg.Q = Q;
/*    char confname[16];
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
    cfg.confname = confname;*/ //Valamiért összekuszálja a struktúra bytekódját, nem érdemes megkockáztatni állítani, nincs gyakorlatilag semmi haszna, marad a hardcode-olt "Brutus"
    unsigned char start;
    if (freq == 0){
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




    int written=0;
    b3header confsignal;
    confsignal.id = 1;
    confsignal.d1 = 64;
    confsignal.d2 = 0;
    unsigned char returncode = 2;
    int r1 = libusb_bulk_transfer(dev_handle,2,(unsigned char*)&confsignal,3,&written,0);
    int r2 = libusb_bulk_transfer(dev_handle,2,(unsigned char*)(&cfg),64,&written,0);
    if (r1 != 0 | r2 != 0){
        string ex = "Bulk write failed";
        libusb_release_interface(dev_handle,0);
        libusb_close(dev_handle);
        libusb_exit(ctx);
        throw ex;
    }
    if (libusb_bulk_transfer(dev_handle, 130, &returncode, 1, &written, 0) != 0){
        string ex = "Bulk read failed";
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


int voltcraft::download(unsigned short int **results, confdata &cfdata){ //visszatérési értéke a letöltött mérési adatmennyiség, azaz a results tömb hossza
    libusb_device_handle *dev_handle = NULL; //eszköz
    libusb_context *ctx; //kontextus
    libusb_init(&ctx);
    libusb_device **devs; //eszközlista
    int cnt = libusb_get_device_list(ctx, &devs);
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
    ret1 = libusb_control_transfer(dev_handle,64,0,65535,0,NULL,0,0);
    ret2 = libusb_control_transfer(dev_handle,64,2,2,0,NULL,0,0);



    int dread=0;
    confdata conf;
    b3header confsignal;
    b2header downloadable;
    confsignal.id = 0;
    confsignal.d1 = 16;
    confsignal.d2 = 1;
    libusb_bulk_transfer(dev_handle,2,(unsigned char*)&confsignal,3,&dread,0);
    libusb_bulk_transfer(dev_handle,130,(unsigned char*)&downloadable,3,&dread,0);
    libusb_bulk_transfer(dev_handle,130,(unsigned char*)&cfdata,64,&dread,0);
    libusb_bulk_transfer(dev_handle,130,NULL,0,NULL,0); //a konfiguráció letöltése állítja le egyben a mérést. A konfig letöltése után az eszköz még küld egy üres packetet, mert olyan kedve van olyankor

    confsignal.id=0;
    confsignal.d1=0;
    confsignal.d2=1;
    downloadable.dat = cfdata.idk*2;
    int requestcount = downloadable.dat/4096;
    int blokkremain = downloadable.dat % 4096;
    int rems = blokkremain/64;
    int datain = 0;
    b2header response;
    if (blokkremain % 64 != 0 && blokkremain != 0){
        rems++;
    }
    int total64;
    if (downloadable.dat%64 != 0){
        total64 = downloadable.dat/64 + 1;
    } else {
        total64 = downloadable.dat;
    }
    unsigned short int *ddata = new unsigned short int [total64*32];
    for (int i = 0; i <= requestcount; i++){
        confsignal.id=0;
        confsignal.d1=i;
        if (i == requestcount){
            confsignal.d2 = rems;
            int db = blokkremain/512;
            int last = rems-db;
            libusb_bulk_transfer(dev_handle,2,(unsigned char*)&confsignal,3,&dread,0);
            libusb_bulk_transfer(dev_handle,130,(unsigned char*)&response,3,NULL,0);
            for (int j = 0; j < db; j++){
                dread = 0;
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&(ddata[datain]),512,&dread,0);
                datain += dread;
            }
            if (last != 0){
                dread = 0;
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&(ddata[datain]),last*64,&dread,0);
                datain += dread;
            }
        } else {
            confsignal.d2 = 64;
            libusb_bulk_transfer(dev_handle,2,(unsigned char*)&confsignal,3,&dread,0);
            libusb_bulk_transfer(dev_handle,130,(unsigned char*)&response,3,NULL,0);
            for (int j = 0; j < 8; j++){
                dread = 0;
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&(ddata[datain]),512,&dread,0);
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&response,3,NULL,0);
                datain += dread;
            }
        }
    }
    libusb_bulk_transfer(dev_handle,130,NULL,0,NULL,0);
    libusb_control_transfer(dev_handle,64,2,4,0,NULL,0,0);
    libusb_release_interface(dev_handle,0);
    libusb_close(dev_handle);
    libusb_exit(ctx);
    *results = ddata;
    return downloadable.dat/2;
}