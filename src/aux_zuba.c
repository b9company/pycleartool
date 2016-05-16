/*************************************************************************
*                                                                        *
* Copyright 2002 Rational Software Corporation.                          *
* All Rights Reserved.                                                   *
* This software is distributed under the Common Public License Version   *
* 0.5 (CPL), and you may use this software if you accept that agreement. *
* You should have received a copy of the CPL with this software          *
* in the file LICENSE.TXT.  If you did not, please visit                 *
* http://www.opensource.org/licenses/cpl.html for a copy of the license. *
*                                                                        *
*************************************************************************/

#if defined(hp11_pa) || defined(hp10_pa)
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#if defined (sun5)
#include <netdb.h>
#endif
#if defined(hp11_pa) || defined(hp10_pa)
#include <sys/param.h>
#endif
#include <sys/types.h>
#define PORTMAP
#include <rpc/rpc.h>


CLIENT  *clntudp_create(struct  sockaddr_in  *addr,rpcprog_t prognum, rpcvers_t versnum, struct timeval wait, int *fdp){
    CLIENT *rv;
    rv = (CLIENT *)NULL;
    return rv;
};



CLIENT  *clntudp_bufcreate(struct  sockaddr_in  *addr,rpcprog_t prognum, rpcvers_t versnum, struct timeval wait, int *fdp,  uint_t sendsz,uint_t recvsz){
    CLIENT *rv;
    rv = (CLIENT *)NULL;
    return rv;
};



