#include "Preferences.h"
#include "src/device/device.h"

Preferences preferences;

void setDeviceNum(int id){
    preferences.putInt("deviceNum",id);
}


int getDeviceNum(){
    return preferences.getInt("deviceNum",0);
}

void setDisabled(bool val){
    disabled = val;
}
