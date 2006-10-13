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

#include "u/libu.h"

/* ----------------------------------------------------------------------------
			Misc
----------------------------------------------------------------------------- */

struct tmp_buf {
	long int		lint;
	char			*ptr;
	char			**pptr;
	u_option_entry_t	*entry;
};

static unsigned int context_get_number_entries(u_option_context_t *ctx)
{
	u_list_t		*list;
	u_option_group_t	*grp;
	unsigned int		num_entr = 0;

	if (!ctx) {
		return 0;
	}

	list = ctx->groups;
	while (list) {
		grp = (u_option_group_t *)list->data;
		num_entr += grp->num_entries;
		list = list->next;
	}

	return num_entr;
}

static char fill_tmp_data(struct tmp_buf *data,
			  int *filled,
			  char *optptr,
			  char *argptr,
			  u_option_entry_t *entry,
			  u_error_t **error)
{
	unsigned int	nd;
	unsigned int	count,	val;
	char		**pptr;

	for (nd = 0; nd < *filled; nd++) {
		if (data[nd].entry == entry) {
			break;
		}
	}

	switch(entry->arg) {
	case U_OPTION_ARG_INT:
		if (!isstrdigit(argptr)) {
			u_error_new(error, 0,
				"Cannot parse integer value \'%s\' for -%s\n",
				argptr, optptr);
			return 0;
		}
		val = strtol(argptr, (char **)NULL, 10);
		if (val == LONG_MIN || val == LONG_MAX) {
			u_error_new(error, 0,
				"Integer value \'%s\'for -%s out of range\n",
				argptr, optptr);
			return 0;
		}
		data[nd].lint = val;
		break;
	case U_OPTION_ARG_STRING:
		data[nd].ptr = strdup(argptr);
		break;
	case  U_OPTION_ARG_STRING_ARRAY:
		count = 0;
		pptr = data[nd].pptr;
		while (pptr != NULL && pptr[count] != NULL) {
			++count;
		}
		data[nd].pptr = u_malloc(sizeof(char *) * (count + 2));
		data[nd].pptr[count] = strdup(argptr);
		data[nd].pptr[count + 1] = NULL;
		while (count != 0) {
			count--;
			data[nd].pptr[count] = pptr[count];
		}
		if (pptr)
			u_free(pptr);
		break;
	case U_OPTION_ARG_NONE:
		data[nd].lint = 1;
		break;
	default:
		u_error_new(error, 0, "Unknown argument data type for -%s\n",
									optptr);
		return 0;
	}

	if (nd == *filled) {
		data[nd].entry = entry;
		*filled += 1;
	}

	return 1;
}

static void free_tmp_data(struct tmp_buf *data, int nd)
{
	int	i,	count;

	for (i = 0; i < nd; i++) {
		switch (data[i].entry->arg) {
		case U_OPTION_ARG_STRING:
			if (data[i].ptr) {
				u_free(data[i].ptr);
			}
			break;
		case U_OPTION_ARG_STRING_ARRAY:
			count = 0;
			while (data[i].pptr != NULL &&
						data[i].pptr[count] != NULL) {
				u_free(data[i].pptr[count]);
					count++;
				}
			if (data[i].pptr) {
				u_free(data[i].pptr);
			}
			break;
		}
	}
}

static void get_tmp_data(struct tmp_buf *data, int nd)
{
	int			i;
	u_option_entry_t	*entry;

	for (i = 0; i < nd; i++) {
		entry = data[i].entry;
		switch (entry->arg) {
		case U_OPTION_ARG_INT:
			*(long int *)entry->arg_data = data[i].lint;
			break;
		case U_OPTION_ARG_STRING:
			*(char **)entry->arg_data = data[i].ptr;
			break;
		case U_OPTION_ARG_STRING_ARRAY:
			*(char ***)entry->arg_data = data[i].pptr;
			break;
		case U_OPTION_ARG_NONE:
			*(char *)entry->arg_data = (char)data[i].lint;
		}
	}
}

static u_option_entry_t* find_long_opt(u_option_context_t *ctx, char *option)
{
	u_list_t		*list;
	u_option_group_t	*grp;
	int			e;
	size_t			nlen;

	if (!ctx) {
		return NULL;
	}

	list = ctx->groups;
	while (list) {
		grp = (u_option_group_t *)list->data;

		for (e = 0; e < grp->num_entries; e++) {
			nlen = strlen(grp->entries[e].name);

			if (!strncmp(option, grp->entries[e].name, nlen)) {
				if (strlen(option) != nlen &&
						*(option + nlen) != '=') {
					continue;
				}
				return &(grp->entries[e]);
			}
		}
		list = list->next;
	}

	return NULL;
}

static u_option_entry_t* find_short_opt(u_option_context_t *ctx, char option)
{
	u_list_t		*list;
	u_option_group_t	*grp;
	int			e;

	if (!ctx) {
		return NULL;
	}

	list = ctx->groups;
	while (list) {
		grp = (u_option_group_t *)list->data;

		for (e = 0; e < grp->num_entries; e++) {
			if (option == grp->entries[e].short_name) {
				return &(grp->entries[e]);
			}
		}
		list = list->next;
	}

	return NULL;
}

/* ----------------------------------------------------------------------------
			Group
----------------------------------------------------------------------------- */

u_option_group_t* u_option_group_new(const char *name,
				     const char *descr,
				     const char *help_descr)
{
	u_option_group_t	*grp;

	grp = (u_option_group_t *) u_zalloc(sizeof(u_option_group_t));

	if (name) {
		grp->name = strdup(name);
	}

	if (descr) {
		grp->descr = strdup(descr);
	}

	if (help_descr) {
		grp->help_descr = strdup(help_descr);
	}

	return grp;
}

void u_option_group_free(u_option_group_t *group)
{
	if (!group)
		return;

	if (group->name)
		u_free(group->name);
	if (group->descr)
		u_free(group->descr);
	if(group->help_descr)
		u_free(group->help_descr);
}

void u_option_group_add_entries(u_option_group_t *group,
				u_option_entry_t *entries)
{
	unsigned int	i;

	if (!group || !entries) {
		return;
	}

	for (i = 0; entries[i].name; i++);
	group->entries = entries;
	group->num_entries = i;

}

/* ----------------------------------------------------------------------------
			Context
----------------------------------------------------------------------------- */

u_option_context_t* u_option_context_new(const char *usage)
{
	u_option_context_t	*ctx;

	ctx = (u_option_context_t *) u_zalloc(sizeof(u_option_context_t));

	if (usage) {
		ctx->usage = strdup(usage);
	}

	ctx->mode |= U_OPTION_CONTEXT_HELP_ENABLED;

	return ctx;
}

void u_option_context_free(u_option_context_t *ctx)
{
	u_list_t	*list;

	if (!ctx)
		return;

	list = ctx->groups;
	while (list) {
		u_option_group_free((u_option_group_t *)list->data);
		list = list->next;
	}
	u_list_remove_all(ctx->groups);

	if (ctx->usage)
		u_free(ctx->usage);

	u_free(ctx);
}

void u_option_context_add_group(u_option_context_t *ctx,
				u_option_group_t *group)
{
	if (!ctx || !group)
		return;

	ctx->groups = u_list_append(ctx->groups, (void *)group);
}

void u_option_context_add_main_entries(u_option_context_t *ctx,
				       u_option_entry_t *options,
				       const char *name)
{
	u_option_group_t	*grp;

	if (!ctx || !options) {
		return;
	}

	grp = u_option_group_new(name, NULL, NULL);
	u_option_group_add_entries(grp, options);
	ctx->groups = u_list_prepend(ctx->groups, (void *)grp);
}

void u_option_context_set_ignore_unknown_options(u_option_context_t *ctx,
						 char ignore)
{
	if (!ctx)
		return;

	if (ignore == 1)
		ctx->mode |= U_OPTION_CONTEXT_IGNORE_UNKNOWN;
	else
		ctx->mode &= ~U_OPTION_CONTEXT_IGNORE_UNKNOWN;

}

void u_option_context_set_help_enabled(u_option_context_t *ctx,
					char help_enabled)
{
	if (!ctx)
		return;

	if (help_enabled == 1)
		ctx->mode |= U_OPTION_CONTEXT_HELP_ENABLED;
	else
		ctx->mode &= ~U_OPTION_CONTEXT_HELP_ENABLED;

}

/* ----------------------------------------------------------------------------
			Parse
----------------------------------------------------------------------------- */

char u_option_context_parse(u_option_context_t *ctx,
			    int *argc,
			    char ***argv,
			    u_error_t **error)
{
	char		*tmp_argv[*argc],	*largv[*argc];
	int		arg_ind = 0,		noopt = 0;
	char		*optptr = NULL,		*argptr = NULL;
	char		next_shopt = '\0';
	int		nd = 0;
	size_t		nlen;
	int		i,	j;

	u_option_entry_t	*found;
	struct tmp_buf		*tmp_data;

	if (!ctx)
		return 0;

	for (i = 1; i < *argc; i++) {
		largv[i] = strdup((*argv)[i]);
	}

	tmp_data = u_zalloc(sizeof(struct tmp_buf) *
					context_get_number_entries(ctx));

	for(i = 1; i < *argc;) {
		if (*largv[i] != '-') {
			tmp_argv[noopt] = largv[i];
			noopt++;
			goto cont;
		}

		argptr = NULL;
		if (next_shopt != '\0') {
			optptr += 1;
			*optptr = next_shopt;
			next_shopt = '\0';
		} else {
			optptr = largv[i] + 1;
		}
		if (*optptr == '-' && optptr == largv[i] + 1) {
			found = find_long_opt(ctx, optptr + 1);
			if (found) {
				nlen = strlen(found->name);
				if (strlen(optptr + 1) != nlen) {
					argptr = optptr + nlen + 2;
					*(optptr + 1 + nlen) = '\0';
				} else {
					if (i + 1 < *argc) {
						argptr = largv[i + 1];
						arg_ind++;
					}
				}
			}
		} else {
			found = find_short_opt(ctx, *optptr);
			if (found) {
				if (arg_ind + i + 1 < *argc) {
					arg_ind++;
					argptr = largv[arg_ind + i];
				}
				next_shopt = *(optptr + 1);
				*(optptr + 1) = '\0';
			}
		}

		if (!found) {
			u_error_new(error, 0,
				"Unknown option %s\n", (*argv)[i]);
			goto ret;
		}

		if (found->arg == U_OPTION_ARG_NONE) {
			if (argptr) {
				arg_ind--;
				argptr = NULL;
			}
			if (found->arg_data) {
				if (!fill_tmp_data(tmp_data, &nd, NULL,
							NULL, found, error)) {
					goto ret;
				}
			}
			goto cont;
		}

		if (argptr == NULL) {
			u_error_new(error, 0,
				"Missing argument for -%s\n", optptr);
			goto ret;
		}

		if (found->arg_data == NULL) {
			goto cont;
		}

		if (!fill_tmp_data(tmp_data, &nd, optptr,
					argptr, found, error)) {
			goto ret;
		}
  cont:
		if (next_shopt == '\0') {
			i = (argptr == largv[i + arg_ind] || !argptr) ?
						i + arg_ind + 1 : i + 1;
			arg_ind = 0;
		}
	}
  ret:
		if (*error) {
			free_tmp_data(tmp_data, nd);
		} else {
			for (i = 1; i < *argc; i++) {
				for (j = 0; j < noopt; j++) {
					if (!strcmp((*argv)[i], tmp_argv[j])) {
						(*argv)[j + 1] = (*argv)[i];
						(*argv)[i] = NULL;
						break;
					}
				}
			}
			get_tmp_data(tmp_data, nd);
		}
		for (i = 1; i < *argc; i++)
			u_free(largv[i]);
		*argc = noopt + 1;
		u_free(tmp_data);

	return 0;
}


