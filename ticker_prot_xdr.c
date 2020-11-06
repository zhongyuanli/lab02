/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "ticker_prot.h"

bool_t
xdr_msg_t (XDR *xdrs, msg_t *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, objp, 79))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_submit_args (XDR *xdrs, submit_args *objp)
{
	register int32_t *buf;

	 if (!xdr_msg_t (xdrs, &objp->msg))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_submit_result (XDR *xdrs, submit_result *objp)
{
	register int32_t *buf;

	 if (!xdr_bool (xdrs, &objp->ok))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_xaction_args (XDR *xdrs, xaction_args *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->my_id))
		 return FALSE;
	 if (!xdr_msg_t (xdrs, &objp->msg))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->ts))
		 return FALSE;
	return TRUE;
}
