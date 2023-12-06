#ifndef _BLEGAMEPADHELPERS_H_
#define _BLEGAMEPADHELPERS_H_
#include <BleGamepad.h>

const int numberOfPotSamples = 5;     // Number of pot samples to take (to smooth the values)
const int delayBetweenSamples = 2;    // Delay in milliseconds between pot samples

void ProcessJoystickVals(BleGamepad &gamepad, int RH, int RV, int LH, int LV, bool RON, bool LON);

#endif /* _BLEGAMEPADHELPERS_H_ */