/*******************************************************************************
* Copyright (C) 2004-2006 Intel Corp. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Intel Corp. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/**
 * @author Anas Nashif
 */

/*
 * **  OSSP uuid - Universally Unique Identifier
 * **  Copyright (c) 2004-2005 Ralf S. Engelschall <rse@engelschall.com>
 * **  Copyright (c) 2004-2005 The OSSP Project <http://www.ossp.org/>
 * **
 * **  This file is part of OSSP uuid, a library for the generation
 * **  of UUIDs which can found at http://www.ossp.org/pkg/lib/uuid/
 * **
 * **  Permission to use, copy, modify, and distribute this software for
 * **  any purpose with or without fee is hereby granted, provided that
 * **  the above copyright notice and this permission notice appear in all
 * **  copies.
 * **
 * **  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * **  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * **  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * **  IN NO EVENT SHALL THE AUTHORS AND COPYRIGHT HOLDERS AND THEIR
 * **  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * **  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * **  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * **  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * **  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * **  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * **  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * **  SUCH DAMAGE.
 * **
 * **  uuid_mac.c: Media Access Control (MAC) resolver implementation
 * */


#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif


#include <stdlib.h>
#include <stdio.h>


#if HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/types.h>

#include <assert.h>
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#include <string.h>

#if (defined (__SVR4) && defined (__sun))
#include <uuid/uuid.h>
#endif

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#endif
#include "u/uuid.h"
#include "u/os.h"
#include "u/debug.h"

#ifdef WIN32
#include "windows.h"
#include "rpcdce.h"
/* don't forget to link with rpcrt4.lib */
int 
generate_uuid(char *buf, int size, int bNoPrefix)
{
	int             ret_val = -1;
	UUID            uuid;
	RPC_STATUS      st = UuidCreate(&uuid);

	if (st == RPC_S_OK || st == RPC_S_UUID_LOCAL_ONLY) {
		char           *str = NULL;
		if ((st = UuidToString(&uuid, (BYTE **) & str)) != RPC_S_OK) {
			//failed
		} else {
			int             len = 5;
			if (!bNoPrefix && size > (len + 1))
				strcpy(buf, "uuid:");
			else
				len = 0;
			strncpy(buf +len, str, size - len);
			RpcStringFree((BYTE **) & str);
			ret_val = 0;
		}
	} else {
		//failed
	}
	return ret_val;
}


#else
#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/utsname.h>
#include <sys/param.h>
#include <netinet/in.h>

#if defined(__APPLE__)  ||  defined(__FreeBSD__)
#include <net/bpf.h>
#include <net/if_dl.h>
#include <net/if_types.h>

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





#elif !(defined (__SVR4) && defined (__sun))

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

#if !(defined (__SVR4) && defined (__sun))

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
#if defined(__APPLE__)  ||  defined(__FreeBSD__)
    if ( mac_address( mac, 6 ) == 0 )
#else    
    if (mac_addr_sys(mac) == 0 )
#endif    
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

#else /* Solaris */
int
generate_uuid ( char* buf,
                int size,
                int no_prefix)
{
    int max_length = SIZE_OF_UUID_STRING;
    char* ptr = buf;
	char uuid_str[UUID_PRINTABLE_STRING_LENGTH];
	uuid_t uuid;

    if ( !no_prefix ) max_length += 5;      // space for "uuid:"
    if ( size < max_length )
        return 0;

    if ( buf == NULL )
        return 0;

    if ( !no_prefix )
    {
        sprintf( ptr, "uuid:" );
        ptr += 5;
    }

	uuid_generate(uuid);
	uuid_unparse(uuid, uuid_str);

	int uuidlen = strlen(uuid_str);
	if (((ptr - buf) + uuidlen) < size) {
		strlcpy(ptr, uuid_str, uuidlen);
		return 1;
	}
	else
		return 0;

}

#endif

#endif /* #if WIN32 ... #else */
