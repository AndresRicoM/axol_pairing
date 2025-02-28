#ifndef MANAGEMENT_H
#define MANAGEMENT_H

struct WaterManager
{
    // Water Management Variables
    int buckets, tanks, quality, envs, avail_storage, avail_liters;
    float fill_percentage;
    const char *dev_name;
} waterManager;

struct EventVariables
{
    // Timed Event Variables - used to send
    long current_time, elapsed_time, sent_time;
    bool sending_climate = true;
    bool sending_activity = false;
    int activity;
    
} eventVariables;

// ESP Now Communication Variables
typedef struct struct_message
{
    char id[50];
    int type;
    float data1;
    float data2;
    float data3;
    float data4;
    float data6;
    float data7;

    char ssid[32];
    char mac_addr[18];

} struct_message;

struct_message myData;

typedef struct pairing_data
{
    char ssid[32];
    char mac_addr[18];
} pairing_data;

struct pairing_data pairingData;

#endif // MANAGEMENT_H