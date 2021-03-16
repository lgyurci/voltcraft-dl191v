//Lázár György, 2020

#pragma once

struct confdata { //The idea belongs to https://github.com/mildis/vdl120 
    const int startsignal = 206; //config start/stop signal, always 206 (hex: 0xce)
    int datacount = 32000; //Maximum data to be recorded (1 measurement point is stored in 2 bytes, which is fine, because the device has a measurement range of 30V, and this way it could even go up to 64V)
    int idk = 0; //Gives back the number of datapoints recorded, while writing the config, this is always has to be 0
    int freq = 2; //interval between measurements in seconds, 0 means 0.0025s (400 Hz)
    int year = 2020; //year of the config write
    int alarm_low1 = 1086324736; //This somehow encodes the lower voltage limit for the alert led to light up, but I don't understand how, so I just left this value here, which appeared to be working
    int alarm_high1 = 1106247680; //Same goes for the upper alert limit
    char month = 7; //month of config write
    char day = 6; //day of config write
    char hour = 10; //hour of config write
    char min = 32; //mins of config write
    char sec = 42; //seconds of config write
    char yes = 0; //always 0
    char Q = 10; //This encodes a lot of things: 1. bit: alert on/off, 2-3. bit: I'm not sure what is this, but it depends on the interval of the measurement (0-2: 00, 5: 01, 10:10, 10+:11), last 5 bit: status led blinking interval in seconds
    char confname[16] = "Brutus"; //name of the configuration, mostly useless (maybe not if you have multiple devices), so it is hardcoded
    char start = 1; //1: The measurement starts with button press 2: The measurement starts instantly after writing the config (at 400 Hz, these values are 17 and 18 respectively [Binary 10001 and 10011])
    int alarm_low2 = 1086324736; //These values appear two times in the config, because on some other devices, you can set the alarm for more than one thing (like temp and humidity)
    int alarm_high2 = 1106247680;
    const int endsignal = 206; //same as the start
};

struct b2header{
    char id; //download/config read request: 0, config write: 1, at device answers: 2 / at downloads, it's more ssensible to use the 3 element header
    unsigned short int dat; //config write: 64, config read: 272, device answers: 0, except when downloading the configuration, because there the first header includes the size of the recorded data (so it indicates how much you should download)
};

struct b3header{ //only usable while downloading, or when writing config data
    char id = 0; // download: 0
    char d1; // The device divides it's 64kB memory into 4kB blocks, and we can specify here, which one do we read from
    char d2; // From the given block, how much times 64 bytes do we request (max 64, 64*64=4096, which is the full blocksize). It is important to know from the download header how much is recorded data, and how much is memory garbage, because the device can only send data in 64 byte arrays
};
