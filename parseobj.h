/*
 * Sean James
 *
 * parseobj.h
 *
 * parseObj() function for parsing a .obj model into an array of PolygonShapes.
 *
 */

#ifndef _PARSEOBJ_H
#define _PARSEOBJ_H

#include "defs.h"

#include "shape.h"
#include "polygonshape.h"
#include "timer.h"
#include "alignedlist.h"

/*
 * Parse the file specified by 'filename' as a .obj model. Sets the pointer
 * out_polys to an array of PolygonShapes, sets out_npolys to the number of polygons,
 * and sets out_bbox to a bounding box for the polygons. The caller is responsible
 * for freeing out_polys when done.
 *
 * Polys must be aligned to 16
 *
 */
bool parseObj(char *filename, AlignedList<PolygonShape> & polys, AABB *out_bbox);

#endif
