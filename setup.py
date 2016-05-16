#!/usr/bin/env python

# setup.py - Setup script to Pycleartool
# Copyright (c) 2005  Vincent Besanceney
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

'''Python Extension to ClearCase

Pycleartool is a Python extension that provides a high-level interface to `IBM
Rational ClearCase`_, suitable for scripting solutions.  Pycleartool links
directly against ClearCase libraries, preventing you from extra-processing
usually encountered in scripts that interact with ClearCase, where the
creation of a cleartool subprocess is required to handle a ClearCase request.
See the ``README`` file for more information.

.. _IBM Rational ClearCase:
   http://www.ibm.com/software/awdtools/clearcase/index.html
'''
__docstring__ = 'reStructuredText'

import os
import sys
import re
import commands

from distutils import sysconfig
from distutils.ccompiler import new_compiler
from distutils.command.build_ext import build_ext as _build_ext
from distutils.core import setup, Extension

# -----------------------------------------------------------------------------
# Custom error messages and exceptions

class SetupError(Exception):
    def __init__(self, msg):
        self.msg = 'Error: %s' % msg

ERR_MSG_BADVER = '''Your version of %s is too old.
    Expected: %s
    Found: %s'''

ERR_MSG_CCASENOHOME = '''
    Cannot determine ClearCase home directory.  Try to set the CLEARCASEHOME
    environment variable to where ClearCase resides at your site and re-run
    this script.'''

ERR_MSG_CCASEVER = '''
    Cannot determine ClearCase version.  Be sure the file:
        %s
    exists and is world-readable.'''

# -----------------------------------------------------------------------------
# Supporting routines and classes

def split_version(version):
    version = re.sub(r'[-+]\d*$', '', version) # Remove patch level, if any
    version = version.split('.')
    version = map(int, version)
    return version

def fmt_version(version):
    return '.'.join(map(str, version))

class ClearcaseConfig:

    home = '/usr/atria'   # ClearCase default home directory
    ver = [0, 0, 0]       # ClearCase release
    libs = ['atriacmdsyn', 'atriacmd', 'atriasumcmd', 'atriasum',
        'atriamsadm', 'atriamntrpc', 'atriacm', 'atriavob', 'atriaview',
        'atriacm', 'atriadbrpc', 'atriatirpc', 'atriaxdr', 'atriamvfs',
        'atriatbs', 'atriaadm', 'atriasplit', 'atriacredmap', 'atriaks',
        'ezrpc', 'rpcsvc', 'atriaccfs', 'atriasquidad', 'atriasquidcore' ]
    libs6 = ['atriamsadm', 'atriamsinfobase', 'atriamsinfovob' ]

    def __init__(self):
        if 'CLEARCASEHOME' in os.environ:
            self.home = os.environ['CLEARCASEHOME']
        elif 'ATRIAHOME' in os.environ:     # Prior to ClearCase 2003
            self.home = os.environ['ATRIAHOME']
        elif sys.platform == 'win32':
            self.home = 'C:\Program Files\ClearCase'
        if not os.path.isdir(self.home):
            raise SetupError(ERR_MSG_CCASENOHOME)
        version_pname = os.path.join(self.home, 'install', 'version')
        try:
            f = open(version_pname, 'r')
            self.ver = f.readline()
        except:
            raise SetupError(ERR_MSG_CCASEVER % version_pname)
        f.close()
        self.ver = self.ver.split(' ')[2]
        self.ver = split_version(self.ver)
        self.ver += [0] * (3-len(self.ver)) # Ensure the version is always
                                            # three-digit long
        ccase_curr_ver = self.ver[:]
        if ccase_curr_ver[0] >= 2000:
            ccase_curr_ver.pop(0)           # New version style, get rid of the
                                            # leading number.
        # Check this version against the minimum required version
        ccase_xpct_ver = [4, 2]
        if ccase_curr_ver < ccase_xpct_ver:
            raise SetupError(ERR_MSG_BADVER % ('ClearCase',
                fmt_version(ccase_xpct_ver),
                fmt_version(ccase_curr_ver)))
        

    def get_incs(self):
        return [ os.path.join(self.home, 'include') ]

    def get_libdirs(self):
        if sys.platform.startswith('win'):
            return [ os.path.join(self.home, 'bin') ]
        else:
            return [ os.path.join(self.home, 'shlib') ]

    def get_libs(self):
        if self.ver[0] < 2003:
            return self.libs
        else:
            return self.libs + self.libs6

# -----------------------------------------------------------------------------
# Specialized extension builder

class build_ext(_build_ext):

    def build_sunos_deps(self):
        # Check for SunOS 5.7 or higher
        sunos_xpct_ver = [5, 7]
        sunos_curr_ver = split_version(os.uname()[2])
        if sunos_curr_ver < sunos_xpct_ver:
            raise SetupError(ERR_MSG_BADVER % ('Solaris',
                fmt_version(sunos_xpct_ver),
                fmt_version(sunos_curr_ver)))

        # Build libzuba
        zuba_cc = new_compiler()
        sysconfig.customize_compiler(zuba_cc)
        zuba_cc.set_executables(
            linker_so = '/usr/ccs/bin/ld -G -f /usr/ucblib/librpcsoc.so.1 -z interpose')
        obj = zuba_cc.compile(['src/aux_zuba.c'],
            output_dir=self.build_temp)
        self._built_objects = obj[:]
        zuba_cc.link_shared_object(obj,
            os.path.join(self.build_lib, 'libzuba.so'),
            build_temp=self.build_temp)

        # libCrun kludge
        libcrun_pname = os.path.join(self.build_temp, 'libCrun.so')
        if os.path.exists(libcrun_pname):
            os.unlink(libcrun_pname)
        os.symlink('/usr/lib/libCrun.so.1', libcrun_pname)

    def build_linux_deps(self):
        # XXX Should provide some sanity checks...
        pass

    def build_hpux_deps(self):
        raise SetupError('Pycleartool does not support this platform yet.')

    def build_aix_deps(self):
        raise SetupError('Pycleartool does not support this platform yet.')

    def build_win32_deps(self):
        raise SetupError('Pycleartool does not support this platform yet.')

    def build_extensions(self):
        pyct_include_dirs = [ sysconfig.get_python_inc() ]
        pyct_define_macros = [ ]
        pyct_compile_args = [ ]
        pyct_libraries = [ ]
        pyct_library_dirs = [ self.build_lib, self.build_temp]
        pyct_runtime_library_dirs = [ ]
        pyct_link_args = [ ]

        # Deal with platform dependencies first
        if sys.platform.startswith('sunos'):
            self.build_sunos_deps()
            pyct_define_macros += [('SVR4', None)]
            pyct_link_args += ['-t', '-ucmdsyn_proc_table']
            pyct_libraries += ['c', 'w', 'Crun', 'zuba']
            pyct_runtime_library_dirs += ['$ORIGIN']
        elif sys.platform.startswith('linux'):
            self.build_linux_deps()
            pyct_define_macros += [('ATRIA_LINUX', None)]
            pyct_link_args += ['-ucmdsyn_proc_table']
            pyct_libraries += ['c', 'ncurses', 'crypt', 'nsl']
        elif sys.platform.startswith('hp-ux'):
            self.build_hpux_deps()
        elif sys.platform.startswith('aix'):
            self.build_aix_deps()
        elif sys.platform.startswith('win'):
            self.build_win32_deps()
        else:
            raise SetupError('This platform is not supported by ClearCase.')

        # ClearCase settings
        ccase = ClearcaseConfig()
        pyct_define_macros += [
            ('CCASE_VER_MAJOR', ccase.ver[0]),
            ('CCASE_VER_MINOR', ccase.ver[1]),
            ('CCASE_VER_PATCH', ccase.ver[2]),
            ]
        pyct_libraries += ccase.get_libs()
        pyct_library_dirs += ccase.get_libdirs()
        pyct_runtime_library_dirs += ccase.get_libdirs()

        # Build the extensions
        self.check_extensions_list(self.extensions)
        for ext in self.extensions:
            if ext.name == 'cleartool':
                ext.include_dirs += pyct_include_dirs
                ext.define_macros += pyct_define_macros
                ext.extra_compile_args += pyct_compile_args
                ext.libraries += pyct_libraries
                ext.library_dirs += pyct_library_dirs
                ext.runtime_library_dirs += pyct_runtime_library_dirs
                ext.extra_link_args += pyct_link_args
                # Configure cleartool extension
            self.build_extension(ext)

# -----------------------------------------------------------------------------
# Setup

MOD_NAME = 'pycleartool'
MOD_VERSION = '2005.01'
MOD_AUTHOR = 'Vincent Besanceney'
MOD_AUTHOR_EMAIL = 'vincent {at} rubycube {dot} net'
MOD_LICENSE = 'GNU General Public License'
MOD_URL = 'http://rubycube.net/ressources/pycleartool/'
MOD_CLASSIFIERS = '''
Development Status :: 5 - Production/Stable
Intended Audience :: Developers
Intended Audience :: System Administrators
License :: OSI Approved :: GNU General Public License (GPL)
Operating System :: POSIX :: Linux
Operating System :: POSIX :: SunOS/Solaris
Programming Language :: C
Programming Language :: Python
Topic :: Software Development :: Libraries :: Python Modules
'''

def main():
    try:
        # Check Python requirements
        py_xpct_ver = (2, 2)
        py_curr_ver = sys.version_info[:2]
        if py_curr_ver < py_xpct_ver:
            raise SetupError(ERR_MSG_BADVER % ('Python',
                fmt_version(py_xpct_ver),
                fmt_version(py_curr_ver)))

        cleartool_ext = Extension('cleartool', ['src/cleartool.c'])
        if py_curr_ver >= (2, 3):
            # Add additional setup arguments, used to register with PyPI,
            # Python 2.2.2 and earlier don't support these (nor the 'register'
            # subcommand of setup.py)
            setup_extra_args = {
                'classifiers': filter(None, MOD_CLASSIFIERS.split('\n')),
                'license': MOD_LICENSE,
                }
        else:
            setup_extra_args = { }

        setup(# PyPI Metadata (PEP 301)
            name = MOD_NAME,
            version = MOD_VERSION,
            author = MOD_AUTHOR,
            author_email = MOD_AUTHOR_EMAIL,
            url = MOD_URL, 
            description = __doc__.split('\n')[0],
            long_description = __doc__.strip(),
            ext_modules = [cleartool_ext],
            cmdclass = {'build_ext': build_ext},
            **setup_extra_args
            )

    except SetupError, why:
        print >>sys.stderr, why.msg
        return 1
    return 0

if __name__ == '__main__':
    sys.exit(main())

# vim:set et ff=unix ft=python sw=4 ts=4 tw=79:
