#ifndef MANAGEMENT_H
#define MANAGEMENT_H

struct EndpointManager
{
    // homehub endpoints
    String homehubCreate;
    String homehubClimate;
    String homehubActivity;

    // tank endpoints
    String tankCreate;
    String tankData;

    // quality endpoints
    String qualityCreate;
    String qualityData;

    // bucket endpoints
    String bucketCreate;
    String bucketData;
}endpointManager;


void initEndpoints()
{
    // initializing default homehub endpoints
    endpointManager.homehubCreate = "/api/homehub";
    endpointManager.homehubClimate = "/api/homehub/weather";
    endpointManager.homehubActivity = "/api/homehub/activity";
    
    // initializing default tank endpoints
    endpointManager.tankCreate = "/api/sensor/tank";
    endpointManager.tankData = "/api/sensor/tankData";
    
    // initializing default quality endpoints
    endpointManager.qualityCreate = "/api/sensor/quality";
    endpointManager.qualityData = "/api/sensor/qualityData";
}

void setDebugEndpoints()
{
    // setting debug homehub endpoints
    endpointManager.homehubCreate = "/api/debug/homehub";
    endpointManager.homehubClimate = "/api/debug/homehub/weather";
    endpointManager.homehubActivity = "/api/debug/homehub/activity";
    
    // setting debug tank endpoints
    endpointManager.tankCreate = "/api/debug/sensor/tank";
    endpointManager.tankData = "/api/debug/sensor/tankData";
    
    // setting debug quality endpoints
    endpointManager.qualityCreate = "/api/debug/sensor/quality";
    endpointManager.qualityData = "/api/debug/sensor/qualityData";
    
    // setting debug bucket endpoints
    endpointManager.bucketCreate = "/api/debug/sensor/bucket";
    endpointManager.bucketData = "/api/debug/sensor/bucketData";
}

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
    // // // char ssid[32];
    char mac_addr[18];
} pairing_data;

struct pairing_data pairingData;

#endif // MANAGEMENT_H