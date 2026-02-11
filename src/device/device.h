#ifndef DEVICE_H
#define DEVICE_H

#define WIFI true


#include "Preferences.h"

extern bool disabled;
extern Preferences preferences;

void setDisabled(bool val);

void setDeviceID(int id);
int getDeviceID();
int deviceID;


#endif
