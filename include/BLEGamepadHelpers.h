#ifndef _BLEGAMEPADHELPERS_H_
#define _BLEGAMEPADHELPERS_H_
#include <BleGamepad.h>
#include <tuple>

const int numberOfPotSamples = 5;     // Number of pot samples to take (to smooth the values)
const int delayBetweenSamples = 2;    // Delay in milliseconds between pot samples

void ProcessJoystickVals(BleGamepad &gamepad, int RH, int RV, int LH, int LV, int RHIn, int RVIn, int LHIn, int LVIn, bool RON, bool LON);
std::tuple<int,int,int,int> ReadJoystickVals(int RH, int RV, int LH, int LV, bool RON, bool LON);
std::tuple<int,int,int,int> AdjustJoystickVals(int RH, int RV, int LH, int LV, int RHIn, int RVIn, int LHIn, int LVIn);
int AccountForOffset(int value, int initial_position);

#endif /* _BLEGAMEPADHELPERS_H_ */