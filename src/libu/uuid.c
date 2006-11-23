



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

#include <sys/time.h>
#include "u/uuid.h"



/* return the Media Access Control (MAC) address of
   the FIRST network interface card (NIC) */
static int mac_address(unsigned char *data_ptr, size_t data_len)
{
    /* sanity check arguments */
    if (data_ptr == NULL || data_len < MAC_LEN)
        return 0;

#if defined(HAVE_IFADDRS_H) && defined(HAVE_NET_IF_DL_H) && defined(HAVE_GETIFADDRS)
    /* use getifaddrs(3) on BSD class platforms (xxxBSD, MacOS X, etc) */
    {
        struct ifaddrs *ifap;
        struct ifaddrs *ifap_head;
        const struct sockaddr_dl *sdl;
        unsigned char *ucp;
        int i;

        if (getifaddrs(&ifap_head) < 0)
            return 0;
        for (ifap = ifap_head; ifap != NULL; ifap = ifap->ifa_next) {
            if (ifap->ifa_addr != NULL && ifap->ifa_addr->sa_family == AF_LINK) {
                sdl = (const struct sockaddr_dl *)(void *)ifap->ifa_addr;
                ucp = (unsigned char *)(sdl->sdl_data + sdl->sdl_nlen);
                if (sdl->sdl_alen > 0) {
                    for (i = 0; i < MAC_LEN && i < sdl->sdl_alen; i++, ucp++)
                        data_ptr[i] = (unsigned char)(*ucp & 0xff);
                    freeifaddrs(ifap_head);
                    return 1;
                }
            }
        }
        freeifaddrs(ifap_head);
    }
#endif

#if defined(HAVE_NET_IF_H) && defined(SIOCGIFHWADDR)
    /* use SIOCGIFHWADDR ioctl(2) on Linux class platforms */
    {
        struct ifreq ifr;
        struct sockaddr *sa;
        int s;
        int i;

        if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
            return FALSE;
        sprintf(ifr.ifr_name, "eth0");
        if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
            close(s);
            return 0;
        }
        sa = (struct sockaddr *)&ifr.ifr_addr;
        for (i = 0; i < MAC_LEN; i++)
            data_ptr[i] = (unsigned char)(sa->sa_data[i] & 0xff);
        close(s);
        return 1;
    }
#endif

#if defined(SIOCGARP)
    /* use SIOCGARP ioctl(2) on SVR4 class platforms (Solaris, etc) */
    {
        char hostname[MAXHOSTNAMELEN];
        struct hostent *he;
        struct arpreq ar;
        struct sockaddr_in *sa;
        int s;
        int i;

        if (gethostname(hostname, sizeof(hostname)) < 0)
            return 0;
        if ((he = gethostbyname(hostname)) == NULL)
            return 0;
        if ((s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
            return 0;
        memset(&ar, 0, sizeof(ar));
        sa = (struct sockaddr_in *)((void *)&(ar.arp_pa));
        sa->sin_family = AF_INET;
        memcpy(&(sa->sin_addr), *(he->h_addr_list), sizeof(struct in_addr));
        if (ioctl(s, SIOCGARP, &ar) < 0) {
            close(s);
            return 0;
        }
        close(s);
        if (!(ar.arp_flags & ATF_COM))
            return 0;
        for (i = 0; i < MAC_LEN; i++)
            data_ptr[i] = (unsigned char)(ar.arp_ha.sa_data[i] & 0xff);
        return 1;
    }
#endif

    return 0;
}







#if 0

static long
mac_addr_sys (u_char *addr)
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

#endif

int 
generate_uuid ( char* buf, 
                int size, 
                int no_prefix) 
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

    if ( !no_prefix ) max_length += 5;      // space for "uuid:"
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
    if ( mac_address( mac, 6 ) == 0 )
    {
        for( i = 0; i < 6; i++ )
            uuid[i] = mac[i];                       // mac address
    } else {
        for( i = 0; i < 6; i++ )
            uuid[i] = clock_sequence & 0xff;        // just in case we do not have a mac
    }

    uuid[6] = clock_sequence & 0xff;                // clock seq. low
    uuid[7] = 0x80 | ((clock_sequence >> 8) & 0x3f);// clock seq. high and variant
    uuid[8] = timeHigh & 0xff;                      // time high/version low bits
    uuid[9] = timeHigh >> 8 | 0x10;                 // time high/version high bits
    *(short int*)(uuid+10) = (short int)timeMid;
    *(int*)(uuid+12) = timeLow;

    if ( !no_prefix )
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
