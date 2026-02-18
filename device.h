#ifndef DEVICE_H
#define DEVICE_H

#define WIFI false 


#include "Preferences.h"

extern Preferences preferences;

void setDisabled(bool val);

void setDeviceID(int id);
int getDeviceID();
extern int deviceID;

void captivePortalLoop();//void *pvParameters);
void captivePortalSetup();


#endif
