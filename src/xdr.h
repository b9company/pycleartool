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
#include <sys/types.h>
#include <rpc/xdr.h>
#include <sys/time.h>



int xdr_timeval(struct XDR *xdrs,struct timeval *objp)
{
    if (!xdr_time_t(xdrs, &objp->tv_sec,0) ||   !xdr_long(xdrs, &objp->tv_usec))
    {
        return;
    }
    return (TRUE);
};
