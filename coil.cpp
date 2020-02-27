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
 if (coil[0]&0x01)
 {
 COIL1_ON;
 }
 else
 {
 COIL1_OFF;
 }
 if (coil[0]&0x01)
 {
 COIL1_ON;
 }
 else
 {
 COIL1_OFF;
 }
 if (coil[0]&0x01)
 {
 COIL1_ON;
 }
 else
 {
 COIL1_OFF;
 }
 if (coil[0]&0x01)
 {
 COIL1_ON;
 }
 else
 {
 COIL1_OFF;
 }

}
