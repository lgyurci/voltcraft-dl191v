#pragma once
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
    char confname[16] = "Brutus"; //konfiguráció neve, igazából lényegtelen, de 16 karakter belefér (sok eszköz esetén talán lehet valami szerepe), bonyolult lenne állíthatóvá tenni úgy, hogy a bytecode sértetlen maradjon, ezért van hardcodeolva
    char start = 1; //1: a mérés gomb megnyomásával indul, 2: a mérés közvetlenül a konfiguráció kiírása után indul (400 Hz-es frekvencia esetén ezek ebben a sorrendben valamiért 17 és 18 [a byte második fele így 1-lesz, az első fele meg marad 1 vagy 10])
    int alarm_low2 = 1086324736; //Ezek az értékek kétszer szerepelnek a konfigurációban, mert más eszközök esetén lehet hogy 2 dologra is be lehet állítani riasztást, de itt nem (pl. hőszenzornál hőmérséklet vagy páratartalom)
    int alarm_high2 = 1106247680;
    const int endsignal = 206; //ugyan az mint a start
};

struct b2header{
    char id; //adatletöltés/konfigurácó kérése esetén: 0, konfiguráció írásakor: 1, az eszköz válaszainál: 2 / adatletöltés esetén érdemesebb a 3 elemű headert használni
    short int dat; //konfig írásakor: 64, konfig lekérésekor: 272, az eszköz válaszainál 0, kivéve a konfiguráció lekérésénél, mert ott az első header itt tartalmazza, hogy hány byte adat keletkezett a legutóbbi méréssel, tehát hogy mennyit kell letölteni (és a letöltökkeből mennyi használható, és mennyi memóriaszemét)
};

struct b3header{ //csak adatletöltéskor és konfig íráskor hasznos
    char id = 0; // adatletöltéskor 0
    char d1; // Az eszköz a 64kB-os memóriáját 4kB-os blokkokra osztja, itt tudjuk megadni, hogy melyik blokkból olvasunk
    char d2; // Az adott blokkból az első hányszor 64 byteot kérjük (max 64, 64*64=4096, ami ugye a teljes blokkméret). Itt fontos az, hogy tudjuk hogy mennyi valóban mérési adat, és mennyi memóriaszemét, mert az eszköz legalább 64 byte-ot mindig elküld
};