/***************************************************************************
 *
 *N  Module VPFRELAT.C
 *
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    This module contains functions supporting relates between VPF
 *    feature classes and primitives (and vice versa).  It relies
 *    upon the information provided by the Feature Class Schema table.
 *    This table is used to generate a feature class relationship (fcrel)
 *    data structure for a feature class.  This structure contains all
 *    of the tables and their primary and foreign keys for the
 *    relationships between a feature table and its primitive, or
 *    from a primitive to its feature table (each relate chain is one way).
 *    This module tries to be as much of a black box as it can to
 *    enable a programmer to simply return the corresponding primitive
 *    row of a feature record, or the corresponding feature row of a
 *    primitive record.
 *
 *    This is one of the most difficult modules required to support
 *    a truly 'generic' VPF application, since VPF allows so many
 *    variations of feature-primitive relationships.  The final version
 *    of this module must support every allowed relationship.
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels           Nov 1991           DOS Turbo C
 *
 *    Added one-to-many relates 3/2/92 - BJM
 *    UNIX mdb port - Dec 1992 - BJM
 *
 *    Added (char *) to strdup calls, lines 504, 703 - 1/25/95 - SGT
 *E
 *::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   External Variables:
 *X
 *    None
 *E
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#ifdef __MSDOS__
#include <alloc.h>
#include <mem.h>
#include <stdarg.h>
#endif 
#if ( defined(CYGWIN) )
#include </usr/include/mingw/values.h>
#else
#include <values.h>
#endif

#include <string.h>
#include "set.h"
#include "vpftable.h"
#include "vpfview.h"
#include "vpfrelat.h"
#include "vpftidx.h"
#include "vpfquery.h"
#include "vvmisc.h"
#include "strfunc.h"


/* Determine if the given table name is in the given list of */
/* vpf relate structures.				     */
static int table_in_list( char *tablename, linked_list_type rlist )
{
   position_type p;
   vpf_relate_struct rcell;

   p = ll_first(rlist);
   while (!ll_end(p)) {
      ll_element(p,&rcell);
      if (stricmp(rcell.table1,tablename)==0) return 1;
      p = ll_next(p);
   }
   return 0;
}

/* 
   Determine if the relate path from the given start table to the end table
   starting at the specified row in the FCS is actually valid.  There is a
   chance that it may start OK, but lead to nowhere.  The "is_first_pass"
   flag should always be passed in as 1 for any function calling this
   other than itself (this is a recursive function - all recursive calls
   set the flag to 0 to indicate that it is in a recursive loop. 

Oct 93--Scott J. Simon
   Added prev_table argument, so that all records in FCS can be searched
   each time. The first time this is called, the prev_table argument should
   be the same as the start_table.
*/
static int is_valid_path( char *fclass, 
                          char *start_table, 
                          char *end_table, 
		          char *prev_table, 
                          int row_num,
		          vpf_table_type fcs, 
                          int is_first_pass )
{
  set_type fc_set;
  char query[80], *second_table;
  static char orig_table[255];
  row_type row;
  int i,TABLE2_,count;

  if (stricmp(start_table,end_table)==0) return 1;

  sprintf(query,"FEATURE_CLASS = %s AND TABLE1 = %s AND TABLE2 <> %s",
          fclass,start_table,prev_table);
  
  fc_set = query_table(query,fcs);
  if (set_empty(fc_set)) {
    set_nuke(&fc_set);
    return 0;
  }

  TABLE2_ = table_pos("TABLE2",fcs);

  if(is_first_pass) {
    strcpy(orig_table,start_table);
    row = get_row(row_num,fcs);
    second_table = (char *) get_table_element(TABLE2_,row,fcs,
					      (void *)NULL,&count);
    rightjust(second_table);
    set_nuke(&fc_set);
    sprintf(query,"FEATURE_CLASS = %s AND TABLE1 = %s AND TABLE2 <> %s",
            fclass,second_table,start_table);
    fc_set = query_table(query,fcs);
    row_num = set_min(fc_set);
    if (is_valid_path(fclass,second_table,end_table,prev_table,row_num,fcs,0)) {
      set_nuke(&fc_set);
      free_row(row,fcs);
      free(second_table);
      return 1;
    }
    free_row(row,fcs);
    free(second_table);
  } else {
    if(stricmp(orig_table,start_table)==0) {
       set_nuke(&fc_set);
       return 0;
    }
    for (i=1;i<=set_max(fc_set);i++) {
      if (set_member(i,fc_set)) {
	row = get_row(i,fcs);
	second_table = (char *) get_table_element(TABLE2_,row,fcs,
						  (void *)NULL,&count);
	rightjust(second_table);
	if (is_valid_path(fclass,second_table,end_table,start_table,i,fcs,0)) {
	  set_nuke(&fc_set);
	  free_row(row,fcs);
	  free(second_table);
	  return 1;
	}
	free_row(row,fcs);
	free(second_table);
      }
    }
  }
  set_nuke(&fc_set);
  return 0;
}

/**************************************************************************
 *
 *N  num_relate_paths
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Find the number of valid possible relate paths for the feature class
 *    from the start table to the end table.  (Complex features can 
 *    have several relates to a single primitive table.)
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    start_table  <input> == (char *) table to start from.
 *    end_table    <input> == (char *) table to start from.
 *    fcname       <input> == (char *) feature class name.
 *    fcs          <input> == (vpf_table_type) feature class schema table.
 *    num_relate_paths<output> == (int) number of relate paths found.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels  DOS Turbo C
 *                    UNIX mdb port    Dec 1992
 *E
 *************************************************************************/
int num_relate_paths( char *start_table,
                      char *end_table,
                      char *fcname, 
                      vpf_table_type fcs )
{
   set_type fcset;
   int i,n;
   char qstr[80];

   sprintf(qstr,"FEATURE_CLASS = %s AND TABLE1 = %s",
           fcname,start_table);
   fcset = query_table(qstr,fcs);
   n = 0;
   for (i=1;i<=fcs.nrows;i++) {
      if (!set_member(i,fcset)) continue;
      if (is_valid_path(fcname,start_table,end_table,start_table,i,fcs,1)) {
         n++;
      }
   }
   set_nuke(&fcset);

   return n;
}

/**************************************************************************
 *
 *N  fcs_relate_list
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Read the feature class schema table and create the list of
 *    tables to chain through.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    fcname       <input> == (char *) feature class name.
 *    start_table  <input> == (char *) table to start from.
 *    end_table    <input> == (char *) table to end with.
 *    fcs          <input> == (vpf_table_type) feature class schema table.
 *    npath        <input> == (int) relate path number.
 *    fcs_relate_list <output> == (linked_list_type) list of tables to
 *                                chain through.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels  DOS Turbo C
 *E
 *************************************************************************/
linked_list_type fcs_relate_list( char *fcname, char *start_table,
				  char *end_table, vpf_table_type fcs,
                                  int npath )
{
   linked_list_type rlist;
   vpf_relate_struct rstruct;
   set_type fcset;
   char tablename[255], *buf, expr[255];
   row_type row;
   int i,rownum,n;
   int TABLE1_, KEY1_, TABLE2_, KEY2_;
   char prevstr[80];

   rlist = ll_init();

   sprintf(expr,"FEATURE_CLASS = %s AND TABLE1 = %s",fcname,start_table);

   fcset = query_table(expr,fcs);

   if (set_empty(fcset)) {
      set_nuke(&fcset);
      return rlist;
   }

   TABLE1_ = table_pos("TABLE1",fcs);
   KEY1_ = table_pos("FOREIGN_KEY",fcs);
   if (KEY1_ < 0) {
      KEY1_ = table_pos("TABLE1_KEY",fcs);
   }
   TABLE2_ = table_pos("TABLE2",fcs);
   KEY2_ = table_pos("PRIMARY_KEY",fcs);
   if (KEY2_ < 0) {
      KEY2_ = table_pos("TABLE2_KEY",fcs);
   }

   /* Get to the relate path number */
   n = -1;
   rownum = 0;
   for (i=1;i<fcs.nrows;i++) {
      if (set_member(i,fcset)) {
         if (!is_valid_path(fcname,start_table,end_table,start_table,i,fcs,1))
           continue;
         rownum = i;
         n++;
	 if (n>=npath) break;
      }
   }
   if (n<npath) rownum = set_max(fcset);

   set_nuke(&fcset);

   row = get_row(rownum,fcs);

   buf = (char *)get_table_element(TABLE1_,row,fcs,NULL,&n);
   strcpy(rstruct.table1,buf);
   rightjust(rstruct.table1);
   free(buf);

   buf = (char *)get_table_element(KEY1_,row,fcs,NULL,&n);
   strcpy(rstruct.key1,buf);
   rightjust(rstruct.key1);
   free(buf);

   buf = (char *)get_table_element(TABLE2_,row,fcs,NULL,&n);
   strcpy(rstruct.table2,buf);
   rightjust(rstruct.table2);
   free(buf);

   buf = (char *)get_table_element(KEY2_,row,fcs,NULL,&n);
   strcpy(rstruct.key2,buf);
   rightjust(rstruct.key2);
   free(buf);

   free_row( row, fcs );

   ll_insert( &rstruct, sizeof(rstruct), ll_last(rlist) );

   strcpy( tablename, rstruct.table2 );
   strcpy( prevstr, rstruct.table1 );


   while ( stricmp(tablename,end_table) != 0) {

      sprintf(expr,"FEATURE_CLASS = %s AND TABLE1 = %s AND TABLE2 <> %s",
              fcname,tablename,prevstr);

      fcset = query_table(expr,fcs);
      if (set_empty(fcset)) {
	 set_nuke(&fcset);
	 return rlist;
      }
      rownum = set_min(fcset);

      set_nuke(&fcset);

      row = get_row(rownum,fcs);

      buf = (char *)get_table_element(TABLE1_,row,fcs,NULL,&n);
      strcpy(rstruct.table1,buf);
      rightjust(rstruct.table1);
      free(buf);

      buf = (char *)get_table_element(KEY1_,row,fcs,NULL,&n);
      strcpy(rstruct.key1,buf);
      rightjust(rstruct.key1);
      free(buf);

      buf = (char *)get_table_element(TABLE2_,row,fcs,NULL,&n);
      strcpy(rstruct.table2,buf);
      rightjust(rstruct.table2);
      free(buf);

      buf = (char *)get_table_element(KEY2_,row,fcs,NULL,&n);
      strcpy(rstruct.key2,buf);
      rightjust(rstruct.key2);
      free(buf);

      free_row( row, fcs );

      if (table_in_list(rstruct.table1, rlist)) break;

      ll_insert( &rstruct, sizeof(rstruct), ll_last(rlist) );

      strcpy( tablename, rstruct.table2 );
      strcpy( prevstr, rstruct.table1 );
   }

   return rlist;
}




/**************************************************************************
 *
 *N  related_row
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Return the related row of table2 based upon the value of table 1's key
 *    Table 2 must be the '1' side of an n:1 relationship  --  If it isn't,
 *    use 'related_rows()'.
 *    Supported data types - I, S, K, and T<n>.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    keyval1  <input> == (void *) key value from table 1 in the relate.
 *    table2   <input> == (vpf_table_type) table 2 in the relate.
 *    key2     <input> == (char *) key column name in table 2.
 *    return  <output> == (int) first related row number in table 2.
 *                        0 is returned if no related rows are found.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels  DOS Turbo C
 *                    UNIX mdb port - use Thematic Index   Dec 1992
 *                    Added type K - 1/93
 *E
 *************************************************************************/
int related_row( void *keyval1,
		      vpf_table_type table2, char *key2,
		      int tile_id )
{
   int rowid, i, ival, n, tile, start,end;
   short int sval;
   row_type row;
   int KEY2_,TILE_;
   char cval, *tval, path[255], *keystring;
   set_type idxset, tileset, searchset;
   id_triplet_type idtrip;

   if (stricmp(key2,"ID")==0) {
      memcpy( &rowid, keyval1, sizeof(rowid) );
      return rowid;
   }

   rowid = 0;

   KEY2_ = table_pos(key2,table2);

   if ((table2.header[KEY2_].type != 'I')&&
       (table2.header[KEY2_].type != 'S')&& 
       (table2.header[KEY2_].type != 'K')&& 
       (table2.header[KEY2_].type != 'T')) return rowid;

   if ((table2.header[KEY2_].type != 'T')&&
       (table2.header[KEY2_].count != 1)) return rowid;

   if (tile_id > 0)
      TILE_ = table_pos("TILE_ID",table2);
   else
      TILE_ = -1;

   idxset.size = 0;
   if (table2.header[KEY2_].tdx) {
      strcpy(path,table2.path);
      rightjust(path);
      if (path[strlen(path)-1] != DIR_SEPARATOR) vpfcatpath(path,"\\");
      vpfcatpath(path,table2.header[KEY2_].tdx);
      if (fileaccess(path,4)==0) {
         if (table2.header[KEY2_].type == 'I') {
            memcpy(&ival,keyval1,sizeof(int));
            idxset = read_thematic_index(path,(char *)&ival);
         } else if (table2.header[KEY2_].type == 'S') {
            memcpy(&sval,keyval1,sizeof(short int));
            idxset = read_thematic_index(path,(char *)&sval);
         } else {
            idxset = read_thematic_index(path,(char *)keyval1);
         }
	 if (TILE_ < 0) {
	    /* Don't bother checking TILE_ID. */
	    /* Take the first value in the set. */
	    i = set_min(idxset);
	    if (i>table2.nrows || i<0) i=0;
	    set_nuke(&idxset);
	    return i;
	 }
      }
   }
   if (idxset.size == 0) {
      idxset = set_init(table2.nrows);
      set_on(idxset);
   }

   tileset.size = 0;
   if (TILE_ >= 0) {
      if (table2.header[TILE_].tdx) {
	 strcpy(path,table2.path);
         rightjust(path);
         if (path[strlen(path)-1] != DIR_SEPARATOR) vpfcatpath(path,"\\");
	 vpfcatpath(path,table2.header[TILE_].tdx);
	 if (fileaccess(path,4)==0) {
	    tile = tile_id;
	    if (table2.header[TILE_].type == 'S') {
	       sval = (short int)tile;
	       tileset = read_thematic_index(path,(char *)&sval);
	    } else if (table2.header[TILE_].type == 'I') {
	       tileset = read_thematic_index(path,(char *)&tile);
	    }
	 }
      }
   }
   if (tileset.size == 0) {
      tileset = set_init(table2.nrows);
      set_on(tileset);
   }

   searchset = set_intersection(tileset,idxset);
   set_nuke(&tileset);
   set_nuke(&idxset);

   if (table2.header[KEY2_].type == 'T') {
      keystring = (char *)strdup((char *)keyval1);
      rightjust(keystring);
   } else {
      keystring = NULL;
   }

   start = set_min(searchset);
   end = set_max(searchset);
   if (start < 1) start = 1;
   if (end > table2.nrows) end = table2.nrows;

   for (i=start;i<=end;i++) {
      if (!set_member(i,searchset)) continue;
      row = get_row(i,table2);

      if (TILE_>0) {
	 tile = tile_id;
	 if (table2.header[TILE_].type == 'S') {
	    get_table_element(TILE_,row,table2,&sval,&n);
	    tile = sval;
	 } else if (table2.header[TILE_].type == 'I') {
	    get_table_element(TILE_,row,table2,&tile,&n);
	 }
	 if (tile != tile_id) {
	    free_row(row,table2);
	    continue;
	 }
      }

      if (table2.header[KEY2_].type == 'I') {
	 get_table_element(KEY2_,row,table2,&ival,&n);
	 if (memcmp(&ival,keyval1,sizeof(ival))==0) rowid = i;
      } else if (table2.header[KEY2_].type == 'S') {
	 get_table_element(KEY2_,row,table2,&sval,&n);
	 ival = (int)sval;
	 if (memcmp(&ival,keyval1,sizeof(ival))==0) rowid = i;
      } else if (table2.header[KEY2_].type == 'K') {
	 get_table_element(KEY2_,row,table2,&idtrip,&n);
         if (idtrip.tile != tile_id) {
            free_row(row,table2);
            continue;
         }
	 ival = (int)idtrip.exid;
	 if (memcmp(&ival,keyval1,sizeof(ival))==0) rowid = i;
      } else if (table2.header[KEY2_].type == 'T') {
	 if (table2.header[KEY2_].count==1) {
	    get_table_element(KEY2_,row,table2,&cval,&n);
	    if (memcmp(&cval,keyval1,sizeof(ival))==0) rowid = i;
	 } else {
	    tval = (char *)get_table_element(KEY2_,row,table2,NULL,&n);
            rightjust(tval);
	    if (stricmp(tval,keystring)==0) rowid = i;
	 }
      }
      free_row(row,table2);
      if (rowid > 0) break;

   }

   set_nuke(&searchset);

   if (keystring) free(keystring);

   return rowid;
}


/**************************************************************************
 *
 *N  related_rows
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Return the list of related rows of table2 based upon the value of
 *    table 1's key.
 *    Supported data types - I, S, K, and T<n>.
 *    Thematic index used, if present on key column.
 *    NOTE: A sequential search operation will search the entire
 *          table if no thematic index ...zzz...
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Parameters:
 *A
 *    keyval1  <input> == (void *) key value from table 1 in the relate.
 *    table2   <input> == (vpf_table_type) table 2 in the relate.
 *    key2     <input> == (char *) key column name in table 2.
 *    return  <output> == (linked_list_type) list of (int) related rows
 *                        in table 2.  If no related rows are found, the
 *                        returned list is empty.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels  DOS Turbo C
 *                    UNIX mdb port - use Thematic Index - Dec 1992
 *                    Added support for type 'K' - 1/93
 *E
 *************************************************************************/
linked_list_type related_rows( void *keyval1,
		       vpf_table_type table2, char *key2,
                       int tile_id )
{
   linked_list_type rowlist;
   set_type idxset, tileset, searchset;
   int rowid, i, ival, n, start,end, tile;
   short int short_tile, sval;
   row_type row;
   int KEY2_, TILE_;
   char cval, *tval, path[255], *keystring;
   id_triplet_type idtrip;

   rowlist = ll_init();

   if (stricmp(key2,"ID")==0) {
      memcpy( &rowid, keyval1, sizeof(rowid) );
      ll_insert(&rowid,sizeof(rowid),rowlist);
      return rowlist;
   }

   KEY2_ = table_pos(key2,table2);

   if ((table2.header[KEY2_].type != 'I')&&
       (table2.header[KEY2_].type != 'S')&&
       (table2.header[KEY2_].type != 'K')&&
       (table2.header[KEY2_].type != 'T')) return rowlist;

   if ((table2.header[KEY2_].type == 'I')&&
       (table2.header[KEY2_].count != 1)) return rowlist;

   if (tile_id > 0)
      TILE_ = table_pos("TILE_ID",table2);
   else
      TILE_ = -1;

   idxset.size = 0;
   if (table2.header[KEY2_].tdx) {
      strcpy(path,table2.path);
      rightjust(path);
      if (path[strlen(path)-1] != DIR_SEPARATOR) vpfcatpath(path,"\\");
      vpfcatpath(path,table2.header[KEY2_].tdx);
      if (fileaccess(path,4)==0) {
         if (table2.header[KEY2_].type == 'I') {
            memcpy(&ival,keyval1,sizeof(int));
            idxset = read_thematic_index(path,(char *)&ival);
         } else if (table2.header[KEY2_].type == 'S') {
            memcpy(&sval,keyval1,sizeof(short int));
            idxset = read_thematic_index(path,(char *)&sval);
         } else {
            idxset = read_thematic_index(path,(char *)keyval1);
         }
	 if (TILE_ < 0) {
	    /* don't bother to check TILE_ID for a match */
	    start = set_min(idxset);
	    end = set_max(idxset);
	    for (i=start;i<=end;i++)
	       if (set_member(i,idxset))
		  ll_insert(&i,sizeof(i),ll_last(rowlist));
	    set_nuke(&idxset);
	    return rowlist;
	 }
      }
   }
   if (idxset.size == 0) {
      idxset = set_init(table2.nrows);
      set_on(idxset);
   }

   tileset.size = 0;
   if (TILE_ >= 0) {
      if (table2.header[TILE_].tdx) {
	 strcpy(path,table2.path);
         rightjust(path);
         if (path[strlen(path)-1] != DIR_SEPARATOR) vpfcatpath(path,"\\");
	 vpfcatpath(path,table2.header[TILE_].tdx);
	 if (fileaccess(path,4)==0) {
	    tile = tile_id;
	    if (table2.header[TILE_].type == 'S') {
	       sval = (short int)tile;
	       tileset = read_thematic_index(path,(char *)&sval);
	    } else if (table2.header[TILE_].type == 'I') {
	       tileset = read_thematic_index(path,(char *)&tile);
	    }
	 }
      }
   }
   if (tileset.size == 0) {
      tileset = set_init(table2.nrows);
      set_on(tileset);
   }

   searchset = set_intersection(tileset,idxset);
   set_nuke(&tileset);
   set_nuke(&idxset);

   if (table2.header[KEY2_].type == 'T') {
      keystring = (char *)strdup((char *)keyval1);
      rightjust(keystring);
   } else {
      keystring = NULL;
   }

   start = set_min(searchset);
   end = set_max(searchset);
   if (start < 1) start = 1;
   if (end > table2.nrows) end = table2.nrows;

   for (i=start;i<=end;i++) {
      if (set_member(i,searchset)) {

	 row = get_row(i,table2);

         if (TILE_>0) {
            tile = tile_id;
            if (table2.header[TILE_].type == 'S') {
               get_table_element(TILE_,row,table2,&short_tile,&n);
               tile = short_tile;
            } else if (table2.header[TILE_].type == 'I') {
               get_table_element(TILE_,row,table2,&tile,&n);
            }
            if (tile != tile_id) {
               free_row(row,table2);
               continue;
            }
         }

	 if (table2.header[KEY2_].type == 'I') {
	    get_table_element(KEY2_,row,table2,&ival,&n);
	    if (memcmp(&ival,keyval1,sizeof(ival))==0)
	       ll_insert(&i,sizeof(i),ll_last(rowlist));
	 } else if (table2.header[KEY2_].type == 'S') {
	    get_table_element(KEY2_,row,table2,&sval,&n);
	    if (memcmp(&sval,keyval1,sizeof(sval))==0)
	       ll_insert(&i,sizeof(i),ll_last(rowlist));
	 } else if (table2.header[KEY2_].type == 'K') {
	    get_table_element(KEY2_,row,table2,&idtrip,&n);
            if (idtrip.tile != tile_id) {
               free_row(row,table2);
               continue;
            }
            ival = idtrip.exid;
	    if (memcmp(&ival,keyval1,sizeof(ival))==0)
	       ll_insert(&i,sizeof(i),ll_last(rowlist));
	 } else if (table2.header[KEY2_].type == 'T') {
	    if (table2.header[KEY2_].count==1) {
	       get_table_element(KEY2_,row,table2,&cval,&n);
	       if (memcmp(&cval,keyval1,sizeof(ival))==0)
		  ll_insert(&i,sizeof(i),ll_last(rowlist));
	    } else {
	       tval = (char *)get_table_element(KEY2_,row,table2,NULL,&n);
               rightjust(tval);
	       if (stricmp(tval,keystring)==0)
		  ll_insert(&i,sizeof(i),ll_last(rowlist));
	    }
	 }
	 free_row(row,table2);
      }

   }

   set_nuke(&searchset);

   return rowlist;
}


/**************************************************************************
 *
 *N  select_feature_class_relate
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Set up the relationships between features and primitives or between
 *    primitives and features (one way only) for a specified feature class.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels  DOS Turbo C
 *                    UNIX mdb port - Dec 1992
 *E
 *************************************************************************/
fcrel_type select_feature_class_relate( char *covpath,
                                        char *fcname,
					char *start_table,
					char *end_table,
                                        int npath )
{
   int storage;
   vpf_table_type fcs;
   int i;
   char path[255];
   position_type p;
   vpf_relate_struct rcell;
   fcrel_type fcrel;

   fcrel.nchain = 0;
   fcrel.table = NULL;
   fcrel.relate_list = NULL;

   rightjust(covpath);
   sprintf( path, "%sfcs", covpath );

   /* Feature Class Schema table */
   fcs = vpf_open_table( path, ram, "rb", NULL );
   if (!fcs.fp) {
      fprintf(stderr,"select_feature_class_relate: Error opening %s\n",path);
      return fcrel;
   }

   fcrel.relate_list = fcs_relate_list( fcname,
					start_table,end_table,
					fcs, npath );

   if (ll_empty(fcrel.relate_list)) {
      ll_reset(fcrel.relate_list);
#ifdef __MSDOS__
      displaymessage("ERROR in feature class relationship!",
		     start_table,end_table,NULL);
#else
      fprintf(stderr, "ERROR in feature class relationship!");
#endif
      return fcrel;
   }

   /* Find the number of tables in the relate chain */
   p = ll_first(fcrel.relate_list);
   fcrel.nchain = 0;
   while (!ll_end(p)) {
      fcrel.nchain++;
      p = ll_next(p);
   }
   /* Allow for last table2 */
   fcrel.nchain++;

   fcrel.table = (vpf_table_type *)
		  malloc((fcrel.nchain+1)*
			     sizeof(vpf_table_type));
   if (!fcrel.table) {
      fprintf(stderr,"Out of memory in select_feature_class_relate\n");
      exit(1);
   }

   for (i=0;i<fcrel.nchain+1;i++) {
      vpf_nullify_table( &(fcrel.table[i]) );
   }


   p = ll_first(fcrel.relate_list);
   for (i=0;i<fcrel.nchain-1;i++) {

      ll_element(p,&rcell);

      /** Can't open primitive table - may be several under tile **/
      /** directories.  Open all others **/
      if (!is_primitive(rcell.table1)) {

	 strcpy(path,covpath);
	 vpfcatpath(path,rcell.table1);
	 storage = disk;

	 fcrel.table[i] = vpf_open_table(path,(storage_type) storage,"rb",NULL);
 
      }

      if (!ll_end(p)) p = ll_next(p);
   }

   /* End of relate chain */
   i = fcrel.nchain-1;
   if (!is_primitive(rcell.table2)) {

      strcpy(path,covpath);
      vpfcatpath(path,rcell.table2);
      storage = disk;

      fcrel.table[i] = vpf_open_table(path,(storage_type) storage,"rb",NULL);

   }

   vpf_close_table( &fcs );

   return fcrel;
}


/**************************************************************************
 *
 *N  fc_row_number
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Given the starting row of a feature class relationship, return the
 *    row number of the table at the end of the feature class relate
 *    chain.
 *    If your relate goes from the feature to the primitive, this will
 *    return the primitive id for the given feature row.
 *    If your relate goes from the primitive to the feature, this will
 *    return the feature id of the given primitive row.
 *
 *    Currently only supports relates on 'I', 'S', or 'K' fields.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels  DOS Turbo C
 *                    UNIX mdb port - Dec 1992
 *E
 *************************************************************************/
int fc_row_number( row_type row, fcrel_type fcrel, int tile )
{
   row_type relrow;
   int count;
   int i, rownum, keyval;
   short int sval;
   id_triplet_type triplet_keyval;
   int KEY1_, KEY_, ID_;
   position_type p;
   vpf_relate_struct rcell;

   p = ll_first(fcrel.relate_list);
   ll_element(p,&rcell);
   KEY1_ = table_pos(rcell.key1,fcrel.table[0]);
   ID_ = table_pos("ID",fcrel.table[0]);

   get_table_element(ID_,row,fcrel.table[0],&rownum,&count);

   if (KEY1_ == ID_) {     /* "ID" */
      keyval = rownum;
   } else {
      switch (fcrel.table[0].header[KEY1_].type) {
	 case 'I':
	    get_table_element(KEY1_,row,fcrel.table[0],&keyval,&count);
	    break;
	 case 'S':
	    get_table_element(KEY1_,row,fcrel.table[0],&sval,&count);
            keyval = sval;
	    break;
	 case 'K':
	    get_table_element(KEY1_,row,fcrel.table[0],&triplet_keyval,
			      &count);
	    keyval = triplet_keyval.exid;
	    if (tile != triplet_keyval.tile) {
	       return -2;
	    }
	    break;
	 default:
	    keyval = 0;
	    break;
      }
   }

   p = ll_first(fcrel.relate_list);
   for (i=1;i<(fcrel.nchain-1);i++) {
      /* Relate through Join table(s) */
      rownum = related_row(&keyval,fcrel.table[i],rcell.key2,tile);
      if (rownum < 1) break;
      relrow = get_row(rownum,fcrel.table[i]);

      p = ll_next(p);
      ll_element(p,&rcell);
      KEY_ = table_pos(rcell.key1,fcrel.table[i]);
      ID_ = table_pos("ID",fcrel.table[i]);

      if (KEY_ == ID_) {
	 keyval = rownum;
      } else {
	 switch (fcrel.table[i].header[KEY_].type) {
	 case 'I':
	    get_table_element(KEY_,relrow,fcrel.table[i],&keyval,&count);
	    break;
	 case 'S':
	    get_table_element(KEY_,relrow,fcrel.table[i],&sval,&count);
            keyval = sval;
	    break;
	 case 'K':
	    get_table_element(KEY_,relrow,fcrel.table[i],&triplet_keyval,
			      &count);
	    keyval = triplet_keyval.exid;
	    if (tile != triplet_keyval.tile) {
	       return -2;
	    }
	    break;
	 default:
	    keyval = 0;
	    break;
	 }
      }

      free_row(relrow,fcrel.table[i]);
   }

   if (rownum < 1) return 0;

   if (stricmp(rcell.key2,"ID")==0)
      rownum = keyval;
   else
      rownum = related_row(&keyval,fcrel.table[i],rcell.key2,tile);

   return rownum;
}


/**************************************************************************
 *
 *N  fc_row_numbers
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Given the starting row of a feature class relationship, return the
 *    list of row numbers of the table at the end of the feature class
 *    relate chain.
 *    If your relate goes from the feature to the primitive, this will
 *    return the primitive ids for the given feature row.
 *    If your relate goes from the primitive to the feature, this will
 *    return the feature ids of the given primitive row.
 *
 *    Currently only supports relates on 'I', 'S', or 'K' fields.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels  DOS Turbo C
 *E
 *************************************************************************/
linked_list_type fc_row_numbers( row_type row,
				 fcrel_type fcrel,
				 int tile )
{
   row_type relrow;
   int count;
   int n, rownum, keyval;
   short int shortval;
   id_triplet_type triplet_keyval;
   int KEY1_, KEY_;
   position_type p, prow, pkey;
   vpf_relate_struct rcell;
   linked_list_type rowlist, keylist, templist;
   int ID_;

   p = ll_first(fcrel.relate_list);
   ll_element(p,&rcell);
   KEY1_ = table_pos(rcell.key1,fcrel.table[0]);
   ID_ = table_pos("ID",fcrel.table[0]);

   get_table_element(ID_,row,fcrel.table[0],&rownum,&count);

   if (KEY1_ == ID_) {
      keyval = rownum;
   } else {
      switch (fcrel.table[0].header[KEY1_].type) {
	 case 'I':
	    get_table_element(KEY1_,row,fcrel.table[0],&keyval,&count);
	    break;
	 case 'S':
	    get_table_element(KEY1_,row,fcrel.table[0],&shortval,&count);
            keyval = shortval;
	    break;
	 case 'K':
	    get_table_element(KEY1_,row,fcrel.table[0],&triplet_keyval,
			      &count);
	    keyval = triplet_keyval.exid;
	    if ( (tile) && (tile != triplet_keyval.tile)) {
	       keyval = -2;
	    }
	    break;
	 default:
	    keyval = 0;
	    break;
      }
   }

   keylist = ll_init();
   ll_insert(&keyval,sizeof(keyval),keylist);

   n = 0;

   p = ll_first(fcrel.relate_list);
   for (n=1;n<(fcrel.nchain-1);n++) {

      /* Relate through Join table(s) */

      if (!fcrel.table[n].fp) {
         continue;
      }	 

      rowlist = ll_init();
      pkey = ll_first(keylist);
      while (!ll_end(pkey)) {
	 ll_element(pkey,&keyval);
	 templist = related_rows(&keyval,fcrel.table[n],rcell.key2,tile);
	 prow = ll_first(templist);
	 while (!ll_end(prow)) {
	    ll_element(prow,&rownum);
	    if (!ll_locate(&rownum,rowlist))
	       ll_insert(&rownum,sizeof(rownum),ll_last(rowlist));
	    prow = ll_next(prow);
	 }
	 ll_reset(templist);
	 pkey = ll_next(pkey);
      }
      ll_reset(keylist);

      p = ll_next(p);
      ll_element(p,&rcell);

      KEY_ = table_pos(rcell.key1,fcrel.table[n]);
      ID_ = table_pos("ID",fcrel.table[n]);

      keylist = ll_init();
      if (ll_empty(rowlist)) break;
      prow = ll_first(rowlist);
      while (!ll_end(prow)) {
	 ll_element(prow,&rownum);
	 relrow = get_row(rownum,fcrel.table[n]);

	 if (KEY_ == ID_) {
	    keyval = rownum;
	 } else {
	    switch (fcrel.table[n].header[KEY_].type) {
	    case 'I':
	       get_table_element(KEY_,relrow,fcrel.table[n],&keyval,&count);
	       break;
	    case 'S':
	       get_table_element(KEY_,relrow,fcrel.table[n],&shortval,&count);
               keyval = shortval;
	       break;
	    case 'K':
	       get_table_element(KEY_,relrow,fcrel.table[n],&triplet_keyval,
				 &count);
	       keyval = triplet_keyval.exid;
	       if ((tile) && (tile != triplet_keyval.tile)) {
		  keyval = -2;
	       }
	       break;
	    default:
	       keyval = 0;
	       break;
	    }
	 }
	 if (keyval > 0)
	    ll_insert(&keyval,sizeof(keyval),ll_last(keylist));
	 prow = ll_next(prow);
	 free_row(relrow,fcrel.table[n]);
      }
      ll_reset(rowlist);
   }

   rowlist = ll_init();
   if (ll_empty(keylist)) return rowlist;

   if (!fcrel.table[n].fp) {
      ll_reset(keylist);
      return rowlist;
   }

   p = ll_first(keylist);
   while (!ll_end(p)) {
      ll_element(p,&keyval);

      templist = related_rows(&keyval,fcrel.table[n],rcell.key2,tile);
      prow = ll_first(templist);
      while (!ll_end(prow)) {
	 ll_element(prow,&rownum);
	 if (!ll_locate(&rownum,rowlist))
	    ll_insert(&rownum,sizeof(rownum),ll_last(rowlist));
	 prow = ll_next(prow);
      }
      ll_reset(templist);
      p = ll_next(p);
   }
   ll_reset(keylist);

   return rowlist;
}


/**************************************************************************
 *
 *N  deselect_feature_class_relate
 *
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   Purpose:
 *P
 *    Clear out a previously allocated feature class relate structure
 *    from memory.
 *E
 *:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 *
 *   History:
 *H
 *    Barry Michaels  DOS Turbo C
 *                    UNIX mdb port - Dec 1992
 *E
 *************************************************************************/
void deselect_feature_class_relate( fcrel_type *fcrel )
{
   register int i;

   if (fcrel->nchain > 0) {
      for (i=0;i<fcrel->nchain;i++) {
	 if (fcrel->table[i].status == OPENED) {
	    vpf_close_table(&(fcrel->table[i]));
	 }
      }
      free(fcrel->table);
      ll_reset(fcrel->relate_list);
   }
   fcrel->nchain = 0;
}

