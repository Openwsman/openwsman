
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/types.h>

#include <assert.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <net/if_arp.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <string.h>


#include "u/uuid.h"

static long mac_addr_sys ( u_char *addr)
{
    struct ifreq ifr;
    struct ifreq *IFR;
    struct ifconf ifc;
    char buf[1024];
    int s, i;
    int ok = 0;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s==-1) {
        return -1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    ioctl(s, SIOCGIFCONF, &ifc);

    IFR = ifc.ifc_req;
    for (i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; IFR++) {

        strcpy(ifr.ifr_name, IFR->ifr_name);
        if (ioctl(s, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) {
                if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) {
                    ok = 1;
                    break;
                }
            }
        }
    }

    close(s);
    if (ok) {
        bcopy( ifr.ifr_hwaddr.sa_data, addr, 6);
    }
    else {
        return -1;
    }
    return 0;
}

int generate_uuid (char* buf, int size, int bNoPrefix) 
{

    static int clock_sequence = 1;
    u_int64_t longTimeVal;
    int timeLow = 0;
    int timeMid = 0;
    int timeHigh = 0;
    unsigned char mac[6];
    unsigned char uuid[16];
    int i;
    char* ptr = buf;
    struct timeval tv;
    int max_length = SIZE_OF_UUID_STRING;

    if ( !bNoPrefix ) max_length += 5;      // space for "uuid:"
    if ( size < max_length )
        return 0;

    if ( buf == NULL )
        return 0;

    // get time data
    gettimeofday( &tv, NULL );
    longTimeVal = (u_int64_t)1000000 * (u_int64_t)tv.tv_sec + (u_int64_t)tv.tv_usec;
    timeLow = longTimeVal & 0xffffffff;     // lower 32 bits
    timeMid = (longTimeVal >> 32) & 0xffff; // middle 16 bits
    timeHigh = (longTimeVal >> 32) & 0xfff; // upper 12 bits

    // update clock sequence number
    clock_sequence++;

    // get mac address
    if ( mac_addr_sys( mac ) < 0 )
        return 0;

    // now assemble UUID from fragments above
    for( i = 0; i < 6; i++ )
        uuid[i] = mac[i];                       // mac address

    uuid[6] = clock_sequence & 0xff;                // clock seq. low
    uuid[7] = 0x80 | ((clock_sequence >> 8) & 0x3f);// clock seq. high and variant
    uuid[8] = timeHigh & 0xff;                      // time high/version low bits
    uuid[9] = timeHigh >> 8 | 0x10;                 // time high/version high bits
    *(short int*)(uuid+10) = (short int)timeMid;
    *(int*)(uuid+12) = timeLow;

    // sha1 the uuid above, to mix things up
    //icrypto_sha1_calc( uuid, 16, sha1uuid );

    // convert these bytes into chars at output


    if ( !bNoPrefix )
    {
        sprintf( ptr, "uuid:" );
        ptr += 5;
    }

    sprintf( ptr, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid[15], uuid[14], uuid[13], uuid[12],
            uuid[11], uuid[10], uuid[9], uuid[8],
            uuid[7], uuid[6], uuid[5], uuid[4],
            uuid[3], uuid[2], uuid[1], uuid[0] );

    return 1;

}
