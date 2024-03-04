#include "BLEGamepadHelpers.h"

void ProcessJoystickVals(BleGamepad &gamepad, int RH, int RV, int LH, int LV, int RHIn, int RVIn, int LHIn, int LVIn, bool RON, bool LON)
{
    auto [potValueRH, potValueRV, potValueLH, potValueLV] = ReadJoystickVals(RH, RV, LH, LV, RON, LON);
    auto [adjustedValueRH, adjustedValueRV, adjustedValueLH, adjustedValueLV] = AdjustJoystickVals(potValueRH, potValueRV, potValueLH, potValueLV, RHIn, RVIn, LHIn, LVIn);

    gamepad.setAxes(adjustedValueLH, adjustedValueLV, 0, 0, adjustedValueRH, adjustedValueRV, DPAD_CENTERED);
}

std::tuple<int,int,int,int> ReadJoystickVals(int RH, int RV, int LH, int LV, bool RON, bool LON)
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

    if(!RON)
    {
        potValueRH = 2048;
        potValueRV = 2048;
    }

    if(!LON)
    {
        potValueLH = 2048;
        potValueLV = 2048;
    }
    return {potValueRH, potValueRV, potValueLH, potValueLV};
}

std::tuple<int,int,int,int> AdjustJoystickVals(int RH, int RV, int LH, int LV, int RHIn, int RVIn, int LHIn, int LVIn)
{
    int adjustedValueRH = map(AccountForOffset(RH,RHIn), 0, 4095, 32767, 0);
    int adjustedValueRV = map(AccountForOffset(RV,RVIn), 0, 4095, 32767, 0);
    int adjustedValueLH = map(AccountForOffset(LH,LHIn), 0, 4095, 32767, 0);
    int adjustedValueLV = map(AccountForOffset(LV,LVIn), 0, 4095, 32767, 0);
    return {adjustedValueRH, adjustedValueRV, adjustedValueLH, adjustedValueLV};
}

int AccountForOffset(int value, int initial_position)
{
    if (value > initial_position)
    {
        return map(value, initial_position+1, 4095, 2048, 4095);
    }
    return map(value, 0, initial_position, 0, 2047);
}

