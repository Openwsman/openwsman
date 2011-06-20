/*******************************************************************************
* Copyright (C) 2004-2007 Intel Corp. All rights reserved.
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
 * @author Liang Hou
 */

#ifndef WSMAN_SUBSCRIPTION_REPOSITORY_H_
#define WSMAN_SUBSCRIPTION_REPOSITORY_H_

#include "u/list.h"

typedef int (*SubscriptionOpInit) (char *, void *);
typedef int (*SubscriptionOpFinalize) (char *, void *);
typedef int (*SubscriptionOpSave) (char *, char *, unsigned char *);
typedef int (*SubscriptionOpDelete) (char *, char *);
typedef int (*SubscriptionOpGet) (char *, char *, unsigned char **, int *);
typedef int (*SubscriptionOpSearch) (char *, char *);
typedef int (*SubscriptionOpUpdate) (char *, char *, char *);
typedef int (*SubscriptionOpLoad) (char *, list_t *);

struct __SubsRepositoryEntry {
        unsigned char *strdoc;
	int len;
        char *uuid;
};
typedef struct __SubsRepositoryEntry *SubsRepositoryEntryH;

struct __SubsRepositoryOpSet{
        SubscriptionOpInit init_subscription; //entry of initializing subscripton repository
        SubscriptionOpFinalize finalize_subscription; //entry of finalizing subscription repository
        SubscriptionOpLoad load_subscription; //entry of loading all of subscriptions
        SubscriptionOpGet get_subscription; //entry of geting a subscription from the repository
        SubscriptionOpSearch search_subscription; //entry of searching a subscription from the repository
        SubscriptionOpSave save_subscritption; //entry of saving a subscription in the repository
        SubscriptionOpUpdate update_subscription; //entry of updating a subscription
        SubscriptionOpDelete delete_subscription; //entry of deleting a subscription from the repository
};
typedef struct __SubsRepositoryOpSet *SubsRepositoryOpSetH;

SubsRepositoryOpSetH wsman_get_subsrepos_opset(void);

#endif
