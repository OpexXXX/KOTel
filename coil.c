#include "coil.h"

void update_coil_state(unsigned char coil[])
{
    if (coil[0]&0x01)
    {
        COIL1_ON;
    }
    else
    {
        COIL1_OFF;
    }
    if (coil[0]&0x02)
    {
        COIL2_ON;
    }
    else
    {
        COIL2_OFF;
    }
    if (coil[0]&0x04)
    {
        COIL3_ON;
    }
    else
    {
        COIL3_OFF;
    }
    if (coil[0]&0x08)
    {
        COIL4_ON;
    }
    else
    {
        COIL4_OFF;
    }
    if (coil[0]&0x10)
    {
        COIL5_ON;
    }
    else
    {
        COIL5_OFF;
    }
        if (coil[0]&0x20)
    {
        COIL6_ON;
    }
    else
    {
        COIL6_OFF;
    }

}
