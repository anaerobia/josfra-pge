static char SCCS[] = "%Z% %M% %I% %G%";
/*************************************************************************
 *
 *N  Module VPFSELEC - VPF SELECTED FEATURES
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     Contains functions for selecting features and primitives from the
 *     VPF data based upon the user-defined view of the data.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     N/A
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   Nov 1991                           DOS Turbo C
 *                     Feb 1992 - Optimized for CD-ROM performance.
 *                     May 1992 - Ported to UNIX
 *                     Oct 1992 - MDB
 *E
 *************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "vpfview.h"
#include "vpfprim.h"
#include "vpfrelat.h"
#include "mapgraph.h"
#include "grprim.h"
#include "vpfproj.h"
#include "vpftidx.h"
#include "vpfdraw.h"
#include "vvselec.h"
#include "vvmisc.h"
#include "vpfquery.h"
#include "vpfspx.h"
#include "vpfprop.h"


/*************************************************************************
 *
 *N  read_selected_features
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    This function reads a saved selection set for a given theme of the
 *    specified view.
 *
 *    NOTE:  This function has "special knowledge" about the structure of
 *    a set.  If that structure is changed, this function must account
 *    for those changes.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     view       <input>==(view_type *) view structure.
 *     themenum   <input>==(int) theme number.
 *     return    <output>==(set_type) set of selected features.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                           DOS Turbo C
 *E
 *************************************************************************/
set_type read_selected_features( view_type *view, int themenum )
{
   set_type selset;
   char path[255], num[11] = "          \0", byte;
   FILE *fp;
   register int i;

   /* Determine path name from theme number */
   strcpy(path,view->path);
   strcat(path,view->name);
   strcat(path,"/sel");
   sprintf(num,"%d",themenum);
   for (i=0;i<(4-strlen(num));i++)
      strcat(path,"0");
   strcat(path,num);

   if (access(path,0) != 0) {
      selset.size = 0;
      selset.buf = NULL;
      return selset;
   }

   /* Read the set */
   if ((fp = fopen(path,"rb")) == NULL) {
      fprintf(stderr,"Error opening %s\n",path);
      selset.size = 0;
      selset.buf = NULL;
      return selset;
   }
   fread( &selset.size, sizeof(int), 1, fp );
   selset.buf = (char *)checkmalloc( ((selset.size/8L)+1L) * sizeof(char) );
   if (selset.buf) {
      fread( selset.buf, sizeof(char), (selset.size/8L)+1L, fp );
   } else {
      fprintf(stderr,"Memory allocation error in read_selected_features()\n");
      exit(1);
   }
   fclose(fp);


   return selset;
}


/*************************************************************************
 *
 *N  save_selected_features
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    This function saves the selection set for a given theme of the
 *    specified view to a file in the view directory.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     view     <input>==(view_type *) view structure.
 *     themenum <input>==(int) theme number.
 *     selset   <input>==(set_type) set of selected features.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                           DOS Turbo C
 *E
 *************************************************************************/
void save_selected_features( view_type *view, int themenum,
			     set_type selset )
{
   char path[80], num[11]="          \0", byte;
   FILE *fp;
   register int i;

   /* Determine the path name from the theme number */
   strcpy(path,view->path);
   strcat(path,view->name);
   strcat(path,"/sel");
   sprintf(num,"%d",themenum);
   for (i=0;i<(4-strlen(num));i++)
      strcat(path,"0");
   strcat(path,num);

   /* Write the set */
   fp = fopen(path,"wb");
   if (!fp) return;
   fwrite( &(selset.size), sizeof(int), 1, fp );
   fwrite( selset.buf, sizeof(char), (selset.size/8L)+1L, fp );
   fclose(fp);
}


/*************************************************************************
 *
 *N  get_selected_features
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    This function gets the selection set for a given theme of the
 *    specified view, either by querying the table, or by reading a
 *    previously saved selection set file.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     view     <input>==(view_type *) view structure.
 *     themenum <input>==(int) theme number.
 *     return  <output>==(set_type) set of selected features.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                           DOS Turbo C
 *E
 *************************************************************************/
set_type get_selected_features( view_type *view, int themenum )
{
   set_type selset;
   vpf_table_type ft;
   register int i;

   /* Read a selection set, if present */
   if (strcmp(view->name,"") != 0) {
      selset = read_selected_features( view, themenum );
      if (selset.size > 0) return selset;
   }

   /* No selection set... */

   /* Query the feature table */
   ft = vpf_open_table( view->theme[themenum].ftable, disk, "rb", NULL );
   if (!ft.fp) {
      fprintf(stderr,"get_selected_features: Error opening %s\n",
	      view->theme[themenum].ftable);
      selset.size=0;
      selset.buf = NULL;
      return selset;
   }
   selset = query_table( view->theme[themenum].expression, ft );
   vpf_close_table( &ft );

   /* Save the selection set so we don't have to query again */
   if (strcmp(view->name,"") != 0) {
      save_selected_features( view, themenum, selset );
   }

   return selset;
}

 
/*************************************************************************
 *
 *N  read_selected_primitives
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    This function returns the primitive selection set for a given theme 
 *    of the specified view by reading a previously saved selection set 
 *    file.  If the file is not present, a zero-sized set is returned.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     view     <input>==(view_type *) view structure.
 *     themenum <input>==(int) theme number.
 *     tile     <input>==(int) tile number of the primitive set.
 *     pclass   <input>==(int) primitive class.
 *     return  <output>==(set_type) set of selected features.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   July 1993                          DOS Turbo C
 *E
 *************************************************************************/
set_type read_selected_primitives( view_type *view, int themenum,
                                   int tile, int pclass )
{
   set_type selset;
   char path[255], tpath[255], num[25], byte;
   char *pname[] = {"","edg","fac","txt","end","cnd"};
   FILE *fp;
   register int i;
   int pos, offset;
 
   selset.size = 0;
   selset.buf = NULL;
 
   /* Determine path name from theme number */
   strcpy(path,view->path);
   strcat(path,view->name);
   strcat(path,"/");
   strcat(path,pname[pclass]);
   sprintf(num,"%d",themenum);
   for (i=0;i<(5-strlen(num));i++)
      strcat(path,"0");
   strcat(path,num);
 
   strcpy(tpath,path);
   strcat(tpath,".til");
 
   if ((fileaccess(path,0) != 0) || (fileaccess(tpath,0) != 0)) {
      return selset;
   }
 
   if (tile) {
      /* Find the file offset for the tile */
      if ((fp = fopen(tpath,"rb")) == NULL) {
         return selset;
      }
      offset = (tile-1L)*(sizeof(int));
      fseek(fp,offset,SEEK_SET);
      fread(&pos,sizeof(int),1,fp);
      fclose(fp);
      if (pos<0) {
         return selset;
      }
   } else {
      pos = 0; 
   }

   /* Read the set */
   if ((fp = fopen(path,"rb")) == NULL) {
      return selset;
   }
   fseek(fp,pos,SEEK_SET);
   fread( &selset.size, sizeof(int), 1, fp );
   selset.buf = (char *)checkmalloc( ((selset.size/8L)+1L) * sizeof(char));
   fread( selset.buf, sizeof(char), (selset.size/8L)+1L, fp );

   fclose(fp);
 
   return selset;
}

#ifndef __MSDOS__
/* 
UNIX version of the function to return the length of an open file
in bytes.
*/
static int filelength( int fd )
{
   struct stat statbuf ;

   if ( fstat ( fd, &statbuf ) < 0 ) return 0;
   return statbuf.st_size ;
}
#endif

/*************************************************************************
 *
 *N  save_selected_primitives
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    This function saves the primitive selection set for a given tile
 *    and primitive class for a theme in a view.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     view     <input>==(view_type *) view structure.
 *     themenum <input>==(int) theme number.
 *     tile     <input>==(int) tile number.
 *     ntiles   <input>==(int) number of tiles in the library.
 *     selset   <input>==(set_type) set of selected primitives.
 *     pclass   <input>==(int) primitive class.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                           DOS Turbo C
 *E
 *************************************************************************/
void save_selected_primitives( view_type *view, int themenum,
                               int tile, int ntiles,
                               set_type selset, int pclass )
{
   char path[255], tpath[255], num[25], byte;
   FILE *fp, *tfp;
   register int i;
   char *pname[] = {"","edg","fac","txt","end","cnd"};
   int pos, size;
 
   /* Determine the path name from the theme number */
   strcpy(path,view->path);
   strcat(path,view->name);
   strcat(path,"/");
   strcat(path,pname[pclass]);
   sprintf(num,"%d",themenum);
   for (i=0;i<(5-strlen(num));i++)
      strcat(path,"0");
   strcat(path,num);
 
   strcpy(tpath,path);
   strcat(tpath,".til");
 
   /* Open files */
   fp = fopen(path,"a+b");
   if (!fp) {
      return;
   }
 
   if (fileaccess(tpath,0) != 0) {
      /* Create tile index file for the theme */
      tfp = fopen(tpath,"w+b");
      if (!tfp) {
         fclose(fp);
         return;
      }
      pos = -1;
      if (!ntiles) ntiles=1;
      for (i=0;i<ntiles;i++) {
         fwrite(&pos,sizeof(int),1,tfp);
      }
      fclose(tfp);
   }
   tfp = fopen(tpath,"r+b");
   if (!tfp) {
      fclose(fp);
      return;
   }
 
   if (tile) {
      /* Need to check tile index to make sure that the tile does not
         already exist in the selection file.  If it does and the size
         is the same (or less), write over the existing set. */
      fseek(tfp,(tile-1L)*sizeof(int),SEEK_SET);
      fread(&pos,sizeof(int),1,tfp);
      if (pos >= 0) {
         /* The set for the tile is already present */
         fseek(fp,pos,SEEK_END);
         fread( &size, sizeof(int), 1, fp );
         if (size > selset.size) {
            /* This is actually an error.  Just leave the existing set 
               in place and add the new one to the end. */
            pos = -1;
         }
      }
    
      /* Write the tile record to the index */
      if (pos<0) pos = filelength(fileno(fp));
      fseek(tfp,(tile-1L)*sizeof(int),SEEK_SET);
      fwrite(&pos,sizeof(int),1,tfp);
   }
   fclose(tfp);
 
   /* Write the set */
   fseek(fp,0L,SEEK_END);
   fwrite( &(selset.size), sizeof(int), 1, fp );
   fwrite( selset.buf, sizeof(char), (selset.size/8L)+1L, fp );
   fclose(fp);
}   
 

 

/* Return the extent in decimal degrees after it has been projected
   in the given coordinate system.  */
static extent_type project_extent( extent_type extent, 
                                   vpf_projection_type proj )
{
   extent_type pextent;
   double xtemp,ytemp;
   vpf_projection_type saveproj;

   if (proj.code == DD) return extent;

   saveproj = get_vpf_forward_projection();

   pextent.x1 = extent.x1;
   pextent.y1 = extent.y1;
   pextent.x2 = extent.x2;
   pextent.y2 = extent.y2;

   set_vpf_forward_projection(proj);
   set_vpf_inverse_projection(proj);

   mapxy(0,0,&xtemp,&ytemp);
   pextent.x1 = (double)Min(pextent.x1,xtemp); 
   pextent.y1 = (double)Min(pextent.y1,ytemp); 
   pextent.x2 = (double)Max(pextent.x2,xtemp); 
   pextent.y2 = (double)Max(pextent.y2,ytemp); 

   mapxy(gpgetmaxx(),gpgetmaxy(),&xtemp,&ytemp);
   pextent.x1 = (double)Min(pextent.x1,xtemp); 
   pextent.y1 = (double)Min(pextent.y1,ytemp); 
   pextent.x2 = (double)Max(pextent.x2,xtemp); 
   pextent.y2 = (double)Max(pextent.y2,ytemp); 

   mapxy(0,gpgetmaxy(),&xtemp,&ytemp);
   pextent.x1 = (double)Min(pextent.x1,xtemp); 
   pextent.y1 = (double)Min(pextent.y1,ytemp); 
   pextent.x2 = (double)Max(pextent.x2,xtemp); 
   pextent.y2 = (double)Max(pextent.y2,ytemp); 

   mapxy(gpgetmaxx(),0,&xtemp,&ytemp);
   pextent.x1 = (double)Min(pextent.x1,xtemp); 
   pextent.y1 = (double)Min(pextent.y1,ytemp); 
   pextent.x2 = (double)Max(pextent.x2,xtemp); 
   pextent.y2 = (double)Max(pextent.y2,ytemp); 

   mapxy(gpgetmaxx()/2,0,&xtemp,&ytemp);
   pextent.x1 = (double)Min(pextent.x1,xtemp); 
   pextent.y1 = (double)Min(pextent.y1,ytemp); 
   pextent.x2 = (double)Max(pextent.x2,xtemp); 
   pextent.y2 = (double)Max(pextent.y2,ytemp); 

   mapxy(gpgetmaxx()/2,gpgetmaxy(),&xtemp,&ytemp);
   pextent.x1 = (double)Min(pextent.x1,xtemp); 
   pextent.y1 = (double)Min(pextent.y1,ytemp); 
   pextent.x2 = (double)Max(pextent.x2,xtemp); 
   pextent.y2 = (double)Max(pextent.y2,ytemp); 

   mapxy(gpgetmaxx(),gpgetmaxy()/2,&xtemp,&ytemp);
   pextent.x1 = (double)Min(pextent.x1,xtemp); 
   pextent.y1 = (double)Min(pextent.y1,ytemp); 
   pextent.x2 = (double)Max(pextent.x2,xtemp); 
   pextent.y2 = (double)Max(pextent.y2,ytemp); 

   mapxy(0,gpgetmaxy()/2,&xtemp,&ytemp);
   pextent.x1 = (double)Min(pextent.x1,xtemp); 
   pextent.y1 = (double)Min(pextent.y1,ytemp); 
   pextent.x2 = (double)Max(pextent.x2,xtemp); 
   pextent.y2 = (double)Max(pextent.y2,ytemp); 

   set_vpf_forward_projection(saveproj);
   set_vpf_inverse_projection(saveproj);

   return pextent;
}

/*************************************************************************
 *
 *N  primitives_within_extent
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Spatially select all of the primitives for a primitive class in a
 *    specified tile that are contained within the specified map extent.
 *    The map extent must be specified in decimal degrees.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     extent   <input>==(extent_type) spatial extent in decimal degrees.
 *     covpath  <input>==(char *) directory path to a VPF coverage.
 *     tiledir  <input>==(char *) tile directory from the tileref.aft.
 *     primclass<input>==(int) primitive class identifier.  Must be
 *                       EDGE, FACE, ENTITY_NODE, CONNECTED_NODE, or TEXT.
 *     numprims <input>==(int) number of rows in the associated
 *                       primitive table.
 *     libproj  <input>==(vpf_projection_type) projection information
 *                       for the current library.
 *     return  <output>==(set_type) set of selected primitives.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                           DOS Turbo C
 *E
 *************************************************************************/
set_type primitives_within_extent( extent_type mapextent,
                                   char *covpath, 
                                   char *tiledir,
                                   int primclass,
                                   int numprims,
                                   vpf_projection_type libproj )
{
   char path[255];
   set_type primitive_rows;
   vpf_projection_type display_proj;
   char *spxname[] = {"","ESI","FSI","TSI","NSI","CSI"};
   char *brname[] = {"","EBR","FBR","TBR","NBR","CBR"};



   primitive_rows.size = 0;
   primitive_rows.buf = (char *)NULL;

   /* Read spatial index */
   strcpy(path,covpath);
   vpfcatpath(path,tiledir);
   vpfcatpath(path,spxname[primclass]);
   /* 20 (below) is a fairly arbitrary cutoff of the number of */
   /* primitives that make a spatial index search worth while. */
   if ((fileaccess(path,0)==0)&&(numprims > 20)) {
      if (libproj.code > DD) {
         display_proj = get_vpf_forward_projection();
         /* Must forward project the map extent (decimal degrees) into
         the projection in which the coordinate data are stored. */
         set_vpf_forward_projection(libproj);
         if (libproj.forward_proj) {
            libproj.forward_proj(&mapextent.x1,&mapextent.y1);
            libproj.forward_proj(&mapextent.x2,&mapextent.y2);
         }
         set_vpf_forward_projection(display_proj);
      }
      primitive_rows = spatial_index_search(path,
                          mapextent.x1,mapextent.y1,
                          mapextent.x2,mapextent.y2);
   } else {
      /* Next best thing - bounding rectangle table */
      strcpy(path,covpath);
      vpfcatpath(path,tiledir);
      vpfcatpath(path,brname[primclass]);
      if ((fileaccess(path,0)==0)&&(numprims > 20)) {
         primitive_rows = bounding_select(path,mapextent,libproj);
      }
   }

   if (primitive_rows.size == 0) {
      /* Search through all the primitives */
      primitive_rows=set_init(numprims+1L);
      set_on(primitive_rows);
   }

   return primitive_rows;
}


/*************************************************************************
 *
 *N  get_selected_tile_primitives
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    This function determines all of the selected primitive rows from
 *    the selected features of a given tile.
 *
 *    This function expects a feature class relationship structure that
 *    has been successfully created with select_feature_class_relate()
 *    from the feature table to the primitive.  The primitive table in
 *    the feature class relate structure must have been successfully
 *    opened for the given tile.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     covpath   <input>==(char *) path th the VPF coverage.
 *     fclass    <input>==(char *) feature class.
 *     expression<input>==(char *) selection expression to apply to the 
 *                        feature table.
 *     fcrel     <input>==(fcrel_type) feature class relate structure.
 *     primclass <input>==(int) primitive class to select.
 *     tile      <input>==(int) tile number.
 *     status   <output>==(int *) status of the function:
 *                         1 if completed, 0 if user escape.
 *     return   <output>==(set_type) set of primitives for the features
 *                         in the corresponding tile.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                           DOS Turbo C
 *E
 *************************************************************************/
set_type get_selected_tile_primitives( char *covpath,
                                       char *fclass,
                                       char *expression,
                                       fcrel_type fcrel,
                                       int primclass,
                                       int tile,
                                       int *status )
{
   int cov, feature, prim;
   row_type row;
   set_type primitives;
   int feature_rownum;
   short int stile;
   register int i, start,end;
   char path[255];
   linked_list_type feature_list;
   position_type p;

   prim = 0;
   feature = fcrel.nchain-1;

   /* Assume that fcrel.table[prim] has been opened */

   primitives = set_init(fcrel.table[prim].nrows+1);

   fseek(fcrel.table[prim].fp,index_pos(1,fcrel.table[prim]),
         SEEK_SET);

   for (i=1;i<=fcrel.table[prim].nrows;i++) {

      row = get_row(i,fcrel.table[prim]);
      if (!row) break;

      feature_list = fc_row_numbers( row, fcrel, tile );

      free_row( row, fcrel.table[prim] );

      if (!ll_empty(feature_list)) {
	 p = ll_first(feature_list);
	 while (!ll_end(p)) {
	    ll_element(p,&feature_rownum);
	    if ((feature_rownum<1)||
		(feature_rownum>fcrel.table[feature].nrows)) {
               p = ll_next(p);
	       continue;
            }
            row = get_row(feature_rownum,fcrel.table[feature]);
	    if (query_table_row( expression, row, 
                                 fcrel.table[feature] )) {
	       set_insert(i,primitives);
               free_row(row,fcrel.table[feature]);
	       break;
	    }
            free_row(row,fcrel.table[feature]);
	    p = ll_next(p);
	 }
      }

      if (feature_list) ll_reset(feature_list);

   }

   *status = 1;

   return primitives;

}


/*************************************************************************
 *
 *N  get_fit_tile_primitives
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Get the set of primitives in the given tile for the selected features
 *    in the given feature class.  Use the Feature Index Table instead of
 *    the schema relationships.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     covpath   <input>==(char *) path th the VPF coverage.
 *     primclass <input>==(int) primitive class to select.
 *     expression<input>==(char *) expression to apply to the feature table.
 *     feature_table <input>==(vpf_table_type) feature table.
 *     tile      <input>==(int) tile number.
 *     fca_id    <input>==(int) Feature Class Attribute table id of the
 *                              selected feature class.
 *     numprims  <input>==(int) number of rows in the specified tile's
 *                              primitive table for the specified primitive
 *                              class.
 *     status   <output>==(int *) status of the function:
 *                         1 if completed, 0 if user escape.
 *     return   <output>==(set_type) set of primitives for the features
 *                         in the corresponding tile.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   May 1991                           DOS Turbo C
 *E
 *************************************************************************/
set_type get_fit_tile_primitives( char *covpath,
				  int primclass,
                                  char *expression,
                                  vpf_table_type feature_table,
                                  int tile,
                                  int fca_id,
                                  int numprims,
                                  int *status )
{
   set_type primitives, tileset, fcset, selset, primitive_rows;
   int i, start, end, prim_id, tile_id, fc_id, feature_id, count;
   short int short_tile_id;
   int PRIM_ID_, TILE_ID_, FC_ID_, FEATURE_ID_;
   vpf_table_type fit;
   row_type row, frow;
   char path[255];
   char *ptable[] = {"","EDG","FAC","TXT","END","CND"};

   primitives = set_init(numprims+1);

   strcpy(path,covpath);
   vpfcatpath(path,ptable[primclass]);
   vpfcatpath(path,".FIT");
   if (fileaccess(path,0) != 0) return primitives;

   fit = vpf_open_table( path, disk, "rb", NULL );
   if (!fit.fp) return primitives;

   TILE_ID_ = table_pos("TILE_ID",fit);
   PRIM_ID_ = table_pos("PRIM_ID",fit);
   FC_ID_ = table_pos("FC_ID",fit);
   if (FC_ID_ < 0) FC_ID_ = table_pos("FCA_ID",fit);
   FEATURE_ID_ = table_pos("FEATURE_ID",fit);
   if ( (TILE_ID_ < 0 && tile) || PRIM_ID_ < 0 || FC_ID_ < 0 ||
        FEATURE_ID_ < 0) {
      fprintf(stderr,"Error in Feature Index Table\n%s\n",path);
      vpf_close_table(&fit);
      *status = 0;
      return primitives;
   }

   /* Look for TILE_ID thematic index */
   tileset.size = 0;
   if (tile) {
      if (fit.header[TILE_ID_].tdx) {
         strcpy(path,covpath);
         vpfcatpath(path,fit.header[TILE_ID_].tdx);
         if (fileaccess(path,0)==0) {
            if (fit.header[TILE_ID_].type == 'I') {
               tile_id = (int)tile;
               tileset = read_thematic_index(path,(char *)&tile_id);
            } else if (fit.header[TILE_ID_].type == 'S') {
               short_tile_id = tile;
               tileset = read_thematic_index(path,(char *)&short_tile_id);
            }
         }
      }
   }
   if (!tileset.size) {
      tileset = set_init(fit.nrows+1);
      set_on(tileset);
      set_delete(0,tileset);
   }

   /* Look for FC_ID thematic index */
   fcset.size = 0;
   if (fit.header[FC_ID_].tdx) {
      strcpy(path,covpath);
      vpfcatpath(path,fit.header[FC_ID_].tdx);
      if (fileaccess(path,0)==0) {
         fc_id = (int)fca_id;
         fcset = read_thematic_index(path,(char *)&fc_id);
      }
   }
   if (!fcset.size) {
      fcset = set_init(fit.nrows+1);
      set_on(fcset);
      set_delete(0,fcset);
   }
   
   /* Get the set of all FIT rows in the search tile that match the
      search fca_id */
   selset = set_intersection(tileset,fcset);
   set_nuke(&tileset);
   set_nuke(&fcset);

   if (set_empty(selset)) {
      vpf_close_table(&fit);
      set_nuke(&selset);
      *status = 1;
      return primitives;
   }

 
   /* Now loop through the FIT and get the matching primitive ids */
   start = set_min(selset);
   end = set_max(selset);

#ifdef __MSDOS__
   fseek(fit.fp, index_pos(start,fit), SEEK_SET);
#endif

   for (i=start;i<=end;i++) {
#ifdef __MSDOS__
      row = read_next_row(fit);
#else
      if (!set_member(i,selset)) continue;
      row = get_row(i,fit);
#endif
      get_table_element(PRIM_ID_,row,fit,&prim_id,&count);
      get_table_element(FC_ID_,row,fit,&fc_id,&count);
      get_table_element(FEATURE_ID_,row,fit,&feature_id,&count);
      tile_id = 0;
      if (tile) {
         if (fit.header[TILE_ID_].type == 'I') {
            get_table_element(TILE_ID_,row,fit,&tile_id,&count);
         } else if (fit.header[TILE_ID_].type == 'S') {
            get_table_element(TILE_ID_,row,fit,&short_tile_id,&count);
            tile_id = short_tile_id;
         }
      }
      free_row(row,fit);
      if (!set_member(i,selset)) continue;
      if (tile_id != tile  ||  fc_id != fca_id) continue;
      frow = get_row(feature_id,feature_table);
      if (query_table_row(expression,frow,feature_table))
         set_insert(prim_id,primitives);
      free_row(frow,feature_table);
   }

   vpf_close_table(&fit);
   set_nuke(&selset);

   *status = 1;

   return primitives;
}


/*************************************************************************
 *
 *N  draw_selected_features
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    This function draws the selected features from a specified feature
 *    class based upon a query (either an expression or the pre-compiled
 *    results of an expression).
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *     view     <inout>==(view_type *) view structure.
 *     themenum <input>==(int) theme number.
 *     mapenv   <input>==(map_environment_type *) map environment structure.
 *     return  <output>==(int) completion status:
 *                                1 if completed successfully,
 *                                0 if an error occurred.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels   August 1991                        DOS Turbo C
 *                     October 1992    UNIX mdb
 *E
 *************************************************************************/
int draw_selected_features( view_type *view,
                            int themenum,
                            map_environment_type *mapenv )
{
   int status, finished=1, tilecover, TILEPATH_=0, prim, feature;
   int number_relate_paths, relpathnum, ntiles;
   int db, lib, projcode,style;
   vpf_projection_type libproj;
   vpf_table_type rngtable,edgtable,fbrtable, tile_table, fcs;
   vpf_table_type fca;
   int fit=0, fc_id=0;
   row_type row;
   char *ptable[] = {"","EDG","FAC","TXT","END","CND"};
   register int i, j, pclass, tile, tileid;
   int display_order[] = {FACE,EDGE,ENTITY_NODE,CONNECTED_NODE,TEXT};
   register int starttile, endtile, startprim, endprim;
   int count, p;
   char path[255], libpath[255], covpath[255], tiledir[255], *buf, str[255],
	sep[2] = {DIR_SEPARATOR,'\0'}, ftable[255], primfound=0;
   char *drive, rngpath[255],edgpath[255],edxpath[255],fbrpath[255];
   set_type primitives, primitive_rows, tempset;
   fcrel_type fcrel;
   extent_type pextent;
   int ftype[] = {0,LINE,AREA,ANNO,POINT,POINT};

   strcpy( libpath, view->theme[themenum].database );
   rightjust( libpath );
   strcat( libpath, sep );
   vpfcatpath( libpath, view->theme[themenum].library );
   rightjust( libpath );
   strcat( libpath, sep );

   strcpy( covpath, libpath );
   vpfcatpath( covpath, view->theme[themenum].coverage );
   rightjust( covpath );
   strcat( covpath, sep );

   db = -1;
   lib = -1;
   for (i=0;i<view->ndb;i++) {
      strcpy(path,view->database[i].path);
      strcat(path,view->database[i].name);
      rightjust(path);
      if (stricmp(path,view->theme[themenum].database)==0) {
	 db = i;
	 for (j=0;j<view->database[i].nlibraries;j++) {
	    if (stricmp(view->database[i].library[j].name,
		       view->theme[themenum].library)==0) {
	       lib = j;
	       break;
	    }
	 }
	 break;
      }
   }
   if (db < 0 || lib < 0) {
      fprintf(stderr,"draw_selected_features: Invalid theme (%d)\n",
	      themenum);
      return 0;
   }

   /****** Set inverse projection parameters for reading *****/
   projcode = view->database[db].library[lib].projection;
   if (projcode > DD) {
      libproj = library_projection( libpath );
      set_vpf_inverse_projection( libproj );
   } else {
      libproj = NOPROJ;
   }

   /* Look for feature class entry in FCA */
   strcpy(path,covpath);
   vpfcatpath(path,"FCA");
   if (fileaccess(path,0)==0) {
      fca = vpf_open_table(path,disk,"rb",NULL);
      j = table_pos("FCLASS",fca);
      for (i=1;i<=fca.nrows;i++) {
	 row = read_next_row(fca);
	 buf = (char *)get_table_element(j,row,fca,NULL,&count);
	 rightjust(buf);
	 if (stricmp(buf,view->theme[themenum].fc)==0) {
	    fc_id = i;
	    i = fca.nrows+2;
	 }
	 free(buf);
	 free_row(row,fca);
      }
      vpf_close_table(&fca);
   }

   /* Look for primitives at the coverage level -> untiled coverage */
   primfound = FALSE;
   for (p=EDGE;p<=CONNECTED_NODE;p++) {
      strcpy(path,covpath);
      vpfcatpath(path,ptable[p]);
      if (fileaccess(path,0) == 0) {
         primfound = TRUE;
         break;
      }
   }

   /* If the coverage is tiled, open the TILEREF.AFT table */
   strcpy(path,libpath);
   vpfcatpath(path,"TILEREF\\TILEREF.AFT");
   if ((fileaccess(path,0) != 0) || (primfound)) {
      tilecover = FALSE;
      ntiles = 1;
   } else {
      tile_table = vpf_open_table(path,disk,"rb",NULL);
      TILEPATH_ = table_pos("TILE_NAME",tile_table);
      tilecover = TRUE;
      ntiles = tile_table.nrows;
   }

   for (p=0;p<5;p++) {
      pclass = display_order[p];

      if ((pclass==EDGE) && 
         (!view->theme[themenum].primclass.edge)) continue;
      if ((pclass==FACE) && 
          (!view->theme[themenum].primclass.face)) continue;
      if ((pclass==TEXT) && 
          (!view->theme[themenum].primclass.text)) continue;
      if ((pclass==ENTITY_NODE) && 
          (!view->theme[themenum].primclass.entity_node)) continue;
      if ((pclass==CONNECTED_NODE) && 
          (!view->theme[themenum].primclass.connected_node)) continue;

      set_theme_symbology( view, themenum, ftype[pclass] );

      /* Get feature table name without directory path */
      j = strlen(view->theme[themenum].ftable)-1;
      while (view->theme[themenum].ftable[j] != DIR_SEPARATOR && j>0) j--;
      if (view->theme[themenum].ftable[j] == DIR_SEPARATOR)
         strcpy(ftable,&view->theme[themenum].ftable[j+1]);
      else
         strcpy(ftable,view->theme[themenum].ftable);
      rightjust(ftable);

      /*** Look for Feature Index Table (FIT) ***/
      strcpy(path,covpath);
      vpfcatpath(path,ptable[pclass]);
      vpfcatpath(path,".FIT");
      fit = (fileaccess(path,0)==0);

      /* Set up the feature class table relate chain.        */
      /* The feature table is fcrel.table[0].                */
      /* The primitive table is the last table in the chain: */
      /*    fcrel.table[ fcrel.nchain-1 ].                   */

      if (fit) {
	 number_relate_paths = 1;
      } else {
	 strcpy( path, covpath );
	 vpfcatpath( path, "FCS" );
	 fcs = vpf_open_table(path,disk,"rb",NULL);
	 number_relate_paths = num_relate_paths( ptable[pclass], ftable,
						 view->theme[themenum].fc,
						 fcs );
	 vpf_close_table(&fcs);
      }

      for (relpathnum=0;relpathnum<number_relate_paths;relpathnum++) {

	 /* Initialize the feature class relates */
	 fcrel = select_feature_class_relate(covpath,
				             view->theme[themenum].fc,
					     ptable[pclass], ftable,
					     relpathnum);
	 prim = 0;
         feature = fcrel.nchain-1;

	 /*** 'Tile' number 1 is the universe polygon for
	       the tileref cover ***/
	 starttile = set_min(view->database[db].library[lib].tile_set);
	 if (starttile < 1) starttile = 1;
	 endtile = set_max(view->database[db].library[lib].tile_set);
	 if (endtile < 1) endtile = 1;
         if (primfound) starttile=endtile=1;
	 for (tile = starttile; tile <= endtile; tile++ ) {
	    if (tilecover) {
	       if (!set_member(tile,view->database[db].library[lib].tile_set))
	          continue;
	       row = get_row(tile,tile_table);
	       buf = (char *)get_table_element(TILEPATH_,row,tile_table,
					     NULL,&count);
	       free_row(row,tile_table);
	       strcpy(tiledir,buf);
	       rightjust(tiledir);
	       strcat(tiledir,sep);
	       free(buf);
               tileid = tile;
	    } else {
	       strcpy(tiledir,"");
               tileid = 0;
	    }

	    finished = TRUE;

	    strcpy(path,covpath);
	    vpfcatpath(path,tiledir);
	    vpfcatpath(path,ptable[pclass]);
	    if (fileaccess(path,0) != 0) continue;
#ifdef __MSDOS__
	    fcrel.table[prim] = vpf_open_table(path,disk,"rb",NULL);
#else
	    fcrel.table[prim] = vpf_open_table(path,ram,"rb",NULL);
#endif

            primitives = read_selected_primitives( view, themenum, 
                                                   tileid, pclass );
            status = 1;
            if (primitives.size == 0) {
               /* primitive set not saved yet - generate and store */

	       if (fit) {
	          primitives = get_fit_tile_primitives( covpath, pclass,
				       view->theme[themenum].expression, 
                                       fcrel.table[feature], tileid, fc_id,
				       fcrel.table[prim].nrows, &status );
	       } else {
	          primitives = get_selected_tile_primitives( covpath,
				            view->theme[themenum].fc,
                                            view->theme[themenum].expression,
					    fcrel, pclass, tile, &status );
               }
               save_selected_primitives( view, themenum, tileid, ntiles,
                                         primitives, pclass );

            }

	    if (primitives.size < 1) {
	       vpf_close_table(&fcrel.table[prim]);
	       continue;
	    }

	    if (set_empty(primitives)) {
	       set_nuke(&primitives);
	       vpf_close_table(&fcrel.table[prim]);
	       continue;
	    }

	    if (!status) {
	       set_nuke(&primitives);
	       vpf_close_table(&fcrel.table[prim]);
	       break;
	    }

            /* Find selected primitives within the extent */
            pextent = project_extent( mapenv->mapextent, mapenv->projection );
            tempset = primitives_within_extent( pextent, covpath, tiledir,
                                                pclass, fcrel.table[prim].nrows,
                                                libproj );
            if (set_empty(tempset)) {
               set_nuke(&tempset);
               vpf_close_table(&fcrel.table[prim]);
               continue;
            }
            primitive_rows = set_intersection( primitives, tempset );
            set_nuke(&tempset);
            set_nuke(&primitives);
            if (set_empty(primitive_rows)) {
               set_nuke(&primitive_rows);
               vpf_close_table(&fcrel.table[prim]);
               continue;
            }
 
	    if (pclass == FACE) {
	       /* Must also open RNG, EDG, and FBR for drawing faces. */
	       buf = (char *)checkmalloc(255);

	       strcpy(path,covpath);
	       vpfcatpath(path,tiledir);
	       vpfcatpath(path,"RNG");
	       rngtable = vpf_open_table(path,disk,"rb",NULL);

	       strcpy(path,covpath);
	       vpfcatpath(path,tiledir);
	       vpfcatpath(path,"EDG");
	       edgtable = vpf_open_table(path,ram,"rb",NULL);

	       strcpy(path,covpath);
	       vpfcatpath(path,tiledir);
	       vpfcatpath(path,"FBR");
	       fbrtable = vpf_open_table(path,disk,"rb",NULL);

	       free(buf);
	    }

	    finished = 1;

	    if (set_empty(primitive_rows)) {
               /* Should not get here */
	       startprim = 0;
	       endprim = -1;
	    } else {
	       startprim = set_min(primitive_rows);
	       endprim = set_max(primitive_rows);
	    }

	    /* It turns out to be MUCH faster off of a CD-ROM to */
	    /* read each row and discard unwanted ones than to   */
	    /* forward seek past them.  It's about the same off  */
	    /* of a hard disk.  (DOS only)                       */

#ifdef __MSDOS__
	    fseek(fcrel.table[prim].fp,
		  index_pos(startprim,fcrel.table[prim]),
		  SEEK_SET);
#endif

            gpgetlinestyle(&style);

	    for (i=startprim;i<=endprim;i++) {

#ifdef __MSDOS__
	       row = read_next_row(fcrel.table[prim]);
#endif

	       if (set_member( i, primitive_rows )) {

#ifndef __MSDOS__
                  row = get_row(i,fcrel.table[prim]);
#endif

		  /* Draw the primitive */
		  switch (pclass) {
		     case EDGE:
			draw_edge_row(row,fcrel.table[prim],libproj,
                                      mapenv->projection);
			break;
		     case ENTITY_NODE:
		     case CONNECTED_NODE:
			draw_point_row(row,fcrel.table[prim],libproj,
                                       mapenv->projection);           
			break;
		     case FACE:
                        if (i > 1) {
			   draw_face_row( row,fcrel.table[prim],
				          rngtable, edgtable, fbrtable,
                                          libproj, mapenv->projection );
                           if (style != NoLine)
                              outline_face(i,fcrel.table[prim],
                                           rngtable,edgtable,
                                           libproj,mapenv->projection);
                        }
			break;
		     case TEXT:
			draw_text_row(row,fcrel.table[prim],libproj,
                                      mapenv->projection);
			break;
		  }

#ifndef __MSDOS__
                  free_row(row,fcrel.table[prim]);
#endif

	       }

#ifdef __MSDOS__
	       free_row(row,fcrel.table[prim]);
#endif

               finished = 1;
	       if (event_escape(COVERAGE_WINDOW)) {
		  finished = 0;
		  status = 0;
		  break;
	       }

	    }

	    if (pclass == FACE) {
	       vpf_close_table(&rngtable);
	       vpf_close_table(&edgtable);
	       vpf_close_table(&fbrtable);
	    }

	    vpf_close_table(&fcrel.table[prim]);

	    set_nuke(&primitive_rows);

	    if (!finished) {
	       status = 0;
	       break;
	    }

	 }

	 if (!finished) {
	    status = 0;
	    deselect_feature_class_relate( &fcrel );
	    break;
	 }

	 status = 1;

	 deselect_feature_class_relate(&fcrel);

      } /* relpathnum loop */

   } /* primclass loop */

   if (tilecover) {
      vpf_close_table(&tile_table);
   }

   return status;
}




