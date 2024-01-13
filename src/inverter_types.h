#pragma once
#define VERSION "0.1.0"
#include "Arduino.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#ifndef inverter_types_h
#define inverter_types_h

typedef struct iv_values
{
    String name;
    int max_power;
    float Efficiency;
    float Irradiation;
    float YieldTotal;
    float YieldDay;
    float Temp;
    float Freq;

    float P_ACr;
    float P_AC;
    float I_AC;
    float U_AC;
    float Q_AC;
    float PF_AC;

    float U_DC;
    float I_DC;
    float P_DC;
    
};

enum dtu_state
{
    DTU_NOTHING,
    DTU_UPDATE_AVAILABLE,
    DTU_UPDATE_FAILED,
    DTU_INIT_INCOMPLETE,
    DTU_UPDATE_PROCESSED
};

typedef struct inverter_vals
{
    String name;
    String address;
    time_t lastupdate;
    tm tm_lastupdate;
    int lastreset;
    boolean available[24];
    int recieve_count[24];
    long uptime;
    boolean update;
    int max_power[5];

    float money_saved; //stored money saved
    float oldyieldtotal;
    float priceperkwh;
    int channels;
    iv_values modules[5];
    iv_values hour_avg[5][24];
    dtu_state state;
};


class dtu
{
    public:
        void init(const String address, int update_time);
        String buildURL(String record);
        void fetch_data(inverter_vals *iv);
        void loop(inverter_vals *iv);
        void resetaverage(inverter_vals *iv, int mday);
        void initInverter(inverter_vals *iv);
    private:
        JsonDocument makequerry(String record);
        void extractData(JsonObject data, inverter_vals *iv);
        void recalcInverterAverage(inverter_vals *iv);
        String _address;
        int _update_time;
        boolean _init_complete;
        time_t dtu_last_success;
        time_t lastpoll;
};

#endif