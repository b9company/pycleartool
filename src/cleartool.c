/* ============================================================================
 * Copyright (c) 2004, 2005, 2007, Rubycube.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Rubycube nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * ============================================================================
 */

#pragma ident "@(#)$Id: //dev/lib/pycleartool/src/cleartool.c#3 $"

/*
 * Pycleartool -- Python Extension to ClearCase
 */

#include "Python.h"

#ifndef PyDoc_STR
#define PyDoc_VAR(name)         static char name[]
#define PyDoc_STR(str)          (str)
#define PyDoc_STRVAR(name, str) PyDoc_VAR(name) = PyDoc_STR(str)
#endif

#include "proc_table.h"
#ifdef ATRIA_HPUX10
#include <string.h>
#include <strings.h>
#include "xdr.h"
#endif
#if defined ATRIA_WIN32_COMMON
#include <windows.h>
#endif
#if defined ATRIA_WIN32_COMMON || defined  ATRIA_HPUX10
#include <stdio.h>
#include <stdlib.h>
#endif

#define MODULE_NAME "cleartool"

/* Application name string.  Use to customize the application name as it
 * appears in messages provided by the ClearCase subsystem. */
#define APP_MAX_NAME_LEN 255
static char app_name[APP_MAX_NAME_LEN+1];

/* ----------------------------------------------------------------------------
 * Static routines
 */

static void
blok_init(BLOK *blokP)
{
    blokP->buffSize = BLOK_START_SIZE;
    blokP->currSize = 0;
    blokP->buffP = (char *) malloc(blokP->buffSize);
}

static void
blok_reset(BLOK *blokP)
{
    *blokP->buffP = '\0';
    blokP->currSize = 0;
}

static void
blok_done(BLOK *blokP)        
{
    free(blokP->buffP);
}

static void
silent(void *argP, char *strP)
{
    ;
}

static void
cmdout(void *argP, char *strP)
{
    BLOK *blokP;
    int len;
    blokP = (BLOK *) argP;
    len = strlen(strP);
    if (blokP->currSize + len + 1 > blokP->buffSize) {
        blokP->buffSize = blokP->currSize + len + 1;
        blokP->buffP = (char *) realloc(blokP->buffP, blokP->buffSize);
    }
    strcat(blokP->buffP, strP);
    blokP->currSize += len;
}

static int
dispatched_syn_call(char *cmdP,
                    BLOK *outP,
                    BLOK *errP,
                    gen_t area,
                    gen_t *a_cmdsyn_cmdflags,
                    gen_2_t *a_cmdsyn_proc_table)
{
    void (*out_rtn)(void*, char*), (*err_rtn)(void*, char*);

    /* Is standard out wanted? */
    if (outP == STANDARD)
        out_rtn = NULL;
    else if (outP == DEVNULL)
        out_rtn = silent;
    else {
        blok_reset(outP);
        out_rtn = cmdout;
    }
    
    /* Is standard err wanted? */
    if (errP == STANDARD)
        err_rtn = NULL;
    else if (errP == DEVNULL)
        err_rtn = silent;
    else {
        blok_reset(errP);
        err_rtn = cmdout;
    }
    imsg_set_app_name(app_name);
    imsg_redirect_output(out_rtn, outP, err_rtn, errP);
    return (cmdsyn_exec_dispatch(cmdP, area, a_cmdsyn_cmdflags, a_cmdsyn_proc_table) == T_OK);
}

static int
dispatched_synv_call(int argc,
                     char *argv[],
                     BLOK *outP,
                     BLOK *errP,
                     gen_t area,
                     gen_t *a_cmdsyn_cmdflags,
                     gen_2_t *a_cmdsyn_proc_table)
{
    void (*out_rtn)(void*, char*), (*err_rtn)(void*, char*);

    /* Is standard out wanted? */
    if (outP == STANDARD)
        out_rtn = NULL;
    else if (outP == DEVNULL)
        out_rtn = silent;
    else {
        blok_reset(outP);
        out_rtn = cmdout;
    }

    /* Is standard err wanted? */
    if (errP == STANDARD)
        err_rtn = NULL;
    else if (errP == DEVNULL)
        err_rtn = silent;
    else {
        blok_reset(errP);
        err_rtn = cmdout;
    }
    imsg_set_app_name(app_name);
    imsg_redirect_output(out_rtn, outP, err_rtn, errP);

    return (cmdsyn_execv_dispatch(argc, argv, area, a_cmdsyn_cmdflags, a_cmdsyn_proc_table) == T_OK);
}

/* ----------------------------------------------------------------------------
 * Python module methods
 */

PyDoc_STRVAR(set_app_name__doc__,
"set_app_name(string) -> None\n\n\
Set the application name.  This is used by the ClearCase subsystem when\n\
returning messages back to the user.  By default, the application name\n\
is set to 'cleartool'.");

static PyObject *
cleartool_set_app_name(PyObject *self, PyObject *args)
{
    char *name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    strncpy(app_name, name, APP_MAX_NAME_LEN);
    return Py_BuildValue("");
}

PyDoc_STRVAR(cmd__doc__,
"cmd(string) -> (status, out, err)\n\n\
Execute a ClearCase subcommand specified as a string.  Return a tuple with\n\
the ClearCase subcommand exit status, standard output, and standard error.");

static PyObject *
cleartool_cmd(PyObject *self, PyObject *args)
{
    char *cmd;
    int status;
    PyObject *retval = NULL;
    BLOK out;
    BLOK err;
    BLOK *blok_out_p;
    BLOK *blok_err_p;
    gen_t area =  stg_create_area(2048);
#ifdef ATRIA_WIN32_COMMON
    WORD VersionRequested;
    WSADATA wsaData;
    int myerr;
#endif

    if (!PyArg_ParseTuple(args, "s", &cmd))
        return NULL;
    blok_init(&out);
    blok_out_p = &out;
    blok_init(&err);
    blok_err_p = &err;
#ifdef ATRIA_WIN32_COMMON
    VersionRequested = MAKEWORD(2, 2);
    myerr = WSAStartup(VersionRequested, &wsaData);
    if (myerr != 0) {
        fprintf(stderr, "we could not find a usable WinSock DLL\n");
        return NULL;
    }
#endif
    pfm_init();
    vob_ob_all_cache_action(NULL,1,1);
    status = dispatched_syn_call(cmd,
                                 blok_out_p, 
                                 blok_err_p, 
                                 area,
                                 cmdsyn_get_cmdflags(),
                                 cmdsyn_proc_table);
    status = status ? 0 : 1;
    vob_ob_all_cache_action(NULL, 3, 0);
    stg_free_area(area, TRUE);
    retval = Py_BuildValue("(iss)", status, out.buffP, err.buffP);
    blok_done(&out);
    blok_done(&err);
    return retval;
}


PyDoc_STRVAR(version__doc__,
"version() -> (major, minor, patch)\n\n\
Return a tuple identifying the current ClearCase version.");

static PyObject *
cleartool_version(PyObject *self, PyObject *args)
{
    int major = CCASE_VER_MAJOR;
    int minor = CCASE_VER_MINOR;
    int patch = CCASE_VER_PATCH;
    int fixpack = CCASE_VER_FIXPACK;

    if (!PyArg_ParseTuple(args, ":version"))
        return NULL;
    return Py_BuildValue("(iiii)", major, minor, patch, fixpack);
}


/* List of all functions in the module */
static PyMethodDef cleartool_methods[] = {
    {"set_app_name", cleartool_set_app_name, METH_VARARGS, set_app_name__doc__},
    {"cmd", cleartool_cmd, METH_VARARGS, cmd__doc__},
    {"version", cleartool_version, METH_VARARGS, version__doc__},
    { NULL, NULL }
};

PyDoc_STRVAR(module__doc__,
"Pycleartool is a Python extension that provides a high-level interface to\n\
`IBM Rational ClearCase`_, suitable for scripting solutions.  Pycleartool\n\
links directly against ClearCase libraries, preventing you from\n\
extra-processing usually encountered in scripts that interact with\n\
ClearCase, where the creation of a cleartool subprocess is required to\n\
handle a ClearCase request.\n\
\n\
SYNOPSIS:\n\
\n\
    import cleartool\n\
    cleartool.set_app_name('myprog')\n\
    (maj, min, patch, fixpack) = cleartool.version()\n\
    (sts, out, err) = cleartool.cmd('lsvob')\n\
\n\
.. _IBM Rational ClearCase:\n\
   http://www.ibm.com/software/awdtools/clearcase/index.html");

/* Module initialization function */
void initcleartool(void) {
    Py_InitModule3(MODULE_NAME, cleartool_methods, module__doc__);
    strncpy(app_name, MODULE_NAME, APP_MAX_NAME_LEN);
}

/* vim:set et ff=unix ft=c sw=4 ts=4 tw=79: */
