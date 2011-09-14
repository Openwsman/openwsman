#!/bin/sh
#
### BEGIN INIT INFO
# Provides: openwsmand
# Required-Start: $remote_fs
# Required-Stop: $network
# Default-Start: 2 3 5
# Default-Stop: 0 1 6 
# Short-Description: Openwsman Daemon
# Description: openwsmand
#       Start/Stop the Openwsman Daemon
### END INIT INFO
#
#
# chkconfig: 2345 36 64
# description: Openwsman Daemon
# processname: openwsmand

NAME=openwsmand
DAEMON=/usr/sbin/$NAME
OPTIONS=-S # with SSL
PIDFILE=/var/run/$NAME.pid

lsb=0

if [ $EUID != 0 ]; then
 echo "This script must be run as root."
 exit 1;
fi

if [ "$DESCRIPTIVE" = "" ]; then
 DESCRIPTIVE="Openwsman Daemon"
fi

lockfile=${X1}{SVIlock:-/var/lock/subsys/$NAME}

[ -x $DAEMON ] || exit 0

if [ -f /etc/rc.status ]; then
        # LSB conformant system
	lsb=1
	
	# See how we were called.
	. /etc/rc.status

	# Reset status of this service
	rc_reset

fi

start()
{
    if [ ! -f "@WSMANCONF_DIR@/serverkey.pem" ]; then
      if [ -f "/etc/ssl/servercerts/servercert.pem" \
           -a -f "/etc/ssl/servercerts/serverkey.pem" ]; then
	echo "Using common server certificate /etc/ssl/servercerts/servercert.pem"
	ln -s /etc/ssl/servercerts/server{cert,key}.pem @WSMANCONF_DIR@
      else
	echo "FAILED: Starting openwsman server"
        echo "There is no ssl server key available for openwsman server to use."
	echo -e "Please generate one with the following script and start the openwsman service again:\n"
	echo "##################################"
        echo "/etc/openwsman/owsmangencert.sh"
	echo "================================="

	echo "NOTE: The script uses /dev/random device for generating some random bits while generating the server key."
	echo "      If this takes too long, you can replace the value of \"RANDFILE\" in @SYSCONFDIR@/ssleay.cnf with /dev/urandom. Please understand the implications of replacing the RNADFILE."
		
      fi
    fi

    # Start daemons.
    echo -n "Starting the $DESCRIPTIVE"
    if [ $lsb -ne 0 ]; then
      startproc -p $PIDFILE $DAEMON $OPTIONS> /dev/null 2>&1
      rc_status -v
    else
      $DAEMON -S && echo "               done." || echo "  failed."
    fi
}

stop()
{
	# Stop daemons.
	echo -n "Shutting down $DESCRIPTIVE  "
	if [ $lsb -ne 0 ]; then
          killproc -TERM $DAEMON
          rc_status -v
	else
          kill -9 `pidof openwsmand` > /dev/null 2>&1
	  if [ $? -eq 0 ]; then
	    echo "		done"
	  fi 
	fi
}

case "$1" in
    start)
	start
    	touch $lockfile
    ;;

    stop)
    	stop
	rm -f $lockfile
    ;;

    restart)
    	stop
    	start

    ;;
    force-reload)
    	stop
    	start

    ;;

    reload)
    echo -n "Reload service $DESCRIPTIVE"
    if [ $lsb -ne 0 ]; then
      killproc -HUP $DAEMON
      rc_status -v
    else
      killall -HUP openwsmand && echo "                 done." || echo "  failed."
    fi
    ;;

    status)
    echo -n "Checking for service $DESCRIPTIVE"
    if [ $lsb -ne 0 ]; then
      checkproc $DAEMON
      rc_status -v
    else
      pidof openwsmand > /dev/null 2>&1
      if [ $? -eq 0 ]; then
        echo "             running"
      else
        echo "             stopped"
      fi
    fi
    ;;

   condrestart)
      [ -e $lockfile ] && restart
    ;;

    *)
    echo "Usage: $0 {restart|start|stop|reload|force-reload|status|condrestart}"
esac

if [ $lsb -ne 0 ]; then
	rc_exit
else
	exit 0
fi
