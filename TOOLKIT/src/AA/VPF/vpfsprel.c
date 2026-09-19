
/*************************************************************************
*
*N  VPFSPREL - VPF Spatial Relationship Module
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*   Contains functions for performing spatial relationship functions using
*   some of the topological information stored in VPF primtive tables.  It
*   includes functions for determining whether a search point is contained
*   within a face (point in polygon), determining the perpendicular distance
*   to an edge (near), and for performing a simple geometric clip on an edge
*   table to a specified a geographic extent.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     N/A
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Barry Michaels     Oct 1992                          GNU C
*E
*************************************************************************/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "vpfsprel.h"
#include "vpfprim.h"


/*
 *  This function determines the next edge for the specified face from
 *  the given edge record and face id.  It follows the rules of
 *  winged-edge topology, but does not trace the network through
 *  dangling edges; rather, it skips over them to stay along the ring
 *  boundaries.
 */
static int next_face_edge( edge_rec_type *edge_rec, 
                                int *prevnode,
                                int face_id )
{
  int next;

  if ( (edge_rec->right_face == face_id) && (edge_rec->left_face == face_id) ) {    /* Dangle - go the opposite direction to continue along the boundary */
    if (*prevnode == edge_rec->start_node) {
      edge_rec->dir = '-';
      next = edge_rec->left_edge;
      *prevnode = edge_rec->start_node;
    } else if (*prevnode == edge_rec->end_node) {
      edge_rec->dir = '+';
      next = edge_rec->right_edge;
      *prevnode = edge_rec->end_node;
    } else next = -1;
  } else if (edge_rec->right_face == face_id) {
    /* The face is on the right - take the right forward edge */
    next = edge_rec->right_edge;
    edge_rec->dir = '+';
    *prevnode = edge_rec->end_node;
  } else if (edge_rec->left_face == face_id) {
    /* The face is on the left - take the left forward edge */
    next = edge_rec->left_edge;
    edge_rec->dir = '-';
    *prevnode = edge_rec->start_node;
  } else next = -1;
    
  return next;
}


/***************************************************************************
 *
 *N  intersect_polygon_edge
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: 
 *
 *   Purpose:
 *P
 *    Count the number of intersections between a plum line from the
 *    test point (x,y) and (infinity,y) and the segments of the given edge.
 *    If the plum line intersects at a vertex, the count is the number of
 *    segments at the vertex.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   Parameters:
 *A
 *    x   <input> == (double) test point x coordinate.
 *    y   <input> == (double) test point y coordinate.
 *    edge_rec <input> == (edge_rec_type) given edge.
 *    intersect_polygon_edge <output> == (int) number of intersection points.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   History:
 *H
 *    Barry Michaels   Oct 1992 
 *E
 **************************************************************************/
int intersect_polygon_edge( double x, double y, edge_rec_type edge_rec )
{
   register int i;
   line_segment_type lseg, pseg;
   int n;
   double xint,yint;
   double_coordinate_type coord1,coord2;
 
   lseg.x1 = x;
   lseg.y1 = y;
/* This line has been changed so that code works on a dec= lseg.x2 += MAXFLOAT/2.0; 
   lseg.x2 += (3.40282346638528860e+38/2.0); */
   lseg.x2 = 20e+10;
   lseg.y2 = y;
 
   n = 0;
 
   coord1 = first_edge_coordinate(&edge_rec);
   for (i=1;i<edge_rec.npts;i++) {
      coord2 = next_edge_coordinate(&edge_rec);
      pseg.x1 = coord1.x;
      pseg.y1 = coord1.y;
      pseg.x2 = coord2.x;
      pseg.y2 = coord2.y;
 
      if (intersect(lseg,pseg,&xint,&yint)) {
	if (!(xint == coord2.x && yint == coord2.y))
         n++;
      }
 
      coord1 = coord2;
   }
   return n;

}

/***************************************************************************
 *
 *N  intersect_polygon_loop
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: 
 *
 *   Purpose:
 *P
 *    Count the number of intersections of a plum line from the test
 *    point to infinity and each of the edges in a polygon loop (ring).
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   Parameters:
 *A
 *    x  <input> == (double) test point x coordinate.
 *    y  <input> == (double) test point y coordinate.
 *    face_id <input> == (int) given polygon loop.
 *    start_edge <input> == (int) one edge of the polygon loop.
 *    edge_table <input> == (vpf_table_type) VPF edge table.
 *    projfunc   <input> == (int *) inverse projection function pointer.
 *    intersect_polygon_loop <output> == (int) number of intersections.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   Oct 1992
 *E
 **************************************************************************/
int intersect_polygon_loop( double x, double y, int face_id,
			    int start_edge, vpf_table_type edgetable,
                            int (*projfunc)(double *, double *) )
{
   edge_rec_type edge_rec;
   int next, prevnode;
   int done = FALSE;
   int n;
 
   edge_rec = read_edge( start_edge, edgetable, projfunc );
 
   edge_rec.dir = '+';
 
   n = intersect_polygon_edge( x, y, edge_rec );
 
   prevnode = edge_rec.start_node;
 
   next = next_face_edge(&edge_rec,&prevnode,face_id);
 
   if (edge_rec.coords) free(edge_rec.coords);
 
   while (!done) {
      if (next <= 0) done = TRUE;
      if (next == start_edge) done = TRUE;
      if (!done) {
         edge_rec = read_edge( next, edgetable, projfunc );
 
         next = next_face_edge(&edge_rec,&prevnode,face_id);
 
         n += intersect_polygon_edge( x, y, edge_rec );
 
         if (edge_rec.coords) free(edge_rec.coords);
      }
   }
 
   return n;
}

/***************************************************************************
 *
 *N  point_in_face
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: 
 *
 *   Purpose:
 *P
 *    Test whether a line (x,y through infinity) intersects a face (which
 *    itself is multiple polygon_loop or ring structures).  Returns a 0
 *    if outside, 1 if inside.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   Parameters:
 *A
 *    x         <input> == (double) x coordinate of given point.
 *    y         <input> == (double) y coordinate of given point.
 *    face_id   <input> == (int) given face.
 *    facetable <input> == (vpf_table_type) VPF face table.
 *    ringtable <input> == (vpf_table_type) VPF ring table.
 *    edgetable <input> == (vpf_table_type) VPF edge table.
 *    projfunc  <input> == (int *) inverse projection function pointer.
 *    point_in_face <output> == (int) boolean:
 *                                            1 --> inside
 *                                            0 --> outside
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   Oct 1992
 *E
 **************************************************************************/
int point_in_face( double x, double y, int face_id, 
                   vpf_table_type facetable,
		   vpf_table_type ringtable, 
                   vpf_table_type edgetable,
                   int (*projfunc)(double *, double *) )
{
   face_rec_type face_rec;
   ring_rec_type ring_rec;
   int n;
 
   face_rec = read_face( face_id, facetable );
   ring_rec = read_ring( face_rec.ring, ringtable );
   n = intersect_polygon_loop( x, y, face_id, ring_rec.edge, edgetable,
                               projfunc );
 
   while (ring_rec.face == face_id ) {
      ring_rec = read_next_ring( ringtable );
 
      if (feof(ringtable.fp)) {
         break;
      }
 
      if (ring_rec.face == face_id) {
         n += intersect_polygon_loop( x, y, face_id,
                                      ring_rec.edge, edgetable,
                                      projfunc );
      }
   }
 
   if (n%2 == 0)   /* Even number of intersections */
      return 0;    /*   Not inside polygon         */
   else            /* Odd number of intersections  */
      return 1;    /*    Inside polygon            */
}

/* Truncate a file name path down to its directory string component */
static void dirpath( char *path )
{                   
   register int i;
 
   i = strlen(path)-1;
   while ( (i>0) && (path[i] != DIR_SEPARATOR) ) i--;
   if (i<(strlen(path)-1)) i++;
   path[i] = '\0';
}

/***************************************************************************
 *
 *N  point_in_face_table
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: 
 *
 *   Purpose:
 *P
 *    Tests whether a point (x,y) is inside of a face or not.  The face id
 *    and the path name to the face table must be specified.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: 
 *   Parameters:
 *A
 *    x        <input> == (double) given point x coordinate.
 *    y        <input> == (double) given point y coordinate.
 *    face_id  <input> == (int) face.
 *    fname    <input> == (char *) pathname to VPF face table.
 *    projfunc <input> == (int *) inverse projection function pointer.
 *    point_in_face_table <output> == boolean:
 *                                            1 --> point is inside
 *                                            0 --> point is not inside
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   Oct 1992
 *E
 **************************************************************************/
int point_in_face_table( double x, double y, int face_id, 
                         char *facepath, int (*projfunc)(double *, double *) )
{
   vpf_table_type facetable, ringtable, edgetable;
   char *name;
   int result;
 
   name = (char *)malloc( 255*sizeof(char) );
 
   facetable = vpf_open_table(facepath, disk, "rb", NULL );
 
#ifdef __MSDOS__
   strupr(fname);
#endif
 
   strcpy( name, facepath );
   dirpath( name );
   strcat( name, "rng" );
   ringtable = vpf_open_table( name, disk , "rb", NULL );
 
   strcpy( name, facepath );
   dirpath( name );
   strcat( name, "edg" );
   edgetable = vpf_open_table( name, disk , "rb", NULL );
 
   free( name );
 
   result = point_in_face( x, y, face_id, facetable, ringtable, edgetable,
                           projfunc );
 
   vpf_close_table(&facetable);
   vpf_close_table(&ringtable);
   vpf_close_table(&edgetable);
 
   return result;
}

/**************************************************************************
 *
 *N  distance_to_edge_rec
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *   Computes the minimum distance from the given point to the given edge
 *   record by looking at each segment in the line formed by the edge.
 *   The distance is returned.  The dec_degrees flag signals if the
 *   coordinate data is specified in decimal degrees.  If so, the 
 *   distance is computed using the great circle distance along the
 *   surface of the earth; otherwise, a simple cartesian distance is
 *   computed.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    x           <input> == (double) given point x coordinate.
 *    y           <input> == (double) given point y coordinate
 *    edge_rec    <input> == (edge_rec_type) given edge record.
 *    dec_degrees <input> == (int) flag to indicate if coordinates are
 *                                 in decimal degrees.
 *    units       <input> == (int) distance units specifier.  Must be
 *                                 one of coord_units_type in coorgeom.h.
 *    distance_to_edge_rec <output> == (double) minimum distance to the
 *                                     edge record.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: * 
 *   History:
 *H
 *    Barry Michaels, DOS Turbo C   1991
 *                    UNIX mdb      Nov 1992
 *E
 *************************************************************************/
double distance_to_edge_rec( double x, double y, edge_rec_type edge_rec,
			     int dec_degrees, int units )
{
   register int i;
   line_segment_type lseg;
   double xint,yint, d, dseg, d1, d2;
   double_coordinate_type coord1, coord2;
 
   d = MAXFLOAT;
 
   coord1 = first_edge_coordinate(&edge_rec);
   for (i=1;i<edge_rec.npts;i++) {
      coord2 = next_edge_coordinate(&edge_rec);
      lseg.x1 = coord1.x;
      lseg.y1 = coord1.y;
      lseg.x2 = coord2.x;
      lseg.y2 = coord2.y;
 
      if (perpendicular_intersection(lseg,x,y,&xint,&yint)) {
         if (dec_degrees)
            dseg = gc_distance( y, x, yint, xint, units );
         else
            dseg = sqrt( (xint-x)*(xint-x) + (yint-y)*(yint-y) );
      } else {
         if (dec_degrees) {
            dseg = (double)Min( gc_distance( y, x, lseg.y1, lseg.x1, 0 ),
                                gc_distance( y, x, lseg.y2, lseg.x2, 0 ) );
         } else {
            d1 = sqrt( (lseg.x1-x)*(lseg.x1-x) + (lseg.y1-y)*(lseg.y1-y) );
            d2 = sqrt( (lseg.x2-x)*(lseg.x2-x) + (lseg.y2-y)*(lseg.y2-y) );
            dseg = (double)Min(d1,d2);
         }
      }
 
      d = (double)Min(d,dseg);
 
      coord1 = coord2;
   }
 
   return d;
}

/**************************************************************************
 *
 *N  distance_to_edge
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Returns the minimum distance from the given point to a specified
 *    edge row id in the given edge table.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    x           <input> == (double) given point x coordinate.
 *    y           <input> == (double) given point y coordinate
 *    edge_id     <input> == (int) specified edge id.
 *    edgetable   <input> == (vpf_table_type) given edge table.
 *    projfunc    <input> == (int *) pointer to the inverse projection
 *                                    function for the coordinates.
 *    units       <input> == (int) distance units specifier.  Must be
 *                                 one of coord_units_type in coorgeom.h.
 *    distance_to_edge <output> == (double) minimum distance to the
 *                                  specified edge from the given point.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   History:
 *H
 *    Barry Michaels, DOS Turbo C   1991
 *                    UNIX mdb   Nov 1992
 *                         Added projection function and units specifier.
 *E
 *************************************************************************/
double distance_to_edge( double x, double y, int edge_id,
			 vpf_table_type edgetable, 
                         int (*projfunc)(double *, double *),
                         int units )
{
   edge_rec_type edge_rec;
   double result;
   int dec_degrees=1;
 
   edge_rec = read_edge( edge_id, edgetable, projfunc );
 
   result = distance_to_edge_rec( x, y, edge_rec, dec_degrees, units );
  
   if (edge_rec.coords) free( edge_rec.coords );
 
   return result;
}

/**************************************************************************
 *
 *N  distance_to_edge_table
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Returns the minimum distance from the given point to a specified
 *    edge row id in the specified edge table.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    x           <input> == (double) given point x coordinate.
 *    y           <input> == (double) given point y coordinate
 *    edge_id     <input> == (int) specified edge id.
 *    edgepath    <input> == (char *) path to the edge table.
 *    projfunc    <input> == (int *) pointer to the inverse projection
 *                                    function for the coordinates.
 *    units       <input> == (int) distance units specifier.  Must be
 *                                 one of coord_units_type in coorgeom.h.
 *    distance_to_edge_table <output> == (double) minimum distance to the
 *                                  specified edge from the given point.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels, DOS Turbo C   1991
 *                    UNIX mdb   Nov 1992
 *                         Added projection function and units specifier.
 *E
 *************************************************************************/
double distance_to_edge_table( double x, double y, int edge_id,
			       char *edgepath, int (*projfunc)(double *, double *),
                               int units )
{
   vpf_table_type edgetable;
   double result;
 
   edgetable = vpf_open_table(edgepath,disk, "rb", NULL);
 
   result = distance_to_edge( x, y, edge_id, edgetable, projfunc, units );
 
   vpf_close_table(&edgetable);
 
   return result;
}



/* Determine if a line segment intersects a box.                   */
/* If so return the intersection coordinate in the parameter list. */
static int box_intersection( line_segment_type lseg,
                             extent_type extent,
                             double_coordinate_type *coord )
{
   line_segment_type boxseg;
   double xint, yint;
 
   boxseg.x1 = extent.x1;
   boxseg.y1 = extent.y1;
   boxseg.x2 = extent.x2;
   boxseg.y2 = extent.y1;
   if (intersect(lseg,boxseg,&xint,&yint)) {
      coord->x = xint;
      coord->y = yint;
      return TRUE;
   }
 
   boxseg.x1 = extent.x2;
   boxseg.y1 = extent.y1;
   boxseg.x2 = extent.x2;
   boxseg.y2 = extent.y2;
   if (intersect(lseg,boxseg,&xint,&yint)) {
      coord->x = xint;
      coord->y = yint;
      return TRUE;
   }
 
   boxseg.x1 = extent.x2;
   boxseg.y1 = extent.y2;
   boxseg.x2 = extent.x1;
   boxseg.y2 = extent.y2;
   if (intersect(lseg,boxseg,&xint,&yint)) {
      coord->x = xint;
      coord->y = yint;
      return TRUE;
   }
 
   boxseg.x1 = extent.x1;
   boxseg.y1 = extent.y2;
   boxseg.x2 = extent.x1;
   boxseg.y2 = extent.y1;
   if (intersect(lseg,boxseg,&xint,&yint)) {
      coord->x = xint;
      coord->y = yint;
      return TRUE;
   }
   return FALSE;
}   
 
/* Replace the given coordinate string into the given edge row */
/* and write the new row to the specified edge table.          */
static void write_edge_record( int id, double_coordinate_type *dcoord,
                               int ncoord, row_type row,
                               vpf_table_type *table )
{
   int ID_, COORD_, i;
   coordinate_type *coord;
   tri_coordinate_type *zcoord;
   double_tri_coordinate_type *ycoord;
 

   ID_ = table_pos("ID",*table);
   COORD_ = table_pos("COORDINATES",*table);
 
   put_table_element(ID_, row, *table, &id, 1);

   switch (table->header[COORD_].type) {
      case 'B':
         put_table_element(COORD_, row, *table, dcoord, ncoord);
         break;
      case 'C':
         coord = (coordinate_type *)malloc(ncoord*sizeof(coordinate_type));
         if (!coord) ncoord = 0; 
         for (i=0;i<ncoord;i++) {
            coord[i].x = dcoord[i].x;
            coord[i].y = dcoord[i].y;
         }
         put_table_element(COORD_, row, *table, coord, ncoord);
         if (coord) free(coord);
         break;
      case 'Z':
         zcoord = (tri_coordinate_type *)malloc(ncoord*
                                         sizeof(tri_coordinate_type));
         if (!zcoord) ncoord = 0; 
         for (i=0;i<ncoord;i++) {
            zcoord[i].x = dcoord[i].x;
            zcoord[i].y = dcoord[i].y;
            zcoord[i].z = NULLFLOAT;
         }
         put_table_element(COORD_, row, *table, zcoord, ncoord);
         if (zcoord) free(zcoord);
         break;
      case 'Y':
         ycoord = (double_tri_coordinate_type *)malloc(ncoord*
                                         sizeof(double_tri_coordinate_type));
         if (!ycoord) ncoord = 0; 
         for (i=0;i<ncoord;i++) {
            ycoord[i].x = dcoord[i].x;
            ycoord[i].y = dcoord[i].y;
            ycoord[i].z = NULLFLOAT;
         }
         put_table_element(COORD_, row, *table, ycoord, ncoord);
         if (ycoord) free(ycoord);
         break;
   }

   write_next_row(row,table);
 
}


#define WITHIN(x,y,ext) ((x>=ext.x1)&&(x<=ext.x2)&& \
                         (y>=ext.y1)&&(y<=ext.y2))
 
/*************************************************************************
 *
 *N  vpf_edge_clip
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function clips an edge table to the given extent and writes
 *     the clipped table to the specified output path.
 *     NOTE: If a segment of an edge in the EDG table has both a beginning
 *     point and ending point outside of the given extent, and the line
 *     segment passes through the extent, the segment will not be written
 *     out to the new edge table.
 *     If no edges intersect, an empty table (header but no rows) will be
 *     created.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   Parameters:
 *A
 *    covpath <input> == (char *) path to the coverage of the edge table
 *                                (including the tile path).
 *    extent  <input> == (extent_type) clipping extent.
 *    outpath <input> == (char *) output path for the clipped edge table.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * 
 *   History:
 *H
 *    Barry Michaels    Dec 1991      Original Version    DOS Turbo C
 *                      Nov 1992      UNIX mdb
 *E
 *************************************************************************/
void vpf_edge_clip( char *covpath, extent_type extent, char *outpath )
{
   vpf_table_type in, out;
   row_type row;
   edge_rec_type inedge, outedge;
   int i,j,n,id, temp;
   line_segment_type lseg;
   double_coordinate_type coord;
   char path[255], *def;
   float xmin, xmax, ymin, ymax;
 
   /* Standardize extent, Lower Left, Upper Right */
   xmin = (double)Min(extent.x1,extent.x2);
   xmax = (double)Max(extent.x1,extent.x2);
   ymin = (double)Min(extent.y1,extent.y2);
   ymax = (double)Max(extent.y1,extent.y2);
   extent.x1 = xmin;
   extent.y1 = ymin;
   extent.x2 = xmax;
   extent.y2 = ymax;
 
   sprintf(path,"%sedg",covpath);
   in = vpf_open_table(path,disk,"rb",NULL);
 
   rewind(in.fp);
/* changed n to int for fread statement */
   fread(&n,sizeof(int),1,in.fp);
   if (in.byte_order != MACHINE_BYTE_ORDER) {
      temp = n;   
      swap_four((char *)&temp,(char *)&n);
   }

   def = (char *)malloc((n+1)*sizeof(char));
   fread(def,sizeof(char),n,in.fp);
   def[n] = '\0';
   sprintf(path,"%sedg",outpath);
   out = vpf_open_table(path,disk,"wb",def);
 
   for (i=1,id=1;i<=in.nrows;i++) {
      row = get_row( i, in );
      inedge = create_edge_rec( row, in, NULL );
      if (!inedge.coords) {
         free_row(row,in);
         continue;
      }
      outedge.coords = (double_coordinate_type *)malloc(inedge.npts*sizeof(
                                                 double_coordinate_type));
      if (!outedge.coords) {
         free(inedge.coords);
         free_row(row,in);
         continue;
      }
 
      n = 0;
 
      for (j=0;j<inedge.npts;j++) {
 
         if (WITHIN(inedge.coords[j].x,inedge.coords[j].y,extent)) {
            /* Current coordinate within extent */
 
            if (j > 0) {
               if (!WITHIN(inedge.coords[j-1].x,inedge.coords[j-1].y,
                            extent)) {
                  /* Previous coordinate was not in the extent - */
                  /* create an intersection vertex */
 
                  lseg.x1 = inedge.coords[j-1].x;
                  lseg.y1 = inedge.coords[j-1].y;
                  lseg.x2 = inedge.coords[j].x;
                  lseg.y2 = inedge.coords[j].y;
                  if (box_intersection(lseg,extent,&coord)) {
                     outedge.coords[n].x = coord.x;
                     outedge.coords[n].y = coord.y;
                  } else {
                     outedge.coords[n].x = lseg.x1;
                     outedge.coords[n].y = lseg.y1;
                  }
                  n++;
               }   
            }
            outedge.coords[n].x = inedge.coords[j].x;
            outedge.coords[n].y = inedge.coords[j].y;
            n++;
         } else {
            if (j > 0) {
               if (WITHIN(inedge.coords[j-1].x,inedge.coords[j-1].y,extent)) {
 
                  /* The coordinate is not within the extent and     */
                  /* the previous one was - Create an intersection   */
                  /* vertex point and write the current edge record. */
 
                  lseg.x1 = inedge.coords[j-1].x;
                  lseg.y1 = inedge.coords[j-1].y;
                  lseg.x2 = inedge.coords[j].x;
                  lseg.y2 = inedge.coords[j].y;
                  if (box_intersection(lseg,extent,&coord)) {
                     outedge.coords[n].x = coord.x;
                     outedge.coords[n].y = coord.y;
                  } else {
                     outedge.coords[n].x = lseg.x2;
                     outedge.coords[n].y = lseg.y2;
                  }
 
                  /* Break the edge in (at least) two */
                  n++;
                  write_edge_record( id, outedge.coords, n, row, &out );
                  id++;
                  n=0;
 
               }
            }
         }     
      }     
      free(inedge.coords);
 
      if (n > 0) {
         write_edge_record( id, outedge.coords, n, row, &out );
         id++;
      }
 
      free(outedge.coords);
      free_row(row,in);
   }
 
   vpf_close_table(&in);
   vpf_close_table(&out);
}



