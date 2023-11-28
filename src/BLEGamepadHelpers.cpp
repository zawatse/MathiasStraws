#include "BLEGamepadHelpers.h"

void ProcessJoystickVals(BleGamepad &gamepad, int RH, int RV, int LH, int LV, bool RON, bool LON)
{
    int potValueRH = 0;
    int potValueRV = 0;
    int potValueLH = 0;
    int potValueLV = 0;
    for (int i = 0; i<numberOfPotSamples; i++)
    {
        potValueRH += analogRead(RH);
        potValueRV += analogRead(RV);
        potValueLH += analogRead(LH);
        potValueLV += analogRead(LV);
        delay(delayBetweenSamples);
    }
    potValueRH = potValueRH / numberOfPotSamples;
    potValueRV = potValueRV / numberOfPotSamples;
    potValueLH = potValueLH / numberOfPotSamples;
    potValueLV = potValueLV / numberOfPotSamples;
    int adjustedValueRH = map(potValueRH, 0, 4095, 32767, 0);
    int adjustedValueRV = map(potValueRV, 0, 4095, 32767, 0);
    int adjustedValueLH = map(potValueLH, 0, 4095, 32767, 0);
    int adjustedValueLV = map(potValueLV, 0, 4095, 32767, 0);

    gamepad.setAxes(adjustedValueLH, adjustedValueLV, 0, 0, adjustedValueRH, adjustedValueRV, DPAD_CENTERED);
}
