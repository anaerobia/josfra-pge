static char SCCS[] = "%Z% %M% %I% %G%";
/*************************************************************************
 *
 *N  Module VVSPQRY - Spatial Query
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This module contains functions used to perform spatial query
 *     functions.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    N/A
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels    May 1991                      DOS Turbo C
 *                      June 1992    UNIX Port
 *                      Nov 1992     MDB - Major rewrite
 *E
 *************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#if ( defined(CYGWIN) )
#include </usr/include/mingw/values.h>
#else
#include <values.h>
#endif
#include "vpftable.h"
#include "vpfview.h"
#include "mapgraph.h"
#include "vpfproj.h"
#include "vvgutil.h"
#include "vpfrelat.h"
#include "vvselec.h"
#include "vpftidx.h"
#include "vpfspx.h"
#include "vvmap.h"
#include "grprim.h"
#include "vpfsprel.h"
#include "vpfprim.h"
#include "vpfascii.h"
#include "coorgeom.h"
#include "vpfprop.h"
#include "vpfdraw.h"
#include "vpfquery.h"


typedef struct {
   int prow;
   char db[255];
   char lib[9];
   char cov[9];
   char tiledir[128];
   int tilenum;
   int pclass;
   double x1,y1,x2,y2;
} selection_cell_type;


typedef struct {
   char fclass[20];
   char ftable[255];
   int frow;
} feature_struct_type;


static color_type textcolor = BLACK,
                  markcolor = LIGHTRED,
                  circlecolor = CYAN,
                  outlinecolor = DARKGRAY,
                  boxcolor = CYAN;



/*
Determine if any themes in the specified library of the view have been
displayed.
*/
static int selected_themes_in_library( view_type *view, int db, int lib )
{
   int i;
   char dbstr[255], *libname;

   sprintf(dbstr,"%s%s",view->database[db].path,view->database[db].name);
   libname = view->database[db].library[lib].name;

   for (i=0;i<view->nthemes;i++) {
      if (!set_member(i,view->displayed)) continue;
      if (stricmp(view->theme[i].database,dbstr)==0 &&
          stricmp(view->theme[i].library,libname)==0) return TRUE;
   }
   return FALSE;
}


/*
Generate the list of displayed feature classes for the specified coverage.
Return NULL if no themes have been displayed for the coverage.
*/
static linked_list_type selected_coverage_feature_classes( view_type *view,
                                                           int db, int lib,
                                                           char *coverage )
{
   int i;
   char dbstr[255], *libname, fclass[9];
   linked_list_type fclist;

   sprintf(dbstr,"%s%s",view->database[db].path,view->database[db].name);
   libname = view->database[db].library[lib].name;

   fclist = ll_init();

   for (i=0;i<view->nthemes;i++) {
      if (!set_member(i,view->displayed)) continue;
      if (stricmp(view->theme[i].database,dbstr)==0 &&
          stricmp(view->theme[i].library,libname)==0 &&
          stricmp(view->theme[i].coverage,coverage)==0) {
         memset(fclass,0,9);
         strncpy(fclass,view->theme[i].fc,8);
         if (!ll_locate(fclass,fclist)) {
            ll_insert(fclass,9*sizeof(char),ll_last(fclist));
         }
      }
   }

   if (ll_empty(fclist)) {
      ll_reset(fclist);
      fclist = NULL;
   }

   return fclist;
}


/*
Determine and return the (first) tile in the library containing the 
search point.  If none match, return a tile id of 0.
*/
static void get_search_tile( view_type *view, int db, int lib,
                             double xsearch, double ysearch,
			     int *tile, char *tiledir,
			     vpf_projection_type proj )
{
   int tilenum, facenum;
   short int sval;
   int TILEPATH_, FAC_ID_, XMIN_,YMIN_,XMAX_,YMAX_;
   double plat, plon, dx, dy;
   extent_type tile_extent, pextent;
   char path[255], *libpath, *buf;
   vpf_table_type tile_table, tile_fbr;
   row_type row;
   int n;
   vpf_projection_type libproj;


   libpath = view->database[db].library[lib].path;

   /* Project lat-lon into plate-carree for cartesian box compares */
   libproj = set_vpf_projection_parms( PC, view->extent );
   set_vpf_forward_projection( libproj );
   plat = ysearch;
   plon = xsearch;
   plate_carree_fwd(&plon,&plat);

   *tile = -1;
   strcpy(tiledir,"");

   sprintf(path,"%stileref%ctileref.aft",libpath,DIR_SEPARATOR);
   if (fileaccess(path,0)==0) {
      tile_table = vpf_open_table(path,disk,"rb",NULL);
      TILEPATH_ = table_pos("TILE_NAME",tile_table);
      FAC_ID_ = table_pos("FAC_ID",tile_table);
      sprintf(path,"%stileref%cfbr",libpath,DIR_SEPARATOR);
      tile_fbr = vpf_open_table(path,disk,"rb",NULL);
      XMIN_ = table_pos("XMIN",tile_fbr);
      YMIN_ = table_pos("YMIN",tile_fbr);
      XMAX_ = table_pos("XMAX",tile_fbr);
      YMAX_ = table_pos("YMAX",tile_fbr);
 
      /* Find the tile containing the search point */
      for (tilenum = 1; tilenum <= tile_fbr.nrows; tilenum++) {
 
         if (!set_member(tilenum,view->database[db].library[lib].tile_set))
            continue;
 
         row = read_row(tilenum,tile_table);
         if (FAC_ID_ >= 0) {
            if (tile_table.header[FAC_ID_].type == 'I') {
               get_table_element(FAC_ID_,row,tile_table,&facenum,&n);
            } else if (tile_table.header[FAC_ID_].type == 'S') {
               get_table_element(FAC_ID_,row,tile_table,&sval,&n);
               facenum = (int)sval;
            } else facenum=0;  /* invalid */
         } else {
            facenum = tilenum;
         }
         if (!facenum) continue;

         set_vpf_inverse_projection( proj );
         tile_extent = read_bounding_rect( facenum, tile_fbr,
                                           proj.inverse_proj );
 
         /* Project tile extent into plate-carree for compare */
         dx = tile_extent.x1;
         dy = tile_extent.y1;
         set_vpf_forward_projection(libproj);
         plate_carree_fwd(&dx,&dy);
         pextent.x1 = dx;
         pextent.y1 = dy;
         dx = tile_extent.x2;
         dy = tile_extent.y2;
         plate_carree_fwd(&dx,&dy);
         pextent.x2 = dx;
         pextent.y2 = dy;
         if (fwithin(plon,plat,pextent)) {
            *tile = tilenum;
            buf = (char *)get_table_element(TILEPATH_,row,tile_table,
                                            NULL,&n);
            strcpy(tiledir,buf);
            rightjust(tiledir);
            n = strlen(tiledir);
            tiledir[n+1] = '\0';
            tiledir[n] = DIR_SEPARATOR;
            free(buf);
            free_row(row,tile_table);
            break;
         }
         free_row(row,tile_table);
      }
      vpf_close_table(&tile_fbr);
      vpf_close_table(&tile_table);
       
   } else {
      /* No tiling for the library */
      *tile = 0;
   }

}


/*
Return the set of primitives that have been displayed as features out of
the set of spatially local primitives.
*/
static set_type selected_feature_class_primitives( view_type *view,
                                                   int db, int lib,
                                                   char *coverage,
                                                   int tile,
                                                   char *tiledir,
                                                   int primclass,
                                                   linked_list_type fclist,
                                                   set_type primitives)
{
   set_type selprim, tmpset, tmpset2, all_prim;
   int i;
   int initialized;
   position_type fc;
   char fclass[9], dbstr[255], *libname;

   selprim.size=0;
   selprim.buf = NULL;

   sprintf(dbstr,"%s%s",view->database[db].path,view->database[db].name);
   libname = view->database[db].library[lib].name;

   /* Loop through all feature classes */
   fc = ll_first(fclist);
   initialized = FALSE;
   while (!ll_end(fc)) {
      memset(fclass,0,9);
      ll_element(fc,fclass);
      rightjust(fclass);

      /* Find all primitives belonging to any of the selected themes */
      /* for this feature class.                                   */
      for (i=0;i<view->nthemes;i++) {
         if (!set_member( i, view->displayed)) continue; 
         if (stricmp(view->theme[i].database,dbstr)==0 &&
             stricmp(view->theme[i].library,libname)==0 &&
             stricmp(view->theme[i].coverage,coverage)==0 &&
             stricmp(view->theme[i].fc,fclass)==0) {

            /* Assumes that if a theme has been displayed for the current
               extent, the selection set exists */
            if (!initialized) {
               all_prim = read_selected_primitives( view, i, tile, primclass );
               if (all_prim.size > 0) initialized = TRUE;
            } else {
               tmpset = read_selected_primitives( view, i, tile, primclass );
               if (tmpset.size > 0) {
                  tmpset2 = set_union(tmpset,all_prim);
                  set_assign(&all_prim,tmpset2);
                  set_nuke( &tmpset );
                  set_nuke( &tmpset2 );
               }
            }
         }
      }
      fc = ll_next(fc);
   }

   if (!initialized) {
      /* No themes for this primitive class */
      selprim = set_init(primitives.size);
   } else {
      selprim = set_intersection(primitives,all_prim);
      set_nuke(&all_prim);
   }

   return selprim;
}

/*
Determine if a feature from a feature table has been displayed.
Check the expression for each theme displayed in the current
feature class against the feature row attributes.
*/
static int feature_displayed( int feature_id,
                              view_type *view, char *db, char *lib,
                              char *cov, char *fclass,
                              vpf_table_type ftable )
{
   int i, disp=0;
   row_type frow;

   frow = get_row( feature_id, ftable );
   for (i=0;i<view->nthemes;i++) {
      if (!set_member(i,view->displayed)) continue;
      if (stricmp(view->theme[i].database,db)==0 &&
          stricmp(view->theme[i].library,lib)==0 &&
          stricmp(view->theme[i].coverage,cov)==0 &&
          stricmp(view->theme[i].fc,fclass)==0) {
         if (query_table_row(view->theme[i].expression,frow,ftable))
            disp = 1;
      }
      if (disp) break;
   }
   free_row(frow,ftable);
   return disp;
}

/*
Return a list of feature structures (feature table,rowid) of all features
related to the specified primitive.
*/
static linked_list_type primitive_related_features( view_type *view,
                                                    char *db, char *lib,
                                                    char *coverage,
                                                    int tile,
                                                    char *tiledir,
                                                    int primclass,
                                                    int primitive_id)
{
   set_type tmpset, tmpset2, tileset;
   int i,j,k,n, id,primid,fcid,feature_id,tile_id;
   int tilemin,tilemax, rowstart,rowend, nrelpath,relpathnum;
   short int short_tile;
   int fit=0, initialized=0;
   char path[255],covpath[255],*libpath,tilepath[255];
   char *fctablename, *fctablepath;
   position_type fc, p;
   int PRIM_ID_, TILE_ID_, FC_ID_, FEATURE_ID_, FCLASS_;
   char fclass[9], fcaname[9], *buf;
   char sep[2] = {DIR_SEPARATOR,'\0'};
   char *primname[] = {"","EDG","FAC","TXT","END","CND"};
   vpf_table_type table, fittable, fcatable, fcs, featable;
   row_type row;
   fcrel_type fcrel;
   int prim, feature;
   linked_list_type fclist, feature_list, feature_ids;
   feature_struct_type fstruct;

   feature_list = ll_init();

   fclist = ll_init();
   for (i=0;i<view->nthemes;i++) {
      if (!set_member(i,view->displayed)) continue;
      if (stricmp(view->theme[i].database,db)==0 &&
          stricmp(view->theme[i].library,lib)==0 &&
          stricmp(view->theme[i].coverage,coverage)==0) {
         memset(fclass,0,9);
         strncpy(fclass,view->theme[i].fc,8);
         if (!ll_locate(fclass,fclist)) {
	    ll_insert(fclass,9*sizeof(char),ll_last(fclist));
	 }
      }
   }

   if (ll_empty(fclist)) {
       ll_reset(fclist);
       return feature_list;
   }

   rightjust(db);
   rightjust(lib);
   rightjust(coverage);
   strcpy(covpath,db);
   if (covpath[strlen(covpath)-1] != DIR_SEPARATOR) strcat(covpath,sep);
   vpfcatpath(covpath,lib);
   strcat(covpath,sep);
   vpfcatpath(covpath,coverage);
   strcat(covpath,sep);

   fit = 0;
   strcpy(path,covpath);
   vpfcatpath(path,primname[primclass]);
   vpfcatpath(path,".FIT");
   if (fileaccess(path,0)==0) {
      strcpy(path,covpath);
      vpfcatpath(path,"FCA");
      if (fileaccess(path,0)==0) fit = 1;
   }

   if (fit) {
      /* Use the Feature Index Table to find related FC rows */

      /* Find the feature class ids in the FCA */
      strcpy(path,covpath);
      vpfcatpath(path,"FCA");
      fcatable = vpf_open_table(path,ram,"rb",NULL);
      FCLASS_ = table_pos("FCLASS",fcatable);
      if (FCLASS_ < 0) {
         fprintf(stderr,"%s: Invalid FCA table - missing FCLASS field\n",
                 path);
         vpf_close_table(&fcatable);
         return feature_list;
      }

      strcpy(path,covpath);
      vpfcatpath(path,primname[primclass]);
      vpfcatpath(path,".FIT");
      fittable = vpf_open_table(path,disk,"rb",NULL);
      PRIM_ID_ = table_pos("PRIM_ID",fittable);
      TILE_ID_ = table_pos("TILE_ID",fittable);
      FC_ID_ = table_pos("FC_ID",fittable);
      if (FC_ID_ < 0) FC_ID_ = table_pos("FCA_ID",fittable);
      FEATURE_ID_ = table_pos("FEATURE_ID",fittable);
      if (PRIM_ID_ < 0 || (TILE_ID_ < 0 && tile) ||
	  FC_ID_ < 0 || FEATURE_ID_ < 0) {
	 fprintf(stderr,"%s: Invalid FIT\n",path);
	 vpf_close_table(&fcatable);
	 vpf_close_table(&fittable);
	 return feature_list;
      }

      /* Find fit rows with matching tile id */
      tileset.size = 0;
      if (tile) {
	 if (fittable.header[TILE_ID_].tdx) {
	    strcpy(path,covpath);
	    vpfcatpath(path,fittable.header[TILE_ID_].tdx);
	    if (fileaccess(path,4)==0) {
	       if (fittable.header[TILE_ID_].type == 'I') {
		  tileset = read_thematic_index( path, (char *)&tile );
	       } else if (fittable.header[TILE_ID_].type == 'S') {
		  short_tile = tile;
		  tileset = read_thematic_index( path, (char *)&short_tile );
	       }
	    }
	 }
      }
      if (!tileset.size) {
	 tileset= set_init(fittable.nrows+1);
	 set_on(tileset);
	 set_delete(0,tileset);
      }
      tilemin = set_min(tileset);
      tilemax = set_max(tileset);
   }

   /* Loop through all feature classes */
   fc = ll_first(fclist);
   while (!ll_end(fc)) {
      memset(fclass,0,9);
      ll_element(fc,fclass);
      rightjust(fclass);

      /* Determine feature class table name */
      fctablepath = (char *)NULL;
      fctablename = (char *)NULL;
      for (i=0;i<view->nthemes;i++) {
         if (stricmp(view->theme[i].database,db)==0 &&
             stricmp(view->theme[i].library,lib)==0 &&
             stricmp(view->theme[i].coverage,coverage)==0 &&
             stricmp(view->theme[i].fc,fclass)==0) {
            fctablepath = view->theme[i].ftable;
            rightjust(fctablepath);
            strcpy(path,fctablepath);
            j = strlen(fctablepath);
            while (j>=0 && fctablepath[j] != DIR_SEPARATOR) j--;
            if (fctablepath[j]==DIR_SEPARATOR)
               fctablename = &fctablepath[j+1];
            else 
               fctablename = fctablepath;
            break;
         }
      }
      if (!fctablename) {
         fc = ll_next(fc);
         continue;
      }

      featable = vpf_open_table(path,disk,"rb",NULL);

      /* Now find all spatially selected primitives for this fc that
         have been displayed. */

      if (fit) {

         /* Find the id in the FCA for this FC */
         id = 0;
         for (i=1;i<=fcatable.nrows;i++) {
            row = get_row(i,fcatable);
            buf = (char *)get_table_element(FCLASS_,row,fcatable,NULL,&n);
            memset(fcaname,0,9);
            strcpy(fcaname,buf);
            free(buf);
            free_row(row,fcatable);
            rightjust(fcaname);
            if (stricmp(fclass,fcaname)==0) {
               id = i;
               break;
            }
         }
         if (!id) {
            /* No matching fc found in FCA */
            fc == ll_next(fc);
            continue;
         }

         rowstart=tilemin;
         rowend=tilemax;
         /* See if the FC_ID column is indexed */
         if (fittable.header[FC_ID_].tdx) {
            strcpy(path,covpath);
            vpfcatpath(path,fittable.header[FC_ID_].tdx);
            if (fileaccess(path,4)==0) {
               tmpset = read_thematic_index( path, (char *)&id );
               tmpset2 = set_intersection(tmpset,tileset);
               rowstart = set_min(tmpset2);
               rowend = set_max(tmpset2);
               set_nuke(&tmpset);
               set_nuke(&tmpset2);
            }
         }

         fseek(fittable.fp,index_pos(rowstart,fittable),SEEK_SET);

         for (i=rowstart; i<=rowend; i++) {
            row = read_next_row(fittable);
	    get_table_element(PRIM_ID_,row,fittable,&primid,&n);
	    tile_id = 0;
	    if (tile) {
	       if (fittable.header[TILE_ID_].type == 'I') {
		  get_table_element(TILE_ID_,row,fittable,&tile_id,&n);
	       } else {
		  get_table_element(TILE_ID_,row,fittable,&short_tile,&n);
		  tile_id = short_tile;
	       }
	    }
	    get_table_element(FC_ID_,row,fittable,&fcid,&n);
            get_table_element(FEATURE_ID_,row,fittable,&feature_id,&n);
            free_row(row,fittable);
            if (fcid != id) continue;
            if (tile_id != tile) continue;
            if (primid != primitive_id) continue;
            if (feature_displayed(feature_id,view,db,lib,
                                  coverage,fclass,featable)) {
               memset(fstruct.fclass,0,20);
               strcpy(fstruct.fclass,fclass);
               memset(fstruct.ftable,0,255);
               strcpy(fstruct.ftable,fctablepath);
               fstruct.frow = feature_id;
               ll_insert(&fstruct,sizeof(fstruct),ll_last(feature_list));
            }
         }

      } else {

         /* No feature index table - use the fcs relates */

         /***
         Find the number of relate paths and iterate over them.
         (complex features may have more than 1 relate path between
         tables)
         ***/
         strcpy(path,covpath);
         vpfcatpath(path,"FCS");
         fcs = vpf_open_table(path,disk,"rb",NULL);
         nrelpath = num_relate_paths(primname[primclass], fctablename,
                                     fclass, fcs);
         vpf_close_table(&fcs);


         for (relpathnum=0;relpathnum<nrelpath;relpathnum++) {

            /* Set up the feature class relate from the primitive to the */
            /* feature. */
            fcrel = select_feature_class_relate( covpath, fclass,
                                                 primname[primclass],
                                                 fctablename, 
                                                 relpathnum );
            feature = fcrel.nchain-1;
            prim = 0;

            strcpy(path,covpath);
            vpfcatpath(path,tiledir);
            vpfcatpath(path,primname[primclass]);
            fcrel.table[prim] = vpf_open_table(path,disk,"rb",NULL);

            row = get_row(primitive_id,fcrel.table[prim]);

            feature_ids = fc_row_numbers(row,fcrel,tile);

            if (feature_ids) {
               if (!ll_empty(feature_ids)) {
                  p = ll_first(feature_ids);
                  while (!ll_end(p)) {
                     ll_element(p,&feature_id);
                     if (feature_id > 0 && 
                         feature_id <= fcrel.table[feature].nrows) {
                        if (feature_displayed(feature_id,view,db,lib,
                                  coverage,fclass,featable)) {
                           memset(fstruct.fclass,0,20);
                           strcpy(fstruct.fclass,fclass);
                           memset(fstruct.ftable,0,255);
                           strcpy(fstruct.ftable,fctablepath);
                           fstruct.frow = feature_id;
                           ll_insert(&fstruct,sizeof(fstruct),
                                     ll_last(feature_list));
                        }
                     }
                     p = ll_next(p);
                  }
	       }
	       ll_reset(feature_ids);
            }

            free_row(row,fcrel.table[prim]);

            deselect_feature_class_relate(&fcrel);
         }

      }

      vpf_close_table(&featable);

      fc = ll_next(fc);
   }

   ll_reset(fclist);

   if (fit) {
      vpf_close_table(&fcatable);
      vpf_close_table(&fittable);
      set_nuke(&tileset);
   }

   return feature_list;
}


/*
Get the nearest displayed node across all coverages for a library.
The node type must be specified as either ENTITY_NODE or CONNECTED_NODE.
*/
static void get_nearest_selected_node( view_type *view,
				       int db, int lib,
				       int tile, char *tiledir,
				       vpf_projection_type proj,
				       char **coverages, int ncov,
				       int node_type,
				       double xsearch, double ysearch,
                                       double search_tolerance, 
				       selection_cell_type *sel )
{
   double d, d0, xp, yp;
   int cov, i;
   char path[255], tilepath[255], *libpath, primname[4], spxname[4];
   char sep[2] = {DIR_SEPARATOR,'\0'};
   set_type primitives, selprim;
   linked_list_type fclist;
   position_type fc;
   vpf_table_type prim;
   node_rec_type node_rec;
   int tile_id, nprims;
   char tile_dir[255];
   extent_type searchbox;

   if (node_type == ENTITY_NODE) {
      strcpy(primname,"END");
      strcpy(spxname,"NSI");
   } else {
      strcpy(primname,"CND");
      strcpy(spxname,"CSI");
   }
 
   libpath = view->database[db].library[lib].path;

   for (cov = 0; cov < ncov; cov++) {
      rightjust(coverages[cov]);
      fclist = selected_coverage_feature_classes(view,db,lib,coverages[cov]);
      if (!fclist) continue;

      tile_id = tile;
      strcpy(tile_dir,tiledir);

      strcpy(tilepath,view->database[db].library[lib].path);
      vpfcatpath(tilepath,coverages[cov]);
      rightjust(tilepath);
      strcat(tilepath,sep);
      vpfcatpath(tilepath,tiledir);

      strcpy(path,tilepath);
      vpfcatpath(path,primname);

      if (fileaccess(path,0) != 0) {
	 /* Look at the coverage level (untiled coverage) */
	 strcpy(tilepath,view->database[db].library[lib].path);
	 vpfcatpath(tilepath,coverages[cov]);
	 rightjust(tilepath);
	 strcat(tilepath,sep);
	 strcpy(path,tilepath);
	 vpfcatpath(path,primname);
	 if (fileaccess(path,0)==0) {
	    tile_id = 0;
	    strcpy(tile_dir,"");
	 } else {
	    ll_reset(fclist);
	    continue;
	 }
      }

      prim = vpf_open_table(path,disk,"rb",NULL);
      nprims = prim.nrows;
      vpf_close_table(&prim);

      primitives.size = 0;
      strcpy(path,tilepath);
      vpfcatpath(path,spxname);
      if (fileaccess(path,0) == 0 && (nprims > 20)) {
         searchbox.x1 = xsearch-search_tolerance;
         searchbox.y1 = ysearch-search_tolerance;
         searchbox.x2 = xsearch+search_tolerance;
         searchbox.y2 = ysearch+search_tolerance;
	 if (proj.code > DD) {
            set_vpf_forward_projection( proj );
            set_vpf_inverse_projection( proj );
	    proj.forward_proj( &searchbox.x1, &searchbox.y1 );
	    proj.forward_proj( &searchbox.x2, &searchbox.y2 );
	 }
	 primitives = spatial_index_search(path,
                                           searchbox.x1,searchbox.y1,
                                           searchbox.x2,searchbox.y2);
	 if (set_empty(primitives)) {
            set_nuke(&primitives);
            continue;
         }
      } else {
	 primitives.size = 0;
	 primitives.buf = (char *)NULL;
      }

      if (primitives.size == 0) {
	 primitives = set_init(nprims+1L);
	 set_on(primitives);
      }

      selprim = selected_feature_class_primitives(view,db,lib,coverages[cov],
						  tile_id,tile_dir,
						  node_type,fclist,
						  primitives);

      ll_reset(fclist);

      if (sel->prow < 1) {
	 d = MAXFLOAT/2.0;
      } else {
	 d = gc_distance(xsearch,ysearch,sel->x1,sel->y2,0);
      }
      strcpy(path,tilepath);
      vpfcatpath(path,primname);
      prim = vpf_open_table(path,disk,"rb",NULL);
      for (i=1;i<=prim.nrows;i++) {
	 if (!set_member(i,selprim)) continue;
         set_vpf_inverse_projection(proj);
	 node_rec = read_node(i,prim,proj.inverse_proj);
	 d0 = gc_distance(xsearch,ysearch,node_rec.x,node_rec.y,0);
	 if (d0 < d) {
	    /* closest so far - set selection cell */
	    d = d0;
	    sel->prow = i;
	    strcpy(sel->tiledir,tile_dir);
            sel->tilenum = tile_id;
	    sprintf(sel->db,"%s%s",view->database[db].path,
		    view->database[db].name);
	    strcpy(sel->lib,view->database[db].library[lib].name);
	    strcpy(sel->cov,coverages[cov]);
	    sel->pclass = node_type;
	    sel->x1 = node_rec.x;
	    sel->y1 = node_rec.y;
	    sel->x2 = node_rec.x;
	    sel->y2 = node_rec.y;
	 }
      }
      vpf_close_table(&prim);

      set_nuke(&primitives);
      set_nuke(&selprim);
   }

}


/*
Get the nearest displayed edge across all coverages for a library.
*/
static void get_nearest_selected_edge( view_type *view,
                                       int db, int lib,
                                       int tile, char *tiledir,
				       vpf_projection_type proj,
				       char **coverages, int ncov,
				       double xsearch, double ysearch,
                                       double search_tolerance, 
				       selection_cell_type *sel )
{
   double d, d0, xp, yp;
   int cov, i;
   char path[255], tilepath[255], *libpath;
   char sep[2] = {DIR_SEPARATOR,'\0'};
   set_type primitives, selprim;
   linked_list_type fclist;
   position_type fc;
   vpf_table_type table, prim, ebr;
   extent_type extent;
   edge_rec_type edge_rec;
   int tile_id, nprims;
   char tile_dir[255];
   extent_type searchbox;

   libpath = view->database[db].library[lib].path;

   for (cov = 0; cov < ncov; cov++) {
      rightjust(coverages[cov]);
      fclist = selected_coverage_feature_classes(view,db,lib,coverages[cov]);
      if (!fclist) continue;

      tile_id = tile;
      strcpy(tile_dir,tiledir);

      strcpy(tilepath,view->database[db].library[lib].path);
      vpfcatpath(tilepath,coverages[cov]);
      rightjust(tilepath);
      strcat(tilepath,sep);
      vpfcatpath(tilepath,tiledir);

      strcpy(path,tilepath);
      vpfcatpath(path,"EDG");

      if (fileaccess(path,0) != 0) {
	 /* Look at the coverage level (untiled coverage) */
	 strcpy(tilepath,view->database[db].library[lib].path);
	 vpfcatpath(tilepath,coverages[cov]);
	 rightjust(tilepath);
	 strcat(tilepath,sep);
	 strcpy(path,tilepath);
	 vpfcatpath(path,"EDG");
	 if (fileaccess(path,0)==0) {
	    tile_id = 0;
	    strcpy(tile_dir,"");
	 } else {
	    ll_reset(fclist);
	    continue;
	 }
      }

      /* Get the total number of primitives */
      strcpy(path,tilepath);
      vpfcatpath(path,"EBR");
      if (fileaccess(path,0) != 0) {
         strcpy(path,tilepath);
         vpfcatpath(path,"EDG");
      }
      table = vpf_open_table(path,disk,"rb",NULL);
      nprims = table.nrows;
      vpf_close_table(&table);

      primitives.size = 0;
      strcpy(path,tilepath);
      vpfcatpath(path,"ESI");
      if (fileaccess(path,0) == 0 && (nprims > 20) ) {
         searchbox.x1 = xsearch-search_tolerance;
         searchbox.y1 = ysearch-search_tolerance;
         searchbox.x2 = xsearch+search_tolerance;
         searchbox.y2 = ysearch+search_tolerance;
         if (proj.code > DD) { 
            set_vpf_forward_projection( proj );
            set_vpf_inverse_projection( proj );
            proj.forward_proj( &searchbox.x1, &searchbox.y1 );
            proj.forward_proj( &searchbox.x2, &searchbox.y2 );
         }
         primitives = spatial_index_search(path,
                                           searchbox.x1,searchbox.y1,
                                           searchbox.x2,searchbox.y2);
	 if (set_empty(primitives)) {
            set_nuke(&primitives);
            continue;
         }
      } else {
	 primitives.size = 0;
	 primitives.buf = (char *)NULL;
      }

      if (primitives.size == 0) {
	 primitives = set_init(nprims+1);
	 set_on(primitives);
      }

      selprim = selected_feature_class_primitives(view,db,lib,coverages[cov],
						  tile_id,tile_dir,
						  EDGE,fclist,
						  primitives);
      ll_reset(fclist);

      if (sel->prow < 1) {
	 d = MAXFLOAT/2.0;
      } else {
	 strcpy(path,sel->db);
	 rightjust(path);
         if (path[strlen(path)-1] != DIR_SEPARATOR) strcat(path,sep);
	 vpfcatpath(path,sel->lib);
	 rightjust(path);
	 strcat(path,sep);
	 vpfcatpath(path,sel->cov);
	 rightjust(path);
	 strcat(path,sep);
	 vpfcatpath(path,sel->tiledir);
	 rightjust(path);
         if (path[strlen(path)-1] != DIR_SEPARATOR) strcat(path,sep);
         vpfcatpath(path,"EDG");
         set_vpf_inverse_projection(proj);
	 d = distance_to_edge_table(xsearch,ysearch,sel->prow,path,
				    proj.inverse_proj,0);
      }
      strcpy(path,tilepath);
      vpfcatpath(path,"EDG");
      prim = vpf_open_table(path,disk,"rb",NULL);
      strcpy(path,tilepath);
      vpfcatpath(path,"EBR");
      ebr = vpf_open_table(path,disk,"rb",NULL);
      set_vpf_inverse_projection(proj);
      for (i=1;i<=prim.nrows;i++) {
	 if (!set_member(i,selprim)) continue;
	 edge_rec = read_edge(i,prim,proj.inverse_proj);
	 d0 = distance_to_edge_rec(xsearch,ysearch,edge_rec,1,0);
         if (edge_rec.coords) free(edge_rec.coords);
	 if (d0 < d) {
	    /* closest so far - set selection cell */
	    d = d0;
	    sel->prow = i;
	    strcpy(sel->tiledir,tile_dir);
            sel->tilenum = tile_id;
	    sprintf(sel->db,"%s%s",view->database[db].path,
		    view->database[db].name);
	    strcpy(sel->lib,view->database[db].library[lib].name);
	    strcpy(sel->cov,coverages[cov]);
	    sel->pclass = EDGE;
	    extent = read_bounding_rect(i,ebr,proj.inverse_proj);
	    sel->x1 = extent.x1;
	    sel->y1 = extent.y1;
	    sel->x2 = extent.x2;
	    sel->y2 = extent.y2;
	 }
      }
      vpf_close_table(&prim);
      vpf_close_table(&ebr);

      set_nuke(&primitives);
      set_nuke(&selprim);
   }

}

/*
Get the displayed containing faces across all coverages for a library.
*/
static void get_selected_containing_faces( view_type *view,
                                           int db, int lib,
                                           int tile, char *tiledir,
				           vpf_projection_type proj,
					   char **coverages, int ncov,
					   double xsearch, double ysearch,
                                           double search_tolerance, 
                                           linked_list_type sellist )
{
   double xp, yp;
   int cov, i, contained;
   char path[255], tilepath[255], *libpath;
   char sep[2] = {DIR_SEPARATOR,'\0'};
   set_type primitives, selprim;
   linked_list_type fclist;
   position_type fc;
   vpf_table_type fac, rng, edg, fbr;
   extent_type extent;
   selection_cell_type sel;
   int tile_id, nprims;
   char tile_dir[255];
   extent_type searchbox;

   libpath = view->database[db].library[lib].path;

   set_vpf_inverse_projection( proj );
   set_vpf_forward_projection( proj );

   for (cov = 0; cov < ncov; cov++) {
      rightjust(coverages[cov]);
      fclist = selected_coverage_feature_classes(view,db,lib,coverages[cov]);
      if (!fclist) continue;

      tile_id = tile;
      strcpy(tile_dir,tiledir);

      strcpy(tilepath,view->database[db].library[lib].path);
      vpfcatpath(tilepath,coverages[cov]);
      rightjust(tilepath);
      strcat(tilepath,sep);
      vpfcatpath(tilepath,tiledir);

      strcpy(path,tilepath);
      vpfcatpath(path,"FAC");

      if (fileaccess(path,0) != 0) {
	 /* Look at the coverage level (untiled coverage) */
	 strcpy(tilepath,view->database[db].library[lib].path);
	 vpfcatpath(tilepath,coverages[cov]);
	 rightjust(tilepath);
	 strcat(tilepath,sep);
	 strcpy(path,tilepath);
	 vpfcatpath(path,"FAC");
	 if (fileaccess(path,0)==0) {
	    tile_id = 0;
	    strcpy(tile_dir,"");
	 } else {
	    ll_reset(fclist);
	    continue;
	 }
      }

      fac = vpf_open_table(path,disk,"rb",NULL);
      nprims = fac.nrows;
      vpf_close_table(&fac);

      primitives.size = 0;
      strcpy(path,tilepath);
      vpfcatpath(path,"FSI");
      if (fileaccess(path,0) == 0 && (nprims > 20) ) {
         searchbox.x1 = xsearch-search_tolerance;
         searchbox.y1 = ysearch-search_tolerance;
         searchbox.x2 = xsearch+search_tolerance;
         searchbox.y2 = ysearch+search_tolerance;
         if (proj.code > DD) { 
            set_vpf_forward_projection( proj );
            set_vpf_inverse_projection( proj );
            proj.forward_proj( &searchbox.x1, &searchbox.y1 );
            proj.forward_proj( &searchbox.x2, &searchbox.y2 );
         }
         primitives = spatial_index_search(path,
                                           searchbox.x1,searchbox.y1,
                                           searchbox.x2,searchbox.y2);
	 if (set_empty(primitives)) {
            set_nuke(&primitives);
            continue;
         }
      } else {
	 primitives.size = 0;
	 primitives.buf = (char *)NULL;
      }

      if (primitives.size == 0) {
	 primitives = set_init(fac.nrows+1);
	 set_on(primitives);
      }

      selprim = selected_feature_class_primitives(view,db,lib,coverages[cov],
						  tile_id,tile_dir,
						  FACE,fclist,
						  primitives);

      ll_reset(fclist);

      strcpy(path,tilepath);
      vpfcatpath(path,"FAC");
      fac = vpf_open_table(path,disk,"rb",NULL);
      strcpy(path,tilepath);
      vpfcatpath(path,"RNG");
      rng = vpf_open_table(path,disk,"rb",NULL);
      strcpy(path,tilepath);
      vpfcatpath(path,"EDG");
      edg = vpf_open_table(path,disk,"rb",NULL);
      strcpy(path,tilepath);
      vpfcatpath(path,"FBR");
      fbr = vpf_open_table(path,disk,"rb",NULL);
      for (i=2;i<=fac.nrows;i++) {
	 if (!set_member(i,selprim)) continue;
	 if (point_in_face(xsearch,ysearch,i,fac,rng,edg,proj.inverse_proj)) {
	    /* set selection cell */
	    sel.prow = i;
	    strcpy(sel.tiledir,tile_dir);
            sel.tilenum = tile_id;
	    sprintf(sel.db,"%s%s",view->database[db].path,
		    view->database[db].name);
	    strcpy(sel.lib,view->database[db].library[lib].name);
	    strcpy(sel.cov,coverages[cov]);
	    sel.pclass = FACE;
	    extent = read_bounding_rect(i,fbr,proj.inverse_proj);
	    sel.x1 = extent.x1;
	    sel.y1 = extent.y1;
	    sel.x2 = extent.x2;
	    sel.y2 = extent.y2;
	    ll_insert(&sel,sizeof(sel),ll_last(sellist));
	 }
      }
      vpf_close_table(&fac);
      vpf_close_table(&rng);
      vpf_close_table(&edg);
      vpf_close_table(&fbr);

      set_nuke(&primitives);
      set_nuke(&selprim);
   }

}

/*
Return the nearest of the two given selection cells.
Really only works for points.  That's how it is used in this module.
It's static.  No problem.
All coordinates assumed to be decimal degrees by the time they get here.
*/
static selection_cell_type nearest_selection_cell( double xsearch,
                                                   double ysearch,
                                                   selection_cell_type sel1,
                                                   selection_cell_type sel2 )
{
   double d1, d2;

   if (sel2.prow < 1) return sel1;
   if (sel1.prow < 1) return sel2;

   d1 = gc_distance(xsearch,ysearch,sel1.x1,sel1.y1,0);
   d2 = gc_distance(xsearch,ysearch,sel2.x1,sel2.y1,0);
   if (d1 < d2)
      return sel1;
   else
      return sel2;
}

/*
Display the selected node primitive for spatial query.
*/
static void display_selected_node( selection_cell_type sel )
{
   int x,y;
   char num[20];

   screenxy(sel.x1,sel.y1,&x,&y);
   gpsetlinecolor(circlecolor);
   gpcircle(x,y,10);
   gpsettextcolor(textcolor);
   gpsetfont("fixed");
   gpsettextheight(10);
   sprintf(num,"%d",sel.prow);
   gptext(x,y,num);
}

/*
Display the selected edge primitive for spatial query.
*/
static void display_selected_edge( selection_cell_type sel )
{
   int x,y, x1,y1,x2,y2;
   char num[20];

   gpsetlinecolor(boxcolor);
   screenxy(sel.x1,sel.y1,&x1,&y1);
   screenxy(sel.x2,sel.y2,&x2,&y2);
   gprectangle(x1,y1,x2,y2);
   screenxy( sel.x1+(sel.x2-sel.x1)/2.0,
             sel.y1+(sel.y2-sel.y1)/2.0, &x, &y );
   gpsettextcolor(textcolor);
   gpsettextjust(CENTER_TEXT,CENTER_TEXT);
   gpsetfont("fixed");
   sprintf(num,"%ld",sel.prow);
   gptext(x,y,num);
}

/*
Display the selected face primitive for spatial query.
*/
static void display_selected_face( selection_cell_type sel,
                                   map_environment_type *mapenv )
{
   int x,y, x1,y1,x2,y2;
   char num[20], path[255], sep[2]={DIR_SEPARATOR,'\0'};
   vpf_projection_type libproj;

   strcpy(path,sel.db);
   rightjust(path);
   if (path[strlen(path)-1] != DIR_SEPARATOR) strcat(path,sep);
   vpfcatpath(path,sel.lib);
   rightjust(path);
   strcat(path,sep);

   libproj = library_projection(path);

   vpfcatpath(path,sel.cov);
   rightjust(path);
   strcat(path,sep);
   vpfcatpath(path,sel.tiledir);
   rightjust(path);
   vpfcatpath(path,"FAC");

   gpsetlinecolor(boxcolor);
   screenxy(sel.x1,sel.y1,&x1,&y1);
   screenxy(sel.x2,sel.y2,&x2,&y2);
   gprectangle(x1,y1,x2,y2);
   gpsetlinecolor(outlinecolor);
   gpsetlinewidth(3);
   gpsetlinemode(GXxor);
   outline_face_table( sel.prow, path, libproj, mapenv->projection );
   gpsetlinemode(GXcopy);
   gpsetlinewidth(1);
   screenxy( sel.x1+(sel.x2-sel.x1)/2.0,
             sel.y1+(sel.y2-sel.y1)/2.0, &x, &y );
   gpsettextcolor(textcolor);
   gpsettextjust(CENTER_TEXT,CENTER_TEXT);
   gpsetfont("fixed");
   sprintf(num,"%d",sel.prow);
   gptext(x,y,num);
}

/* Try to determine if the table is a join table based upon its
   file extension.  Returns TRUE if the extension is ".*JT". */
static int is_join_table( char *tablename )
{
   char *locname, *end;
   int retval=0;

   locname = (char *)checkmalloc((strlen(tablename)+1)*sizeof(char));

   strcpy(locname,tablename);
   rightjust(locname);

   /* if '.' in string, delete through last . */

   end = strrchr(locname,'.');                   
 
   if (end != NULL) {
     strcpy(locname,end);
   }

   strupr(locname);
   locname[1] = '*';
    
   if (strcmp(locname,".*JT")==0) retval = TRUE;
 
   free(locname);
 
   return retval;
}


/* Anything in the FCS that is not primitive, join, or feature is a
   related attribute table (RAT) */
static char **get_rats_from_fcs( char *path, char *fclass,
                                 int *num_rats )
{
  char **rats = (char **)NULL;
  char loc_path[255],qstr[80];
  vpf_table_type fcs;
  row_type row;
  char query[80];
  char *rat_name;
  set_type rat_set;
  int i;
  int TABLE2_, count;
 
  *num_rats = 0;
  strcpy(loc_path,path);
  vpfcatpath(loc_path,"FCS");
  fcs = vpf_open_table(loc_path,disk,"rb",NULL);
  TABLE2_ = table_pos( "TABLE2",fcs);
 
  sprintf(query,"FEATURE_CLASS = %s",fclass);
  rat_set = query_table(query,fcs);

  for(i=0;i<set_max(rat_set);i++) {
    if (set_member(i,rat_set)) {
      row = read_row(i,fcs);
      rat_name = (char *)get_table_element( TABLE2_, row, fcs, NULL, &count );
      rightjust(rat_name);
      if ( (!is_primitive(rat_name)) &&
           (!is_join_table(rat_name)) &&
           (!is_feature(rat_name)) ) /* must be RAT */ {
         *num_rats = *num_rats + 1;
         if (!rats) {
            rats = (char **)checkmalloc(sizeof(char *));
         } else {
            rats = (char **)checkrealloc(rats, sizeof(char*)*(*num_rats));
         }
 
         rats[(*num_rats)-1] = (char *)checkmalloc(sizeof(char)*15);
         strcpy(rats[(*num_rats)-1],rat_name);
      }
      free(rat_name);
      free_row(row,fcs);
    }
  }
     
  vpf_close_table(&fcs);
  set_nuke(&rat_set);
 
  return rats;
}
 
/* Write the related attribute data to the specified file */
static void display_rat_data( char *path, char *start_table, char *fclass,
                       row_type row, FILE *fp )
{
  int i, num_rats, num_rel;
  int row_num;
  char **rats = (char **) NULL;
  char loc_path[255],rat_path[255];
  vpf_table_type fcs;
  fcrel_type fcrel;
  linked_list_type rows;
  position_type p;
 
  /* Double check file pointer */
 
  if (!fp) {
    fprintf(stderr,"display_rat_data:: passed NULL file pointer\n");
    return;
  }
 
  rats = get_rats_from_fcs( path, fclass, &num_rats );
 
  strcpy(loc_path,path);
  vpfcatpath(loc_path,"FCS");
  fcs = vpf_open_table(loc_path,disk,"rb",NULL);
 
  for (i=0;i<num_rats;i++) {
 
    strcpy(loc_path,path);

    strcpy(rat_path,path);
    vpfcatpath(rat_path,rats[i]);
    rightjust(rat_path);
 
    /* Set up feature class relate from rat to start_table */
 
    fcrel = select_feature_class_relate( loc_path, fclass, start_table,
                                         rats[i], 0);
 
    /* tile is 0 because relating to a RAT, not primitive table */
    rows = fc_row_numbers( row, fcrel, 0 );
    if (!ll_empty(rows))
       fprintf(fp,"%s: %s\n",
               strupr(fcrel.table[fcrel.nchain-1].name),
               fcrel.table[fcrel.nchain-1].description);
    /* Loop through list of rows */
    p = ll_first(rows);
    while (!ll_end(p)) {
      ll_element(p,&row_num);
      vpf_display_table_record( rat_path, row_num, fp );
      p = ll_next(p);
    } /* while !ll_end */

    ll_reset(rows);       
     
    /* Take down feature class relate */
    deselect_feature_class_relate( &fcrel );
 
  } /* for i */

  vpf_close_table(&fcs);
 
  if (rats) {
    for (i=num_rats-1; i>=0; i--)
      free(rats[i]);
    free(rats);
  }
}




/*
Write the attributes for the selected primitive to the given file.
Need to display attributes from all applicable selected feature classes
of the primitive.
*/
static void display_selected_attributes( selection_cell_type sel,
                                         view_type *view,
                                         FILE *fp )
{
   linked_list_type feature_list;
   position_type p;
   feature_struct_type fstruct;
   char *descr, *ftableptr, ftable[80];
   int tilenum;
   char covpath[255], start_table[255];
   vpf_table_type table;
   row_type row;

   fprintf(fp,"Database: %s\n",sel.db);
   fprintf(fp,"Library: %s\n",sel.lib);
   fprintf(fp,"Coverage: %s\n",sel.cov);

   tilenum = sel.tilenum;
   if (stricmp(sel.tiledir,"")==0) tilenum = 0;

   feature_list = primitive_related_features(view,sel.db,sel.lib,sel.cov,
                                             tilenum,sel.tiledir,
                                             sel.pclass,sel.prow);
   if (!feature_list) {
      return;
   }
   if (ll_empty(feature_list)) {
      ll_reset(feature_list);
      return;
   }

   /* set up path to coverage */

   strcpy(covpath,sel.db);
   vpfcatpath(covpath,"/");
   vpfcatpath(covpath,sel.lib);
   vpfcatpath(covpath,"/");
   vpfcatpath(covpath,sel.cov);
   vpfcatpath(covpath,"/");

   p = ll_first(feature_list);
   while (!ll_end(p)) {
      ll_element(p,&fstruct);

      ftableptr = strrchr(fstruct.ftable,'/');
      if (ftableptr)
         ftableptr++;
      else
         ftableptr = fstruct.ftable;
      strcpy(ftable,ftableptr);
      strupr(ftable);

      descr = feature_class_table_description(fstruct.ftable);
      fprintf(fp,"%s: %s - \n",ftable,descr);
      free(descr);

      vpf_display_table_record(fstruct.ftable,fstruct.frow,fp);

      /* Display any related attribute data... */
      table = vpf_open_table(fstruct.ftable,disk,"rb",NULL);
      if (!table.fp) {
         p = ll_next(p);
         continue;
      }
      row = read_row(fstruct.frow,table);

      display_rat_data(covpath,ftable,fstruct.fclass,row,fp);
      free_row(row,table);
      vpf_close_table(&table);

      fprintf(fp,"\n");

      p = ll_next(p);
   }

   ll_reset(feature_list);
}

static void nullify_selection_cell( selection_cell_type *sel )
{
   sel->prow = -1;
   strcpy(sel->db,"");
   strcpy(sel->lib,"");
   strcpy(sel->cov,"");
   strcpy(sel->tiledir,"");
   sel->tilenum = 0;
   sel->pclass = 0;
   sel->x1 = 0.0;
   sel->y1 = 0.0;
   sel->x2 = 0.0;
   sel->y2 = 0.0;
}

/*************************************************************************
 *
 *N  spatial_query
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *     This function displays feature attribute information about 
 *     displayed features spatially selected from the map.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    view     <input>==(view_type *) view structure.
 *    mapenv   <input>==(map_environment_type *) map environment structure.
 *    xsearch  <input>==(double) search coordinate X value.
 *    ysearch  <input>==(double) search coordinate Y value.
 *    points   <input>==(int) flag to search for point features.
 *    lines    <input>==(int) flag to search for line features.
 *    areas    <input>==(int) flag to search for area features.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels    May 1991                      DOS Turbo C
 *                      Nov 1992    UNIX mdb - major rewrite.
 *E
 *************************************************************************/
void spatial_query( view_type *view,
                    map_environment_type *mapenv,
                    double xsearch, double ysearch,
                    int points, int lines, int areas )
{
   int db, lib, tile, i, ncov, x,y;
   char tiledir[255], msg[80], tmpfile[128], **coverages;
   selection_cell_type endsel, cndsel, pointsel, edgsel, facsel;
   linked_list_type facsel_list;
   position_type p;
   FILE *fp;
   vpf_projection_type proj;
   double search_tolerance;

   if ((!points) && (!lines) && (!areas)) return;

   if ( (!mapenv->mapdisplayed) || (!mapenv->study_area_selected) ) {
      return;
   }
 
   set_coverage_window();

   set_vpf_forward_projection( mapenv->projection );
   set_vpf_inverse_projection( mapenv->projection );
   mapinit( mapenv->mapextent.x1,mapenv->mapextent.y1,
            mapenv->mapextent.x2,mapenv->mapextent.y2,
            gpgetmaxx(),gpgetmaxy(),
            mapenv->projection.forward_proj,
            mapenv->projection.inverse_proj );
   screenxy( xsearch, ysearch, &x, &y );
   gpsetlinecolor( markcolor );
   gpcircle( x, y, 4 );
   gpflushdevice(); 

   search_tolerance = (mapenv->mapextent.x2-mapenv->mapextent.x1)/10.0;

   nullify_selection_cell(&endsel);
   nullify_selection_cell(&cndsel);
   nullify_selection_cell(&pointsel);
   nullify_selection_cell(&edgsel);
   facsel_list = ll_init();

   for (db=0; db<view->ndb; db++) {
      for (lib=0; lib<view->database[db].nlibraries; lib++) {
         if (!view->database[db].library[lib].viewable) continue;

	 if (!selected_themes_in_library(view,db,lib)) continue;

	 proj = library_projection(view->database[db].library[lib].path);

	 get_search_tile( view,db,lib, xsearch,ysearch, &tile, tiledir,
			  proj );
         set_vpf_forward_projection( mapenv->projection );

         if (tile < 0) continue;

	 coverages = library_coverage_names(
		     view->database[db].library[lib].path, &ncov );

         if (points) {
	    get_nearest_selected_node( view, db, lib,
				       tile,tiledir,
				       proj,
				       coverages, ncov,
				       ENTITY_NODE,
				       xsearch, ysearch,
                                       search_tolerance,
				       &endsel );
	    get_nearest_selected_node( view, db, lib,
				       tile,tiledir,
				       proj,
				       coverages, ncov,
				       CONNECTED_NODE,
				       xsearch, ysearch,
                                       search_tolerance,
				       &cndsel );
         }

         if (lines) {
            get_nearest_selected_edge( view, db, lib,
				       tile,tiledir,
				       proj,
				       coverages, ncov,
				       xsearch, ysearch,
                                       search_tolerance,
				       &edgsel );
         }

         if (areas) {
            get_selected_containing_faces( view, db, lib,
                                           tile,tiledir,
				           proj,
					   coverages, ncov,
					   xsearch, ysearch,
                                           search_tolerance,
                                           facsel_list );
	 }
	 for (i=0;i<ncov;i++) free(coverages[i]);
	 if (coverages) free(coverages);
      }
   }

   tmpnam(tmpfile);
   fp = fopen(tmpfile,"wt");

   set_vpf_forward_projection( mapenv->projection );
   set_vpf_inverse_projection( mapenv->projection );
   mapinit( mapenv->mapextent.x1,mapenv->mapextent.y1,
            mapenv->mapextent.x2,mapenv->mapextent.y2,
            gpgetmaxx(),gpgetmaxy(),
            mapenv->projection.forward_proj,
            mapenv->projection.inverse_proj );

   if (points && (endsel.prow > 0 || cndsel.prow > 0)) {
      pointsel = nearest_selection_cell( xsearch, ysearch, endsel, cndsel );
      display_selected_node(pointsel);
      if (pointsel.pclass == ENTITY_NODE)
         fprintf(fp,"ENTITY NODE %d:\n",pointsel.prow);
      else
         fprintf(fp,"CONNECTED NODE %d:\n",pointsel.prow);
      display_selected_attributes(pointsel,view,fp); 
   }

   if (lines && edgsel.prow > 0) {
      display_selected_edge(edgsel); 
      fprintf(fp,"EDGE %d:\n",edgsel.prow);
      display_selected_attributes(edgsel,view,fp); 
   }

   if (areas && (!ll_empty(facsel_list))) {
      p = ll_first(facsel_list);
      while (!ll_end(p)) {
         ll_element(p,&facsel);
         display_selected_face(facsel,mapenv); 
         fprintf(fp,"FACE %d:\n",facsel.prow);
         display_selected_attributes(facsel,view,fp); 
         p = ll_next(p);
      }
   }

   if (pointsel.prow < 0 && edgsel.prow < 0 && ll_empty(facsel_list))
      fprintf(fp,"No features found at specified location\n");

   ll_reset(facsel_list);

   fclose(fp);
   spqry_popup_show(tmpfile);
   remove(tmpfile);

   set_vpf_forward_projection( mapenv->projection );
   set_vpf_inverse_projection( mapenv->projection );
}



