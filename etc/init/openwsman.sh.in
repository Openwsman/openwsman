#!/bin/sh
#
### BEGIN INIT INFO
# Provides: openwsmand
# Required-Start: $remote_fs
# Required-Stop: $network
# Default-Start: 2 3 4 5
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
OPTIONS=
PIDFILE=/var/run/$NAME.pid

if [ $EUID != 0 ]; then
 echo "This script must be run as root."
 exit 1;
fi

if [ "$DESCRIPTIVE" = "" ]; then
 DESCRIPTIVE="Openwsman Daemon"
fi

lockfile=${SVIlock:-/var/lock/subsys/$NAME}

[ -x $DAEMON ] || exit 0

if [ -f "/etc/sysconfig/openwsman" ]; then
    . /etc/sysconfig/openwsman
fi
# See how we were called.
. /etc/rc.status

# Reset status of this service
rc_reset


case "$1" in
    start)
    if [ ! -f "@SYSCONFDIR@/serverkey.pem" ]; then
        if [ -f "/etc/ssl/servercerts/servercert.pem" \
                -a -f "/etc/ssl/servercerts/serverkey.pem" ]; then
            echo "Using common server certificate /etc/ssl/servercerts/servercert.pem"
            ln -s /etc/ssl/servercerts/server{cert,key}.pem /etc/openwsman/
        else
            echo "Generating Openwsman server public certificate and private key"
            FQDN=`hostname --fqdn`
            if [ "x${FQDN}" = "x" ]; then
                FQDN=localhost.localdomain
            fi
cat << EOF | sh @SYSCONFDIR@/owsmangencert.sh > /dev/null 2>&1
--
SomeState
SomeCity
SomeOrganization
SomeOrganizationalUnit
${FQDN}
root@${FQDN}
EOF
        fi
    fi

    # Start daemons.
    echo -n "Starting the $DESCRIPTIVE"
    startproc -p $PIDFILE $DAEMON > /dev/null 2>&1
    rc_status -v
    touch $lockfile
    ;;

    stop)
    # Stop daemons.
    echo -n "Shutting down $DESCRIPTIVE"
    killproc -p $PIDFILE -TERM $DAEMON
    rc_status -v
    rm -f $lockfile
    ;;

    restart|force-reload)
    $0 stop
    $0 start

    ;;

    reload)
    echo -n "Reload service $DESCRIPTIVE"
    killproc -p $PIDFILE  -HUP $DAEMON
    rc_status -v
    ;;

    status)
    echo -n "Checking for service $DESCRIPTIVE"
    checkproc -p $PIDFILE $DAEMON
    rc_status -v
    ;;

    *)
    echo "Usage: $0 {restart|start|stop|reload|force-reload|status}"
esac

rc_exit
