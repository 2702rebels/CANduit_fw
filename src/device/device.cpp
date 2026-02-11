#include "Preferences.h"
#include "src/device/device.h"


Preferences preferences;

void setDeviceID(int id){
    preferences.begin("canduit",false);
    preferences.putInt("deviceNum",id);
    deviceID = id;
    preferences.end();
}


int getDeviceID(){
    preferences.begin("canduit",false);
    int deviceNum = preferences.getInt("deviceNum",0);
    
    preferences.end();
    return deviceNum;
}

void setDisabled(bool val){
    disabled = val;
}
