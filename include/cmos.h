#ifndef CMOS_H
#define CMOS_H

#include <types.h>

class CMOS
{
public:
    static byte Read(byte reg);
    static void Write(byte reg, byte val);
    static bool DisableNMI();
    static void RestoreNMI(bool state);
    static uint GetIRQ();
    static void EnableTimer();
    static void DisableTimer();
    static int GetTimerFrequency();
    static void SetTimerDivider(int divider);
};

#endif // CMOS_H
