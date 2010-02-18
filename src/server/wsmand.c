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

#ifdef HAVE_CONFIG_H
#include "wsman_config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdlib.h>

#include <stdio.h>
#include <ctype.h>

#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include <string.h>
#include <sys/stat.h>
#include <u/libu.h>


#include <time.h>
#include <assert.h>


#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"

#include "wsman-xml.h"
#include "wsman-xml-serializer.h"
#include "wsman-dispatcher.h"

#include "wsman-plugins.h"
#include "wsmand-listener.h"
#include "wsmand-daemon.h"


static int log_pid = 0;

static void
debug_message_handler(const char *str,
		      debug_level_e level, void *user_data)
{

	if (log_pid == 0)
		log_pid = getpid();

	if (level <= wsmand_options_get_debug_level()
	    || wsmand_options_get_foreground_debug() > 0) {
		struct tm *tm;
		time_t now;
		char timestr[128];
		char *log_msg;
		int p;

		time(&now);
		tm = localtime(&now);
		strftime(timestr, 128, "%b %e %T", tm);

		log_msg = u_strdup_printf("%s [%d] %s\n",
					  timestr, log_pid, str);
		if ((p =
		     write(STDERR_FILENO, log_msg, strlen(log_msg))) < 0)
			fprintf(stderr, "Failed writing to log file\n");
		fsync(STDERR_FILENO);

		u_free(log_msg);
	}
	if (level <= wsmand_options_get_syslog_level()) {
		char *log_name = u_strdup_printf("wsmand[%d]", log_pid);

		openlog(log_name, 0, LOG_DAEMON);
		syslog(LOG_INFO, "%s", str);
		closelog();
		u_free(log_name);
	}
}


static void initialize_logging(void)
{
	debug_add_handler(debug_message_handler, DEBUG_LEVEL_ALWAYS, NULL);

}				/* initialize_logging */

static void signal_handler(int sig_num)
{
	const char *sig_name = NULL;

	if (sig_num == SIGQUIT)
		sig_name = "SIGQUIT";
	else if (sig_num == SIGTERM)
		sig_name = "SIGTERM";
	else if (sig_num == SIGINT)
		sig_name = "SIGINT";
	else
		assert(1 == 1);

	debug("Received %s... Shutting down.", sig_name);
	wsmand_shutdown();
} /* signal_handler */




static void sighup_handler(int sig_num)
{
	debug("SIGHUP received; reloading data");

	if (wsmand_options_get_debug_level() == 0) {
		int fd;

		close(STDOUT_FILENO);

		fd = open("/var/log/wsmand.log",
			  O_WRONLY | O_CREAT | O_APPEND,
			  S_IRUSR | S_IWUSR);
		assert(fd == STDOUT_FILENO);

		close(STDERR_FILENO);

		fd = dup(fd);	/* dup fd to stderr */
		assert(fd == STDERR_FILENO);
	}

}				/* sighup_handler */



static int rc_write(int fd, const char *buf, size_t count)
{
	size_t bytes_remaining = count;
	const char *ptr = buf;

	while (bytes_remaining) {
		size_t bytes_written;

		bytes_written = write(fd, ptr, bytes_remaining);

		if (bytes_written == -1) {
			if (errno == EAGAIN || errno == EINTR) {
				continue;
			} else {
				break;
			}
		}

		bytes_remaining -= bytes_written;
		ptr += bytes_written;
	}

	if (bytes_remaining) {
		return (FALSE);
	}

	return (TRUE);
}


static void daemonize(void)
{
	int fork_rv;
	int i;
	int fd;
	char *pid;

	if (wsmand_options_get_foreground_debug() > 0) {
		return;
	}

	fork_rv = fork();
	if (fork_rv < 0) {
		fprintf(stderr, "wsmand: fork failed!\n");
	}

	if (fork_rv > 0) {
		exit(0);
	}

	log_pid = 0;
	setsid();

	/* Change our CWD to / */
	i=chdir("/");
        assert(i == 0);

	/* Close all file descriptors. */
	for (i = getdtablesize(); i >= 0; --i)
		close(i);

	fd = open("/dev/null", O_RDWR);	/* open /dev/null as stdin */
	assert(fd == STDIN_FILENO);

	/* Open a new file for our logging file descriptor.  This
	   will be the fd 1, stdout. */
	fd = open("/var/log/wsmand.log",
		  O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	assert(fd == STDOUT_FILENO);

	fd = dup(fd);		/* dup fd to stderr */
	assert(fd == STDERR_FILENO);

	fd = open(wsmand_options_get_pid_file(),
		  O_WRONLY | O_CREAT | O_TRUNC, 0644);
	pid = u_strdup_printf("%d", getpid());
	rc_write(fd, pid, strlen(pid));
	u_free(pid);
	close(fd);

	// TODO
	// remove /var/run/wsmand.pid
	// remove /var/lock/subsys/wsmand
}




int main(int argc, char **argv)
{
	struct sigaction sig_action;
	dictionary *ini;
	char *filename;
	WsManListenerH *listener = NULL;

	if (!wsmand_parse_options(argc, argv)) {
		fprintf(stderr, "Failed to parse command line options\n");
		exit(EXIT_FAILURE);
	}

	filename = (char *) wsmand_options_get_config_file();
	ini = iniparser_new(filename);
	debug("Using conf file: %s", filename);
	if (ini == NULL) {
		fprintf(stderr, "Cannot parse file [%s]\n", filename);
		return 1;
	} else if (!wsmand_read_config(ini)) {
		fprintf(stderr, "Configuration file not found\n");
		exit(EXIT_FAILURE);
	}

	daemonize();

	/* Set up SIGTERM and SIGQUIT handlers */
	sig_action.sa_handler = signal_handler;
	sigemptyset(&sig_action.sa_mask);
	sig_action.sa_flags = 0;
	sigaction(SIGINT, &sig_action, NULL);
	sigaction(SIGTERM, &sig_action, NULL);
	sigaction(SIGQUIT, &sig_action, NULL);

	/* Set up SIGHUP handler. */
	sig_action.sa_handler = sighup_handler;
	sigemptyset(&sig_action.sa_mask);
	sig_action.sa_flags = 0;
	sigaction(SIGHUP, &sig_action, NULL);

	initialize_logging();
	
	listener = wsmand_start_server(ini);
  
        if (listener) {
	   wsman_plugins_unload(listener);
	   u_free(listener);
	}
  
	debug_destroy_handlers();
	iniparser_free(ini);
        if (!listener) {
	   exit(EXIT_FAILURE);
	}
	return 0;
}
