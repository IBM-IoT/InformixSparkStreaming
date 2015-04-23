#include <mi.h>          /* Used to access the Informix API */
#include <float.h>       /* DBL_EPSILON */
#include <math.h>        /* abs */
#include <stdio.h>       /* sscanf */
#include <string.h>
#include <stdlib.h>

/*
 * Tracing-related macros
 */
#define TRACE_CLASS "BoundingBox"
#define TRACE_LEVEL 20

#define BBOX_TRACE_ENTER(fn) DPRINTF(TRACE_CLASS, TRACE_LEVEL, ("Enter " #fn))
#define BBOX_TRACE_EXIT(fn)  DPRINTF(TRACE_CLASS, TRACE_LEVEL, ("Exit " #fn))
#define BBOX_TRACE(args)     DPRINTF(TRACE_CLASS, TRACE_LEVEL, args)

typedef mi_unsigned_char1   mi_uchar;
typedef mi_unsigned_integer mi_uint;
typedef mi_double_precision mi_double;

typedef struct
{
  mi_double x;
  mi_double y;
  mi_double z;
}
aPoint3;

typedef struct
{
  aPoint3 ll;
  aPoint3 ur;
}
BoundingBox;


/*
 * INSERT INTO informix.systraceclasses(name) VALUES ('BoundingBox');
 * 
 * CREATE PROCEDURE traceset(LVARCHAR, INTEGER)
 * EXTERNAL NAME '/opt/ifmx/extend/boundingbox.0.0/boundingbox.bld(BoundingBoxTraceSet)'
 * LANGUAGE C;
 *
 * EXECUTE PROCEDURE TRACESET('/tmp/boundingbox.trc', 40);
 */
void BoundingBoxTraceSet (mi_lvarchar *trace_path, 
                          mi_integer   trace_level)
{
  char trace_cmd[20];
  mi_string errBuf[80];
  mi_string *r = "BoundingBoxTraceSet";
  
  mi_tracefile_set (mi_lvarchar_to_string (trace_path));
  sprintf (trace_cmd, "%s %d", TRACE_CLASS, trace_level);
  
  if (mi_tracelevel_set (trace_cmd) == MI_ERROR) {
    sprintf (errBuf, "%s: mi_tracelevel_set failed!", r);
    mi_db_error_raise (NULL, MI_EXCEPTION, errBuf);
  }
  
  DPRINTF (TRACE_CLASS, TRACE_LEVEL, ("%s: Trace Level (%d)", r, trace_level));
}

void swapDouble(double *a, double *b)
{
  double tmp = *a;
  *a = *b;
  *b = tmp;
}

MI_DECL BoundingBox *
BoundingBoxInput (mi_lvarchar *input_varchar)
{
  BoundingBox *bbox;
  mi_char     *input_text;
  mi_integer  n;
  mi_double   tmp;

  BBOX_TRACE_ENTER("BoundingBoxInput");

  /* Allocate space for the bounding box being read in */  
  bbox = mi_alloc (sizeof(BoundingBox));
  DPRINTF (TRACE_CLASS, TRACE_LEVEL, ("sizeof(BoundingBox) = %d", sizeof(BoundingBox)));

  /* Convert the input */ 
  input_text = mi_lvarchar_to_string (input_varchar);

  /* Use sscanf to read in the input */
  n = sscanf (input_text, "(%lf %lf %lf %lf %lf %lf)",
    &(bbox->ll.x), &(bbox->ll.y), &(bbox->ll.z),
    &(bbox->ur.x), &(bbox->ur.y), &(bbox->ur.z));

  /* Check for errors in the input */
  if (n != 6) {
    mi_db_error_raise (NULL, MI_EXCEPTION, "Cannot parse input text", 0);
  }

  /* Fix input where the lower-left point is greater than the upper-right point */
  if (bbox->ll.x > bbox->ur.x) {
    swapDouble (&(bbox->ll.x), &(bbox->ur.x));
  }
  if (bbox->ll.y > bbox->ur.y) {
    swapDouble (&(bbox->ll.y), &(bbox->ur.y));
  }
  if (bbox->ll.z > bbox->ur.z) {
    swapDouble (&(bbox->ll.z), &(bbox->ur.z));
  }

  BBOX_TRACE_EXIT("BoundingBoxInput");

  return bbox;
}

MI_DECL mi_lvarchar *
BoundingBoxOutput (BoundingBox *bbox)
{
  mi_char textbuf[50];

  BBOX_TRACE_ENTER("BoundingBoxOutput");
  
  sprintf (textbuf, "(%.15G %.15G %.15G %.15G %.15G %.15G)",
    bbox->ll.x, bbox->ll.y, bbox->ll.z,
    bbox->ur.x, bbox->ur.y, bbox->ur.z);

  BBOX_TRACE_EXIT("BoundingBoxOutput");
  
  return mi_string_to_lvarchar (textbuf);
}

MI_DECL BoundingBox *
BoundingBoxReceive (mi_sendrecv *data)
{
  mi_uchar    *datap;
  BoundingBox *bbox;

  BBOX_TRACE_ENTER("BoundingBoxReceive");
  
  bbox = mi_alloc (sizeof(BoundingBox));

  datap = (mi_uchar*) mi_get_vardata ((mi_lvarchar*) data);
  datap = mi_get_double_precision (datap, &(bbox->ll.x));
  datap = mi_get_double_precision (datap, &(bbox->ll.y));
  datap = mi_get_double_precision (datap, &(bbox->ll.z));
  datap = mi_get_double_precision (datap, &(bbox->ur.x));
  datap = mi_get_double_precision (datap, &(bbox->ur.y));
  datap = mi_get_double_precision (datap, &(bbox->ur.z));
  
  BBOX_TRACE_EXIT("BoundingBoxReceive");

  return bbox;
}

MI_DECL mi_sendrecv *
BoundingBoxSend (BoundingBox *bbox)
{
  mi_sendrecv *retval;
  mi_uchar    *datap;
  
  BBOX_TRACE_ENTER("BoundingBoxSend");

  retval = (mi_sendrecv*) mi_new_var (sizeof(BoundingBox));
  datap = (mi_uchar*) mi_get_vardata ((mi_lvarchar*) retval);

  datap = mi_put_double_precision (datap, &(bbox->ll.x));
  datap = mi_put_double_precision (datap, &(bbox->ll.y));
  datap = mi_put_double_precision (datap, &(bbox->ll.z));
  datap = mi_put_double_precision (datap, &(bbox->ur.x));
  datap = mi_put_double_precision (datap, &(bbox->ur.y));
  datap = mi_put_double_precision (datap, &(bbox->ur.z));
  
  BBOX_TRACE_EXIT("BoundingBoxSend");

  return retval;
}

MI_DECL BoundingBox *
BoundingBoxImportText (mi_lvarchar *input)
{
  return BoundingBoxInput (input);
}

MI_DECL mi_lvarchar *
BoundingBoxExportText (BoundingBox *bbox)
{
  return BoundingBoxOutput (bbox);
}

MI_DECL BoundingBox *
BoundingBoxImportBinary (mi_sendrecv *input)
{
  return BoundingBoxReceive (input);
}

MI_DECL mi_sendrecv *
BoundingBoxExportBinary (BoundingBox* bbox)
{
  return BoundingBoxSend (bbox);
}

MI_DECL mi_integer
BoundingBoxUnion (BoundingBox *bbox1,
                  BoundingBox *bbox2,
                  BoundingBox *out)
{
  BBOX_TRACE_ENTER("BoundingBoxUnion");

  out->ll.x = (bbox1->ll.x < bbox2->ll.x) ? bbox1->ll.x : bbox2->ll.x;
  out->ll.y = (bbox1->ll.y < bbox2->ll.y) ? bbox1->ll.y : bbox2->ll.y;
  out->ll.z = (bbox1->ll.z < bbox2->ll.z) ? bbox1->ll.z : bbox2->ll.z;
  out->ur.x = (bbox1->ur.x > bbox2->ur.x) ? bbox1->ur.x : bbox2->ur.x;
  out->ur.y = (bbox1->ur.y > bbox2->ur.y) ? bbox1->ur.y : bbox2->ur.y;
  out->ur.z = (bbox1->ur.z > bbox2->ur.z) ? bbox1->ur.z : bbox2->ur.z;
  
  BBOX_TRACE_EXIT("BoundingBoxUnion");

  return MI_OK;
}

MI_DECL mi_integer
BoundingBoxSize (BoundingBox *bbox, 
                 mi_double *size)
{
  mi_double dx;
  mi_double dy;
  mi_double dz;
  
  BBOX_TRACE_ENTER("BoundingBoxSize");

  dx = bbox->ur.x - bbox->ll.x;
  dy = bbox->ur.y - bbox->ll.y;
  dz = bbox->ur.z - bbox->ll.z;

  if (dx<0. && dy<0. && dz<0.)
  {
    /* No intersection case
     * R-Tree preceded this Size() call with an Inter() call
     * detected no intersection between two bounding boxes
     */
     *size = 0;
  } else {
    /* Normal case.
     * Take care to always return non-zero value. Adding epsilon
     * will account for bounding boxes with zero-length, 
     * zero-width, or zero-height.
     */
    *size = (dx * dy * dz) + DBL_EPSILON;
  }
  
  BBOX_TRACE_EXIT("BoundingBoxSize");

  return MI_OK; 
}

MI_DECL mi_integer
BoundingBoxInter (BoundingBox *bbox1,
                  BoundingBox *bbox2)
{
  mi_db_error_raise (NULL, MI_EXCEPTION, "Inter function not implemented", 0);
  /* NOT REACHED */
  return MI_OK;
}

MI_DECL mi_boolean
BoundingBoxCompare (BoundingBox *bbox1,
                    BoundingBox *bbox2)
{
  return memcmp ((mi_char*) bbox1, (mi_char*) bbox2, sizeof(BoundingBox));
}

MI_DECL mi_boolean
BoundingBoxEqual (BoundingBox *bbox1,
                  BoundingBox *bbox2)
{
  /* Call Compare to perform the comparison */
  return (mi_boolean) (0 == BoundingBoxCompare (bbox1, bbox2));
}

MI_DECL mi_boolean
BoundingBoxNotEqual (BoundingBox *bbox1,
                     BoundingBox *bbox2)
{
  /* Call Compare to perform the comparison */
  return (mi_boolean) (0 != BoundingBoxCompare (bbox1, bbox2));
}

MI_DECL mi_boolean
BoundingBoxOverlap (BoundingBox *bbox1,
                    BoundingBox *bbox2)
{
  mi_boolean retval;
  
  BBOX_TRACE_ENTER("BoundingBoxOverlap");

  retval = (bbox1->ll.x <= bbox2->ur.x && bbox2->ll.x <= bbox1->ur.x &&
            bbox1->ll.y <= bbox2->ur.y && bbox2->ll.y <= bbox1->ur.y &&
            bbox1->ll.z <= bbox2->ur.z && bbox2->ll.z <= bbox1->ur.z);
  
  BBOX_TRACE_EXIT("BoundingBoxOverlap");

  return retval;
}

MI_DECL mi_boolean
BoundingBoxContains (BoundingBox *bbox1,
                     BoundingBox *bbox2)
{
  mi_boolean retval;
  
  BBOX_TRACE_ENTER("BoundingBoxContains");

  retval = (bbox2->ll.x >= bbox1->ll.x && bbox2->ur.x <= bbox1->ur.x &&
            bbox2->ll.y >= bbox1->ll.y && bbox2->ur.y <= bbox1->ur.y &&
            bbox2->ll.z >= bbox1->ll.z && bbox2->ur.z <= bbox1->ur.z); 
 
  // BBOX_TRACE(("BoundingBoxContains retval: %i", retval)); 
  BBOX_TRACE_EXIT("BoundingBoxContains");

  return retval;
}

MI_DECL mi_boolean
BoundingBoxWithin (BoundingBox *bbox1,
                   BoundingBox *bbox2)
{
  mi_boolean retval;
  
  BBOX_TRACE_ENTER("BoundingBoxWithin");

  retval = (bbox1->ll.x >= bbox2->ll.x && bbox1->ur.x <= bbox2->ur.x &&
            bbox1->ll.y >= bbox2->ll.y && bbox1->ur.y <= bbox2->ur.y &&
            bbox1->ll.z >= bbox2->ll.z && bbox1->ur.z <= bbox2->ur.z); 
  
  BBOX_TRACE_EXIT("BoundingBoxWithin");

  return retval;
}

void computeGlobalBoundingBox (
     double x, double y, double z,
     double rotx, double roty, double rotz,
     double scale,
     BoundingBox *localBoundingBox,
     BoundingBox *globalBoundingBox)
{


}

