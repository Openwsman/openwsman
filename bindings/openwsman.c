#include <u/libu.h>
#include <wsman-debug.h>
#include "openwsman.h"
/*-----------------------------------------------------------------*/
/* Rbwsman */
/* debug (mostly stolen from src/server/wsmand.c) */

void
debug_message_handler (const char *str,
                       debug_level_e level,
                       void *user_data)
{
  static int log_pid = 0;

  if (log_pid == 0)
    log_pid = getpid ();
#if 0
  if (level <= wsmand_options_get_debug_level ()
      || wsmand_options_get_foreground_debug() > 0 )
#endif
  {
    struct tm *tm;
    time_t now;
    char timestr[128];
    char *log_msg;
    int p;

    time (&now);
    tm = localtime (&now);
    strftime (timestr, 128, "%b %e %T", tm);

    log_msg = u_strdup_printf ("%s [%d] %s\n",
                               timestr, log_pid, str);
    if ( (p = write (STDERR_FILENO, log_msg, strlen (log_msg)) ) < 0  )
      fprintf(stderr, "Failed writing to log file\n");
    fsync (STDERR_FILENO);

    u_free (log_msg);
  }
#if 0
  if ( level <= wsmand_options_get_syslog_level ())
  {
    char *log_name = u_strdup_printf( "wsmand[%d]", log_pid );

    openlog( log_name, 0, LOG_DAEMON );
    syslog( LOG_INFO, "%s", str );
    closelog();
    u_free( log_name );
  }
#endif
}

