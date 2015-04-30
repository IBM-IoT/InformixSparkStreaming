/*
** Title:          udr.c
** SCCSid:         %W% %E% %U%
** CCid:           %W% %E% %U%
** Author:         Informix Software, Inc.
** Created:        08/27/1999 14:15
** Description:    This is a generated source file for the boundingbox DataBlade module.
** Comments:       Generated for project boundingbox.0.0
*/

/*
**  The following is placed here to insure
**  that name "mangling" does not occur.
*/
#ifdef __cplusplus

extern "C"
{

#endif

/* Standard library includes. */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

/* Used by Informix GLS routines. */
#include <ifxgls.h>

/* Include when accessing the Informix API. */
#include <mi.h>

/* This is the project include file. */
#include "boundingbox.h"


/* {{FUNCTION(84b1d2f1-f79a-11d2-9ccf-0060970f543e) (MergeSection) */

/*******************************************************************************
**
** Function name:
**
**	BoundingBoxVersion
**
** Description:
**
** Special Comments:
**
**	Entrypoint for the SQL routine BoundingBoxVersion () returns lvarchar.
**
**      A stack size of 32,767 bytes  has  been  requested  for
**      the routine.  Normally, this is sufficient memory for most
**      invocations of your UDR.  If you intend, however, to  call
**      this routine recursively or other routines that use  large
**      or unknown stack sizes, you should use mi_call().  mi_call
**      checks to insure that sufficient stack space is available.
**      For  more  details  regarding  this   function,  look  in:
**      The  DataBlade  API Programmer's  Manual  (see Stack Space
**      Allocation in Chapter 11).
**
** Parameters:
**
**	None
**
** Return value:
**
**	mi_lvarchar
**
** History:
**
**	08/27/1999 - Generated by BladeSmith Version 4.00.TC1B.
**
** Identification:
**
** NOTE:
**
**	BladeSmith will add and remove parameters from the function
**	prototype, and will generate tracing calls.  Only edit code
**	in blocks marked  Your_<section>.  Any other  modifications
**	will require manual merging.
**
********************************************************************************
*/
UDREXPORT mi_lvarchar *BoundingBoxVersion(MI_FPARAM *Gen_fparam)
{
	mi_lvarchar *        Gen_RetVal;      /* The return value.            */
	MI_CONNECTION *      Gen_Con;         /* The connection handle.       */
	/* ------ {{Your_Declarations (PreserveSection) BEGIN ------ */
	/* ------ }}Your_Declarations (#SVG2) END ------             */

	/* Use the NULL connection. */
	Gen_Con = NULL;


	/* ------ {{Your_Code (PreserveSection) BEGIN ------ */

	Gen_RetVal = mi_string_to_lvarchar("BoundingBox DataBlade version 1.2");

	/* ------ }}Your_Code (#OIVP) END ------ */

	/* Return the function's return value. */
	return Gen_RetVal;
}
/* }}FUNCTION (#6JTQ) */



#ifdef __cplusplus

}

#endif