SUBDIRS =  init

pamdir = $(sysconfdir)/pam.d
pam_DATA = pam/openwsman
noinst_DATA = ssleay.cnf openwsman.conf openwsman_client.conf
openwsmanconf_DATA = owsmangencert.sh
openwsmanconfdir = $(sysconfdir)/openwsman

EXTRA_DIST = $(noinst_DATA) $(openwsmanconf_DATA) $(pam_DATA)
CLEANFILES = owsmangencert

