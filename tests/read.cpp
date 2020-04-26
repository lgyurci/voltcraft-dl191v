#include <iostream>
#include <libusb-1.0/libusb.h> //A sima usb.h nem tudja (általam ismert módon) lecsatlakoztatni a kernel drivert, ezért nem nagyon alkalmas ide
#define dven 4292
#define dprod 60001

using namespace std;

struct confdata { //Ezt az ötletet https://github.com/mildis/vdl120 -ből loptam (szerintem zseniális, én ezerszer bonyolultabban csináltam volna meg), aki egy szintén voltcraft hőmérsékletmérő vezérlését kódolta le. Ez a projekt a kommunikáció dekódolásában is sokat segített, máshoz viszont nem használtam fel (alapból az elavultabb, usb.h-t használja libusb helyett)
    const int startsignal = 206; //config start/stop jel, mindig 206 (hex: 0xce)
    int datacount = 16000; //Rögzítendő adatmennyiség maximális száma (1 mérési adat 2 byte-ban tárolódik, ami rendben van, mert 30V a műszer méréshatára, és így max 65V-ig tudna adatot tárolni)
    int idk = 0; //Mindig 0
    int freq = 2; //rögzítés időköze másodpercben, 0: 0.0025s-t jelent (400 Hz)
    int year = 2020; //konfiguráció kiírásának éve
    int alarm_low1 = 1086324736; //Ez valahogy a riasztás alsó feszültséghatárát kódolja, sajnos nem jöttem rá hogy hogyan, szóval egy érvényes értéken hagytam
    int alarm_high1 = 1106247680; //ugyan úgy a felsőt
    char month = 7; //konfiguráció kiírásának hónapja
    char day = 6; //napja
    char hour = 10; //órája
    char min = 32; //perce
    char sec = 42; //másodperce
    char yes = 0; //mindig 0
    char Q = 10; //Ez sok mindent kódol: 1. bit: riasztás ki-be, 2-3. bit: a mérés frekvenciájától függ, valami módot állít szerintem (0-2: 00, 5: 01, 10:10, 10+:11), utolsó 5 bit: állapotjelző led villogásának periódusideje másodpercben
    char *confname; //konfiguráció neve, igazából lényegtelen, de 16 karakter belefér (sok eszköz esetén talán lehet valami szerepe)
    char start = 1; //1: a mérés gomb megnyomásával indul, 2: a mérés közvetlenül a konfiguráció kiírása után indul (400 Hz-es frekvencia esetén ezek ebben a sorrendben valamiért 17 és 18 [a byte második fele így 1-lesz, az első fele meg marad 1 vagy 10])
    int alarm_low2 = 1086324736; //Ezek az értékek kétszer szerepelnek a konfigurációban, mert más eszközök esetén lehet hogy 2 dologra is be lehet állítani riasztást, de itt nem (pl. hőszenzornál hőmérséklet vagy páratartalom)
    int alarm_high2 = 1106247680;
    const int endsignal = 206; //ugyan az mint a start
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
    libusb_bulk_transfer(dev_handle,130,NULL,0,NULL,0);

    confsignal[0]=0;
    confsignal[1]=0;
    confsignal[2]=1;
    libusb_bulk_transfer(dev_handle,2,confsignal,3,&written,0);
    short int rdt[32];
    libusb_bulk_transfer(dev_handle,130,confsignal,3,&written,0);
    cout << "header received: " << (int) confsignal[0] << " " << (int) confsignal[1] << " " << (int) confsignal[2] << " " << endl;
    libusb_bulk_transfer(dev_handle,130,(unsigned char*) &rdt,64,&written,0);
    cout << written << " bytes were read:" << endl;
    for (int i = 0; i < written/2; i++){
        cout << rdt[i] << " ";
    }
    cout << endl;
    libusb_bulk_transfer(dev_handle,130,NULL,0,NULL,0);
    libusb_control_transfer(dev_handle,64,2,4,0,NULL,0,0);
    libusb_release_interface(dev_handle,0);
    libusb_close(dev_handle);
    libusb_exit(ctx);
}