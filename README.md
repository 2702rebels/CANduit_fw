# CANduit_fw
Firmware for the ESP32 on the CANduit

Merge files (one function per file is too small)  
Two files called pwm.cpp causes confusion

Arduino IDE only shows the .ino file.  Other source files should be moved to the same folder as the .ino
so they appear in the IDE

Once the files are consolodated, clear file names should allow all to be moved to a single folder with the .ino

Use clearer code to call the reading/writing function to avoid confustion - just a comment and an array name 
like readFuncArray and writeFuncArray.
