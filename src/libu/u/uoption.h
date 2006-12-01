/*******************************************************************************
* Copyright (C) 2006 Intel Corp. All rights reserved.
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
 * @author Denis Sadykov
 */

#ifndef UOPTION_H
#define UOPTION_H



#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------------
			Entry
----------------------------------------------------------------------------- */

enum {
	U_OPTION_ARG_NONE = 0,
	U_OPTION_ARG_INT,
	U_OPTION_ARG_STRING,
	U_OPTION_ARG_STRING_ARRAY,
};

typedef struct u_option_entry {
	const char		*name;
	char			short_name;
	int			arg;
	void			*arg_data;
	const char		*descr;
	const char		*arg_descr;
} u_option_entry_t;

/* ----------------------------------------------------------------------------
			Group
----------------------------------------------------------------------------- */

typedef struct u_option_group {
	char			*name;
	char			*descr;
	char			*help_descr;
	u_option_entry_t	*entries;
	unsigned int		num_entries;
	char			ismain;
} u_option_group_t;

u_option_group_t* u_option_group_new(const char *name,
				     const char *descr,
				     const char *help_descr);
void u_option_group_free(u_option_group_t *group);
void u_option_group_add_entries(u_option_group_t *group,
				u_option_entry_t *entries);

/* ----------------------------------------------------------------------------
			Context
----------------------------------------------------------------------------- */

#define U_OPTION_CONTEXT_IGNORE_UNKNOWN		0x1
#define U_OPTION_CONTEXT_HELP_ENABLED		0x2

typedef struct u_option_context {
	char			*usage;
	list_t			*groups;
	unsigned int		mode;
	char			*prog_name;
} u_option_context_t;

u_option_context_t* u_option_context_new(const char *usage);
void u_option_context_free(u_option_context_t *ctx);
void u_option_context_add_group(u_option_context_t *ctx,
				u_option_group_t *group);
void u_option_context_add_main_entries(u_option_context_t *ctx,
				       u_option_entry_t *options,
				       const char *name);
void u_option_context_set_ignore_unknown_options(u_option_context_t *ctx,
						 char ignore);
void u_option_context_set_help_enabled(u_option_context_t *ctx,
					char help_enabled);

char u_option_context_parse(u_option_context_t *ctx,
			    int *argc,
			    char ***argv,
			    u_error_t **error);

#ifdef __cplusplus
}
#endif

#endif /* UOPTION_H */
