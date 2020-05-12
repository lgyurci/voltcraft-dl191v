#include <libusb-1.0/libusb.h>
#include "voltcraft.h"
#include "datastructs.h"
#include <ctime>
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

int voltcraft::configure (int datacount, int freq, std::tm *time, int ledmode, bool instant) const{ //return value igazából nem is kéne, mert megoldja az exception handling
    libusb_device_handle *dev_handle = NULL; //eszköz
    libusb_context *ctx; //kontextus
    libusb_init(&ctx);
    libusb_device **devs; //eszközlista
    int cnt = libusb_get_device_list(ctx, &devs);
    bool found = false;
     for (int i = 0; i < cnt && !found; i++){
        libusb_device_descriptor desc;
        libusb_get_device_descriptor(devs[i], &desc);
        if (desc.idVendor == v && desc.idProduct == p) found = true; //megnézzük, hogy egyáltalán be van-e dugva az eszköz
    }
    libusb_free_device_list(devs, 1);
    if (!found){
        string ex = "Device not found. Is it connected?"; //reklamálunk ha nincs
        libusb_exit(ctx);
        throw ex;
    }
    dev_handle = libusb_open_device_with_vid_pid(ctx,v,p); //rámászunk az eszközre
    if (dev_handle == NULL){
        string ex = "Device is found, but opening it failed. Do you have the permission to do so?"; //99% hogy permission error
        libusb_exit(ctx);
        throw ex;
    }
    if (libusb_kernel_driver_active(dev_handle,0) == 1){ //ha a kernel ráakasztott valami drivert
        if (libusb_detach_kernel_driver(dev_handle,0) != 0){ //akkor leszedjük róla
            string ex = "Could not detach kernel driver assigned to the device. This may be a permission issue."; //Ez szívás. Ilyen hibám még nem volt, nem tudom mit kéne csinálni
            libusb_close(dev_handle);
            libusb_exit(ctx);
            throw ex;
        }
    }
    if (libusb_claim_interface(dev_handle,0) != 0){ //magunkévá tesszük az interfészt
        string ex = "Claiming USB interface failed. Check if you have the correct permissions to execute this task."; //szintén nem tudom hogy mikor fordul elő
        libusb_close(dev_handle);
        libusb_exit(ctx);
        throw ex;
    }
    int ret1,ret2 = -1; //mivel nem ellenőrzöm a visszatérési értékeket, ezért nincs rá szükség, de egyszer hátha beleépítem még, addig foglalja a setup alatt itt a 4 byteot
    ret1 = libusb_control_transfer(dev_handle,64,0,65535,0,NULL,0,usb_timeout); // itt átvisszük az usb control adatokat, ez egy az egyben a windowsos szoftver által átküldött adatok mása
    ret2 = libusb_control_transfer(dev_handle,64,2,2,0,NULL,0,usb_timeout); // (és amúgy sem tudom hogy mit csinál pontosan, de enélkül nem működik)

    //ha idáig eljutottunk, akkor van egy élő kommunikációnk az eszközzel. Hurrá
    confdata cfg; //létrehozzuk a konfigot
    cfg.datacount = datacount;
    cfg.freq = freq; //igazából nem frekvencia, hanem periódus, de ez menniyvel jobban hangzik már
    cfg.year = time->tm_year + 1900;
    cfg.month = time->tm_mon + 1;
    cfg.day = time->tm_mday;
    cfg.hour = time->tm_hour;
    cfg.min = time->tm_min;
    cfg.sec = time->tm_sec;
    unsigned char ledchar = (unsigned char) ledmode; // Led villogásának periódusideje
    unsigned char frmode; // Ez nemtudom mi, de bizonyos frekvenciákhoz adott értékek tartoznak, a többihez meg 11 (binárisban, a 2. és 3. biten)
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
    bool alarm = false; //future function, a műszeren be lehet állítani egy riasztást, ami nagy vagy kis feszültségek esetén riaszt, de nem sikerült megfejtenem ennek a kódját, és nem fontos amúgy sem
    unsigned char alarmchar;
    if (alarm){
        alarmchar = 128;
    } else {
        alarmchar = 0;
    }
    unsigned char Q = ledchar | frmode | alarmchar; //kombináljuk a számokat, így kapva meg a megfelelő bináris kódot
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
    cfg.confname = confname;*/ //Valamiért összekuszálja a struktúra bytekódját, nem érdemes megkockáztatni állítani, nincs gyakorlatilag semmi haszna, marad a hardcode-olt "Brutus"
    unsigned char start; // elinduljon-e a mérés közvetlenül a konfig kiírása után
    if (freq == 0){ // 400Hz-es frekvenciánál valamiért mások a bitek, tippre az ezen a frekvencián folyamatosan világító led miatt, de nem tudom pontosan
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




    int written=0; // kiírt byteok száma
    b3header confsignal; // ezzel jelezzük az eszköznek, hogy konfigurálni akarjuk
    confsignal.id = 1;
    confsignal.d1 = 64;
    confsignal.d2 = 0;
    unsigned char returncode = 2;
    int r1 = libusb_bulk_transfer(dev_handle,2,(unsigned char*)&confsignal,3,&written,usb_timeout); //elküldjük neki, hogy konfigurációs beállításokat akarunk küldeni
    int r2 = libusb_bulk_transfer(dev_handle,2,(unsigned char*)(&cfg),64,&written,usb_timeout); // elküldjük neki a 64 byteos konfigurációs struktúrát
    if (r1 != 0 | r2 != 0){
        string ex = "Bulk write failed"; //Arra utal, hogy valamit elcsesztem a kódban, nem a felhasználó hibája (kivételesen, minden mást simán ráfogok)
        libusb_release_interface(dev_handle,0);
        libusb_close(dev_handle);
        libusb_exit(ctx);
        throw ex;
    }
    if (libusb_bulk_transfer(dev_handle, 130, &returncode, 1, &written, usb_timeout) != 0){ //jó esetben az eszköz válaszol hogy OK (ezt egy 8 egyesből álló byte-tal teszi)
        string ex = "Bulk read failed"; //mint a write-nál
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


int voltcraft::download(unsigned short int **results, confdata &cfdata) const{ //visszatérési értéke a letöltött mérési adatmennyiség, azaz a results tömb hossza
    libusb_device_handle *dev_handle = NULL; //eszköz
    libusb_context *ctx; //kontextus
    libusb_init(&ctx);
    libusb_device **devs; //eszközlista
    int cnt = libusb_get_device_list(ctx, &devs); //nem folytatom, ugyan az mint a setupnál végig
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


    //na innen különbözik
    int dread=0; //beolvasott byteok száma, itt aktuálisan van haszna is
    confdata conf;
    b3header confsignal; //ezzel jelezzük, hogy kérjük tőle a konfigurációs beállításokat. Ez állítja le a mérést is.
    b2header downloadable;
    confsignal.id = 0;
    confsignal.d1 = 16;
    confsignal.d2 = 1;
    libusb_bulk_transfer(dev_handle,2,(unsigned char*)&confsignal,3,&dread,usb_timeout); // szólunk neki, hogy mutassa meg, milye van
    libusb_bulk_transfer(dev_handle,130,(unsigned char*)&downloadable,3,&dread,usb_timeout); //küld egy headert, jó esetben az első byteban 02-vel
    libusb_bulk_transfer(dev_handle,130,(unsigned char*)&cfdata,64,&dread,usb_timeout); //elküldi a konfigurációs beállításait, ebben lényeges a frekvencia, az idő, és hogy hány adatot rögzített
    libusb_bulk_transfer(dev_handle,130,NULL,0,NULL,usb_timeout); //a konfiguráció letöltése állítja le egyben a mérést. A konfig letöltése után az eszköz még küld egy üres packetet, mert olyan kedve van olyankor

//    downloadable.dat = 64000;
//    cfdata.idk = 32000;
    downloadable.dat = cfdata.idk*2; //inkább dolgozok ebből, mert megbízhatóbbnak tűnik. A headerben valahogy elcsúszik a bytecode, és nincs kedvem kinyomozni, és amúgy meg mindegy
    int requestcount = downloadable.dat/4096; //hányszor kell tőle adatot kérnünk (max 4096 byte-ot tud elküldeni egy kéréssel)
    int blokkremain = downloadable.dat % 4096; //mennyi adat lóg ki a 4kB-os blokkokból
    int rems = blokkremain/64; // hányszor kell 64-et kérnünk tőle az utolsó blokkban
    int datain = 0;
    b2header response;
    if (blokkremain % 64 != 0 && blokkremain != 0){
        rems++; //ha nem osztható pont, akkor értelemszerűen meg kell egyel növelnünk
    }
    int total64; //Hány 64 byteos csomagba fér bele az összes adat
    if (downloadable.dat%64 != 0){
        total64 = downloadable.dat/64 + 1;
    } else {
        total64 = downloadable.dat;
    }
    unsigned short int *ddata = new unsigned short int [total64*32]; //létrehozzuk a megfelelő méretű tömböt
    for (int i = 0; i <= requestcount; i++){ //ahány kérést kell csinálnunk
        confsignal.id=0; //ez mindig nulla
        confsignal.d1=i; //ez azzal a számmal egyezik meg, ahanyadik blokkban vagyunk
        if (i == requestcount){ //ha az utolsó adatokat tartalmazó blokkban vagyunk
            confsignal.d2 = rems; //ennyiszer 64 byte adat kell tőle
            int db = blokkremain/512; //ennyi packetet fog elküldeni, mert 512 byte-os packeteket küld max (min 64-et)
            int last = (blokkremain%512)/64; //utolsó részben hányszor 64 kell
            if ((blokkremain%512)%64 != 0){
                last++;
            }
            libusb_bulk_transfer(dev_handle,2,(unsigned char*)&confsignal,3,&dread,usb_timeout); //szólunk neki, hogy leszednénk az adatokat
            libusb_bulk_transfer(dev_handle,130,(unsigned char*)&response,3,NULL,usb_timeout); //szól hogy OK (02 00 00 header), annyira nem érdekes, de ki kell olvasni
            for (int j = 0; j < db; j++){
                dread = 0;
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&(ddata[datain]),512,&dread,usb_timeout); // kiolvassuk a többi packetet
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&response,3,NULL,usb_timeout);
                datain += dread/2;
            }
            if (last != 0){
                dread = 0;
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&(ddata[datain]),last*64,&dread,usb_timeout);//kiolvassuk az utolsó packetet, ha maradt még
                libusb_bulk_transfer(dev_handle,130,NULL,0,NULL,usb_timeout); 
                datain += dread/2;
            }
        } else {
            confsignal.d2 = 64;
            libusb_bulk_transfer(dev_handle,2,(unsigned char*)&confsignal,3,&dread,usb_timeout); //itt ugyan az történik mint előbb, csak itt garantáltan mindig 8-szor 512 byte-os packetek fognak érkezni sorban
            libusb_bulk_transfer(dev_handle,130,(unsigned char*)&response,3,NULL,usb_timeout); //válasz
            for (int j = 0; j < 8; j++){
                dread = 0;
                int bread = 0;
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&(ddata[datain]),512,&dread,usb_timeout); //adatkiolvasás
                libusb_bulk_transfer(dev_handle,130,(unsigned char*)&response,3,&bread,usb_timeout); // üres packet
                datain += dread/2;
            }
        }
    }
    libusb_control_transfer(dev_handle,64,2,4,0,NULL,0,usb_timeout); //cleanup
    libusb_release_interface(dev_handle,0); //clenaup
    libusb_close(dev_handle); //cleanup
    libusb_exit(ctx);//cleanup
    *results = ddata; //mérési eredmények
    return downloadable.dat/2; // visszaadjuk a kiolvasott adatmenniységet, de igazából már ez sem érdekes, mert máshogy csinálom
}