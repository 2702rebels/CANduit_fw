#ifndef DEVICE_H
#define DEVICE_H 

#include "Preferences.h"

extern bool disabled;
extern Preferences preferences;

void setDisabled(bool val);

void setDeviceNum(int id);
int getDeviceNum();

#endif
