/*
 * gen_uuid_nt.c -- Use NT api to generate uuid
 *
 * Written by Andrey Shedel (andreys@ns.cr.cyco.com)
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 *
 * %Begin-Header%
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * %End-Header%
 */

#ifdef WIN32

#include "uuidP.h"

#pragma warning(push,4)

#pragma comment(lib, "ntdll.lib")

//
// Here is a nice example why it's not a good idea
// to use native API in ordinary applications.
// Number of parameters in function below was changed from 3 to 4
// for NT5.
//
//
// NTSYSAPI
// NTSTATUS
// NTAPI
// NtAllocateUuids(
//     OUT PULONG p1,
//     OUT PULONG p2,
//     OUT PULONG p3,
//     OUT PUCHAR Seed // 6 bytes
//   );
//
//

unsigned long
__stdcall
NtAllocateUuids(
   void* p1,  // 8 bytes
   void* p2,  // 4 bytes
   void* p3   // 4 bytes
   );

typedef
unsigned long
(__stdcall*
NtAllocateUuids_2000)(
   void* p1,  // 8 bytes
   void* p2,  // 4 bytes
   void* p3,  // 4 bytes
   void* seed // 6 bytes
   );



//
// Nice, but instead of including ntddk.h ot winnt.h
// I should define it here because they MISSED __stdcall in those headers.
//

__declspec(dllimport)
struct _TEB*
__stdcall
NtCurrentTeb(void);


//
// The only way to get version information from the system is to examine
// one stored in PEB. But it's pretty dangerouse because this value could
// be altered in image header.
//

static
int
Nt5(void)
{
	//return NtCuttentTeb()->Peb->OSMajorVersion >= 5;
	return (int)*(int*)((char*)(int)(*(int*)((char*)NtCurrentTeb() + 0x30)) + 0xA4) >= 5;
}



/* Use the native Windows UUID generation facilities. */
void uuid_generate(uuid_t out)
{
	if(Nt5())
	{
		unsigned char seed[6];
		((NtAllocateUuids_2000)NtAllocateUuids)(out, ((char*)out)+8, ((char*)out)+12, &seed[0] );
	}
	else
	{
		NtAllocateUuids(out, ((char*)out)+8, ((char*)out)+12);
	}
}


/* The following two functions exist to provide compatibility. */
void uuid_generate_random(uuid_t out)
{
	uuid_generate(out);
}

void uuid_generate_time(uuid_t out)
{
	uuid_generate(out);
}

#endif /* WIN32 */
