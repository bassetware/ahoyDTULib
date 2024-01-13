#include "inverter_types.h"

void dtu::init(const String address, int update_time)
{
    _address = address;
    _update_time = update_time;
    _init_complete = true;
};

void dtu::initInverter(inverter_vals *iv)
{
    JsonDocument docIv;
    docIv = this->makequerry("inverter/list");
    // serializeJsonPretty(docIv, Serial);
    if (docIv.containsKey("error"))
    {
        iv->state = DTU_INIT_INCOMPLETE;
        return;
    }
    else
    {
        iv->name = docIv["inverter"][0]["name"].as<String>();
        iv->channels = docIv["inverter"][0]["channels"].as<unsigned int>();
        iv->max_power[0] = 0;
        for (int i = 0; i < iv->channels; i++)
        {
            iv->modules[i + 1].name = docIv["inverter"][0]["ch_name"][i].as<String>();
            iv->max_power[i + 1] = docIv["inverter"][0]["ch_max_pwr"][i].as<unsigned int>();
            iv->max_power[0] += iv->max_power[i + 1];
        }
        iv->state = DTU_NOTHING;
    }
}

String dtu::buildURL(String record)
{
    String url = "http://";
    url += _address;
    url += "/api/";
    url += record;
    return url;
};

void dtu::recalcInverterAverage(inverter_vals *iv)
{
    // Inverter gesamt
    int hour = iv->tm_lastupdate.tm_hour;
    int next = (hour + 1) % 24; // wenn 24 uhr dann wieder bei null anfangen
    iv->available[next] = false;
    for (int i = 0; i <= iv->channels; i++)
    {
        iv->hour_avg[i][hour].P_DC = iv->hour_avg[i][hour].P_DC + ((iv->modules[i].P_DC - iv->hour_avg[i][hour].P_DC) / iv->recieve_count[hour]);
        iv->hour_avg[i][hour].P_AC = iv->hour_avg[i][hour].P_AC + ((iv->modules[i].P_AC - iv->hour_avg[i][hour].P_AC) / iv->recieve_count[hour]);
    }
}

void dtu::resetaverage(inverter_vals *iv, int mday)
{
    for (int h = 0; h < 24; h++)
    {
        iv->available[h] = false;
        iv->hour_avg[iv->channels][h].P_DC = 0;
        iv->hour_avg[iv->channels][h].P_AC = 0;
        iv->recieve_count[h] = 0;
        iv->lastreset = mday;
    }
}

void dtu::extractData(JsonObject data, inverter_vals *iv)
{

    // iv->lastupdate = index["inverter"][0]["ts_last_success"].as<unsigned long>();
    //  Serial.println("IV: lastupdate:");
    //  Serial.println(iv->lastupdate);
    iv->lastupdate = data["ts_last_success"].as<unsigned long>();
    localtime_r(&iv->lastupdate, &iv->tm_lastupdate);

    iv->modules[0].U_AC = data["ch"][0][0].as<float>();
    iv->modules[0].I_AC = data["ch"][0][1].as<float>();
    iv->modules[0].P_AC = data["ch"][0][2].as<float>();
    iv->modules[0].Freq = data["ch"][0][3].as<float>();
    iv->modules[0].PF_AC = data["ch"][0][4].as<float>();
    iv->modules[0].Temp = data["ch"][0][5].as<float>();
    iv->modules[0].YieldTotal = data["ch"][0][6].as<float>();
    iv->modules[0].YieldDay = data["ch"][0][7].as<float>();
    iv->modules[0].P_DC = data["ch"][0][8].as<float>();
    iv->modules[0].Efficiency = data["ch"][0][9].as<float>();
    iv->modules[0].Q_AC = data["ch"][0][10].as<float>();
    iv->modules[0].max_power = data["ch"][0][11].as<float>();

    for (int i = 1; i <= iv->channels; i++)
    {
        iv->modules[i].name = data["ch_name"][i].as<String>();
        iv->modules[i].U_DC = data["ch"][i][0].as<float>();
        iv->modules[i].I_DC = data["ch"][i][1].as<float>();
        iv->modules[i].P_DC = data["ch"][i][2].as<float>();
        iv->modules[i].YieldDay = data["ch"][i][3].as<float>();
        iv->modules[i].YieldTotal = data["ch"][i][4].as<float>();
        iv->modules[i].Irradiation = data["ch"][i][5].as<float>();
        iv->modules[i].max_power = data["ch"][i][6].as<float>();
    }
    // Serial.println(iv->modules[1].name);
    // Serial.println(iv->modules[1].max_power);
    /*
    for (JsonVariant item : data["inverter"][0].as<JsonArray>())
    {
        if (item["fld"].as<String>() == "U_DC")
        {
            iv->modules[channel].U_DC = item["val"].as<float>();
        }
        if (item["fld"].as<String>() == "I_DC")
        {
            iv->modules[channel].I_DC = item["val"].as<float>();
        }
        if (item["fld"].as<String>() == "P_DC")
        {
            iv->modules[channel].P_DC = item["val"].as<float>();
        }
        if (item["fld"].as<String>() == "YieldDay")
        {
            iv->modules[channel].YieldDay = item["val"].as<float>();
        }
        if (item["fld"].as<String>() == "YieldTotal")
        {
            iv->modules[channel].YieldTotal = item["val"].as<float>();
        }
        if (item["fld"].as<String>() == "Irradiation")
        {
            iv->modules[channel].Irradiation = item["val"].as<float>();
        }
        if (item["fld"].as<String>() == "U_AC")
        {
            iv->modules[channel].U_AC = item["val"].as<float>();
        }
        if (item["fld"].as<String>() == "I_AC")
        {
            iv->modules[channel].I_AC = item["val"].as<float>();
        }
        if (item["fld"].as<String>() == "P_AC")
        {
            iv->modules[channel].P_AC = item["val"].as<float>();
        }
        i++;
        if (i == 6)
        {
            channel++;
        }
    }
    // int channels = source[0]["channels"].as<unsigned long>();
    // iv->name = source[0]["name"].as<String>();
    /*
    iv->modules[0].name = source[0]["ch_names"][0].as<String>();
    iv->modules[0].U_AC = source[0]["ch"][0][0].as<float>();
    iv->modules[0].I_AC = source[0]["ch"][0][1].as<float>();
    iv->modules[0].P_AC = source[0]["ch"][0][2].as<float>();
    iv->modules[0].Freq = source[0]["ch"][0][3].as<float>();
    iv->modules[0].Temp = source[0]["ch"][0][5].as<float>();
    iv->modules[0].YieldTotal = source[0]["ch"][0][6].as<float>();
    iv->modules[0].YieldDay = source[0]["ch"][0][7].as<float>();
    iv->modules[0].Efficiency = source[0]["ch"][0][8].as<float>();

    for(int i = 1; i <=channels; i++)
    {
        iv->modules[0].name = source[0]["ch_names"][i].as<String>();
        iv->modules[i].U_DC = source[0]["ch"][i][0].as<float>();
        iv->modules[i].I_DC = source[0]["ch"][i][1].as<float>();
        iv->modules[i].P_DC = source[0]["ch"][i][2].as<float>();
        iv->modules[i].YieldDay = source[0]["ch"][i][3].as<float>();
        iv->modules[i].YieldTotal = source[0]["ch"][i][4].as<float>();
        iv->modules[i].Irradiation = source[0]["ch"][i][5].as<float>();
    }
    */
    iv->available[iv->tm_lastupdate.tm_hour] = true;
    iv->recieve_count[iv->tm_lastupdate.tm_hour]++;
    /*
    Serial.println(iv->modules[0].name);
    Serial.println(iv->modules[0].P_DC);
    Serial.println(iv->modules[1].name);
    Serial.println(iv->modules[1].P_DC);
    Serial.println(iv->modules[2].name);
    Serial.println(iv->modules[2].P_DC);
    */
    this->recalcInverterAverage(iv);
}

JsonDocument dtu::makequerry(String record)
{
    String url;
    int returnCode;
    WiFiClient calClient; // or WiFiClientSecure for HTTPS
    HTTPClient http;
    http.useHTTP10(true);
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    JsonDocument doc_error;
    doc_error["error"] = 1;
    // Serial.println(url);
    //  WiFiClientSecure httpsecureClient;

    // Fetching  data
    url = this->buildURL(record);
    // Serial.println(url);
    if (!http.begin(url))
    {
        http.end();
        // Serial.println("IV: failed begin http");
        return doc_error;
    }

    returnCode = http.GET();

    if (returnCode != 200)
    {
        Serial.println("IV: failed to get from http");
        return doc_error;
    }
    JsonDocument doc_querry;

    DeserializationError error = deserializeJson(doc_querry, http.getStream());
    // serializeJsonPretty(doc_querry, Serial);
    http.end();
    if (error.code() != DeserializationError::Ok)
    {
        // Serial.println("IV: UPDATE FAILED");
        // Serial.println("Capacity of json object:");
        // Serial.println(doc.capacity());
        // Serial.println(error.c_str());
        // Serial.println(error.code());
        return doc_error;
    }
    else
    {
        return doc_querry;
    }
}

void dtu::fetch_data(inverter_vals *iv)
{
    JsonDocument doc_data;
    doc_data = this->makequerry("inverter/id/0");
    if (doc_data.containsKey("error"))
    {
        iv->state = DTU_UPDATE_FAILED;
        return;
    }

    time_t ts_last_success = doc_data["ts_last_success"].as<unsigned long>();
    // Serial.println(ts_last_success);

    if (ts_last_success == iv->lastupdate && ts_last_success != 0)
    {
        // Serial.println("IV: NO UPDATE");
        // Serial.println(ts_last_success);
        // Serial.println(iv->lastupdate);
        iv->state = DTU_NOTHING;
        return;
    }
    // Somthing new is available... Fetch it!

    recalcInverterAverage(iv);
    this->extractData(doc_data.as<JsonObject>(), iv);

    iv->state = DTU_UPDATE_AVAILABLE;
    return;
}

void dtu::loop(inverter_vals *iv)
{

    if (!_init_complete)
    {
        // Serial.println("IV: not init");
        iv->state = DTU_INIT_INCOMPLETE;
        return;
    }

    if (iv->state == DTU_UPDATE_AVAILABLE)
    {
        return; // Return as update was not processed
    }

    time_t now;
    time(&now);
    tm timeinfo;
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_mday != iv->lastreset)
    {
        this->resetaverage(iv, timeinfo.tm_mday);
    }
    if (now < (lastpoll + _update_time))
    {
        iv->state = DTU_NOTHING;
        return;
    }
    Serial.println("IV: Polling data");

    fetch_data(iv);
    lastpoll = now;
    if (iv->state == DTU_UPDATE_FAILED)
    {
        Serial.println("IV: POLLING failed");
        // mqttsend("test", "debug");
    }
    else if (iv->state == DTU_NOTHING)
    {
        Serial.println("IV: NOTHING");
    }
    else if (iv->state == DTU_UPDATE_AVAILABLE)
    {
        Serial.println("IV: UPDATE AVAILABLE");
    }

    return;
};