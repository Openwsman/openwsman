#!/bin/sh

#!/bin/sh -e

CERTFILE=@WSMANCONF_DIR@/servercert.pem
KEYFILE=@WSMANCONF_DIR@/serverkey.pem
CNFFILE=@WSMANCONF_DIR@/ssleay.cnf

if [ "$1" != "--force" -a -f $KEYFILE ]; then
  echo "$KEYFILE exists!  Use \"$0 --force.\""
  exit 0
fi

if [ "$1" = "--force" ]; then
  shift
fi

echo
echo creating selfsigned certificate
echo "replace it with one signed by a certification authority (CA)"
echo
echo enter your ServerName at the Common Name prompt
echo

# use special .cnf, because with normal one no valid selfsigned
# certificate is created

openssl req -days 365 $@ -config $CNFFILE \
  -newkey rsa:2048 -x509 -nodes -out $CERTFILE \
  -keyout $KEYFILE
chmod 600 $KEYFILE

