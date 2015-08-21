////////////////////////////////////////////////////////////////////
// PSP Network Library Helpers
//  by PspPet

////////////////////////////////////////////////////////////////////
// Reverse engineering source material:
// "fair use" for making compatible software.
// most call sequences and initialization values taken from Wipeout Pure game
//   Wipeout has both ad-hoc game sharing and Apctl init for web access
//   also useful resources:
//     Twisted Metal game
//       (also uses Apctl, in a slightly different way)
//     KD modules "pspnet_apctl.prx" and "pspnet_ap_dialog_dummy.prx"
//       (especially for NetParam values)
//   Standard Berkeley socket socket.h header (or the MS winsock.h)
//
////////////////////////////////////////////////////////////////////

#include "std.h"
#include "nlh.h"

unsigned short htons(unsigned short wIn)
{
    u8 bHi = (wIn >> 8) & 0xFF;
    u8 bLo = wIn & 0xFF;
    return ((unsigned short)bLo << 8) | bHi;
}
