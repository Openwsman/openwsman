#!/usr/bin/env python

"""
setup.py file for pywsman
"""

from __future__ import print_function
from distutils.core import setup, Extension
from distutils.command.build import build as _build
import subprocess
import shutil
import sys
import os



PYDIST_VERSION = 0


def errprint(*objs):
    print(*objs, file=sys.stderr)


def get_build_info():
    '''
    Figure out whether we are building from within the openwsman source
    directory, or from a Python source distribution.
    Returns a tuple of (openwsman version string, Bool)
    '''
    try:
        with open(os.devnull, 'w') as devnull:
            subprocess.check_call(['cmake', '-P', 'version.cmake'], stdout=devnull, stderr=devnull)
    except subprocess.CalledProcessError as e:
        if e.returncode == 1:
            errprint('No version.i.in file found -- Building from sdist.')
            in_openwsman = False
        else:
            raise
    except OSError:
        errprint('Failed to run cmake - is it installed? I will assume we are building from an sdist, but I may be wrong!')
        in_openwsman = False
    else:
        errprint('version.i.in file found -- Building from openwsman source.')
        in_openwsman = True
    finally:
        from version import __VERSION__
    return (__VERSION__, in_openwsman)


def copy_files(*source_dirs):
    with open('MANIFEST') as _manifest:
        for _file in _manifest:
            _file = _file.strip()
            dirname = os.path.dirname(_file)
            if dirname and not os.path.exists(dirname):
                os.makedirs(dirname)
            if not os.path.exists(_file):
                copied = False
                for source_dir in source_dirs:
                    try:
                        shutil.copyfile('%s/%s' % (source_dir, _file), _file)
                    except IOError:
                        continue
                    else:
                        copied = True
                        errprint('Copied %s/%s to %s' % (source_dir, _file, _file))
                if not copied:
                    raise


class Build(_build):
    sub_commands = [
        ('build_ext', _build.has_ext_modules),
        ('build_py', _build.has_pure_modules),
        ('build_clib', _build.has_c_libraries),
        ('build_scripts', _build.has_scripts),
    ]


with open('README.rst') as _readme:
    long_description = _readme.read()

version, from_source = get_build_info()

include_dirs = [os.path.abspath(os.path.dirname(__file__)), ]
if from_source:
    copy_files('../..', '../../..')
    include_dirs += ['../..', '../../../include']
else:
    wsman_inc = os.environ.get('OPENWSMAN_INCLUDE', '/usr/include/openwsman')
    include_dirs += [wsman_inc]
for path in include_dirs:
    found = os.path.exists('%s/wsman-client.h' % path)
    if found:
        break
if not found:
    raise RuntimeError(('Could not find openwsman headers.'),
        ('Please install them and/or specify their location using the OPENWSMAN_INCLUDE environment variable.'))


_pywsman = Extension('_pywsman',
    sources = ['openwsman.c', 'openwsman.i', ],
    include_dirs = include_dirs,
    swig_opts = ['-I' + dir for dir in include_dirs] + [
        '-features', 'autodoc',
    ],
    libraries = ['pthread', 'curl', 'wsman', 'wsman_client', 'wsman_curl_client_transport'],
)


setup(name='pywsman',
    version='%s-%s' % (version, PYDIST_VERSION),
    description='Python openwsman bindings.',
    author='The Openwsman project',
    author_email='openwsman-devel@lists.sourceforge.net',
    license='BSD 3-clause',
    long_description=long_description,
    url='http://openwsman.github.io',
    ext_modules=[_pywsman],
    py_modules=['pywsman'],
    classifiers=[
        'License :: OSI Approved :: BSD License',
    ],
    cmdclass={'build': Build},
)

