Openwsman - WS-Management for all
#################################

============    =============================================
home            http://openwsman.github.io
wiki            https://github.com/Openwsman/openwsman/wiki
mailing list    openwsman-devel@lists.sourceforge.net
source          https://github.com/Openwsman/openwsman
bugs            https://github.com/Openwsman/openwsman/issues
============    =============================================

Description
===========

Openwsman is an open source implementation of WS-Management; enabling the in-band management of Linux/unix/Windows platforms.  Openwsman supports the generic WS-Management protocol as well as specific protocol extensions for the `Common Information Model <http://www.dmtf.org/standards/cim>`_ (CIM).

Openwsman provides 'C' language API by default and a 'C++' API in an 'alpha' state. For other languages, see the bindings.

About this Python package
=========================

This folder allows one to build and/or install a Python package of the openwsman bindings. It has been tested on Ubuntu 14.04 (Trusty) with libopenwsman v2.4.15.

Building a source distribution
------------------------------

Running ``python setup.py sdist``, from within this directory, will create a Python source distribution which brings in the required C/SWIG source and include files.

Subsequent installation of this sdist assumes you have the necessary development headers and libraries installed system-wide.

Alternatively, the ``OPENWSMAN_INCLUDE`` environment variable can be set during build/install, specifying a custom path in which to find the libopenwsman development headers.

Building from within the openwsman source directory
---------------------------------------------------

Running ``setup.py install``, or similar, from within the openwsman source directory, will cause the setup script to look for its include files in the source directory, as opposed to an installation path on the system. 

NOTE: Shared libraries will still be sourced from the standard system location(s).

About the bindings
==================

The bindings provide the simplest way to perform WS-Management operations with Openwsman.

Bindings exist for
`Ruby <https://github.com/Openwsman/openwsman/tree/master/bindings/ruby>`_,
`Python <https://github.com/Openwsman/openwsman/tree/master/bindings/python>`_,
`Perl <https://github.com/Openwsman/openwsman/tree/master/bindings/perl>`_, and
`Java <https://github.com/Openwsman/openwsman/tree/master/bindings/java>`_.
with plenty of examples in the respective 'tests' directories.

Bugs
====

If you have problems using Openwsman, report to the `mailing list <mailto:openwsman-devel@lists.sourceforge.net>`_ first.

If you are sure to have found a bug, please report it via the `Github issue tracker <https://github.com/Openwsman/openwsman/issues>`_.

License
=======

Openwsman is copyright (C) 2004-2006 by Intel Corp, 2006-2013 by SUSE Linux Products GmbH.

The python packaging scripts contained in this directory are copyright (C) 2015, Ocado Innovation Ltd.

Openwsman is free software, and may be redistributed under the terms of the `BSD-3-Clause <https://github.com/Openwsman/openwsman/blob/master/COPYING>`_ license.

Warranty
========

This software is provided "as is" and without any express or implied warranties, including, without limitation, the implied warranties of merchantability and fitness for a particular purpose.
