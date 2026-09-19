static char SCCS[] = "%Z% %M% %I% %G%";
/*************************************************************************
*
*N  Module VVTHEME - Theme Manipulation Module
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function contains functions that allow the user to create and 
*     manipulate the themes of a view.  A theme is a set of rows from a 
*     VPF feature table defined by a logical selection expression that 
*     are assigned a graphical display symbol and given a textual 
*     description.
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
*
*    Scott Simon	Oct 1992                         UNIX mdb version
*E
*************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include "vvtheme.h"
#include "vvmisc.h"
#include "vpftable.h"
#include "vpfprop.h"
extern int vverr;

/*************************************************************************
*
*N  create_theme
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function creates a new theme in the specified view with the
*     given values.  All parameter values must be valid and correct for
*     this function to succeed.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     view          <input> == (view_type *) view structure.
*     description   <input> == (char *) theme description.
*     expression    <input> == (char *) logical selection expression.
*     database      <input> == (char *) database path.
*     library       <input> == (char *) library name.
*     coverage      <input> == (char *) coverage name.
*     feature_class <input> == (char *) feature class name.
*     symbol        <input> == (theme_symbol_type) symbol structure.
*     create_theme  <output> == (int) status of the create_theme function
*                               0 = failure
* 				1 = success
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   External Variables:
*X
*     vverr       <output> ==  (int) Set only if create_view fails
*                     	       1 - file I/O error
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*
*    Scott Simon	Oct 1992                         UNIX mdb version
*E
*************************************************************************/
int create_theme( view_type *view,
                  char *description,
                  char *expression,
                  char *database,
                  char *library,
                  char *coverage,
                  char *feature_class,
                  theme_symbol_type symbol )
{
   int i, themenum, nsym;
   char path[255], **sym, sep[2] = {VPF_DIR_SEPARATOR,'\0'};
   FILE *fp;

   themenum = view->nthemes;

   /* Realloc view with another theme (malloc if view->nthemes==0) */
   if (view->nthemes == 0) {
      view->theme = (theme_type *)checkmalloc(sizeof(theme_type));
   } else {
      view->theme = (theme_type *)checkrealloc(view->theme,
                                   (view->nthemes+1)*sizeof(theme_type));
      set_nuke(&view->selected);
      set_nuke(&view->displayed);
   }
   view->nthemes++;

   /* Reinitialize selected and displayed sets with another theme */
   view->selected = set_init(view->nthemes+1);
   view->displayed = set_init(view->nthemes+1);

   /* Set new theme with parameter values */

   view->theme[themenum].description = (char *)checkmalloc(
					       strlen(description)+1);
   strcpy(view->theme[themenum].description,description);
   rightjust(view->theme[themenum].description);

   view->theme[themenum].database = (char *)checkmalloc(strlen(database)+1);
   strcpy(view->theme[themenum].database,database);
   rightjust(view->theme[themenum].database);

   view->theme[themenum].library = (char *)checkmalloc(strlen(library)+1);
   strcpy(view->theme[themenum].library,library);
   rightjust(view->theme[themenum].library);

   view->theme[themenum].coverage = (char *)checkmalloc(strlen(coverage)+1);
   strcpy(view->theme[themenum].coverage,coverage);
   rightjust(view->theme[themenum].coverage);

   view->theme[themenum].fc = (char *)checkmalloc(strlen(feature_class)+1);
   strcpy(view->theme[themenum].fc,feature_class);
   rightjust(view->theme[themenum].fc);

   view->theme[themenum].expression = (char *)checkmalloc(
					      strlen(expression)+1);
   strcpy(view->theme[themenum].expression,expression);
   rightjust(view->theme[themenum].expression);

   strcpy(path,database);
   vpfcatpath(path,sep);
   vpfcatpath(path,library);
   vpfcatpath(path,sep);

   view->theme[themenum].ftable = feature_class_table(path,coverage,
						      feature_class);

   view->theme[themenum].primclass = feature_class_primitive_type(path,
					  coverage,feature_class);

   view->theme[themenum].symbol.point_color=symbol.point_color;
   view->theme[themenum].symbol.point=symbol.point;
   view->theme[themenum].symbol.line_color=symbol.line_color;
   view->theme[themenum].symbol.line=symbol.line;
   view->theme[themenum].symbol.area_color=symbol.area_color;
   view->theme[themenum].symbol.area=symbol.area;
   view->theme[themenum].symbol.text_color=symbol.text_color;
   view->theme[themenum].symbol.text=symbol.text;

   /* Write out new 'themes' file in view */
   sprintf(path,"%s%s%cthemes",view->path,view->name,DIR_SEPARATOR);
   fp = fopen(path,"at");
   if (!fp) {
      fprintf(stderr,"create_theme: Error opening %s\n",path);
      vverr = 1;
      return 0;
   }
   fprintf(fp,"%s\n%s\n%s\n%s\n%s\n%s\n",
	   view->theme[themenum].description,
	   view->theme[themenum].database,
	   view->theme[themenum].library,
	   view->theme[themenum].coverage,
	   view->theme[themenum].fc,
	   view->theme[themenum].expression);

   fclose(fp);

   /* Not saving selection set file for theme here, */
   /* in case you were wondering.                   */

   /* Update all '.sym' files in view */
   sprintf(path,"%s%s",view->path,view->name);
   sym = findall(path,"^.*\\.sym$",&nsym);
   for (i=0;i<nsym;i++) {

      sprintf(path,"%s%s%c%s",view->path,view->name,DIR_SEPARATOR,sym[i]);

      fp = fopen(path,"at");
      if (!fp) {
	 fprintf(stderr,"create_theme: Error opening %s\n",path);
      } else {
	 fprintf(fp,"%d %d %d %d %d %d %d %d\n",
		    view->theme[themenum].symbol.point_color,
		    view->theme[themenum].symbol.point,
		    view->theme[themenum].symbol.line_color,
		    view->theme[themenum].symbol.line,
		    view->theme[themenum].symbol.area_color,
		    view->theme[themenum].symbol.area,
		    view->theme[themenum].symbol.text_color,
		    view->theme[themenum].symbol.text);
	 fclose(fp);
      }
      free(sym[i]);
   }
   if (nsym > 0 && sym) free(sym);

   return 1;
} /* end create_theme() */

/*************************************************************************
*
*N  modify_theme
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function modify the specified theme in the view with the given
*     values.  All parameter values must be valid and correct for this
*     function to succeed. 
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     view          <input>  == (view_type *) view structure.
*     theme_number  <input>  == (int) theme number to modify.
*     description   <input>  == (char *) theme description.
*     expression    <input>  == (char *) logical selection expression.
*     database      <input>  == (char *) database path.
*     library       <input>  == (char *) library name.
*     coverage      <input>  == (char *) coverage name.
*     feature_class <input>  == (char *) feature class name.
*     symbol        <input>  == (theme_symbol_type) symbol structure.
*     modify_theme  <output> == (int) status of the create_theme function
*                               0 = failure
* 				1 = success
*     
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   External Variables:
*X
*     vverr       <output> ==  (int) Set only if modify_view fails
*                     	       1 = file I/O error
*                              2 = invalid theme number
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*
*    Scott Simon	Oct 1992                         UNIX mdb version
*E
*************************************************************************/
int modify_theme( view_type *view,
                  int theme_number,
                  char *description,
                  char *expression,
                  char *database,
                  char *library,
                  char *coverage,
                  char *feature_class,
                  theme_symbol_type symbol )
{
   int i,j, exprchanged;
   char path[255], num[20], sep[2]={VPF_DIR_SEPARATOR,'\0'};
   FILE *fp;
   char *prname[] = {"","edg","fac","txt","end","cnd"};

   if (theme_number < 0 || theme_number >= view->nthemes) {
      vverr = 2;
      return 0;
   }

   if (strcmp(view->theme[theme_number].expression,expression)==0)
      exprchanged = 0;
   else
      exprchanged = 1;

   free(view->theme[theme_number].description);
   free(view->theme[theme_number].database);
   free(view->theme[theme_number].library);
   free(view->theme[theme_number].coverage);
   free(view->theme[theme_number].fc);
   free(view->theme[theme_number].expression);
   free(view->theme[theme_number].ftable);

   /* Set theme with new parameter values */

   view->theme[theme_number].description = (char *)checkmalloc(
					       strlen(description)+1);
   strcpy(view->theme[theme_number].description,description);

   view->theme[theme_number].database = (char *)checkmalloc(strlen(database)+1);
   strcpy(view->theme[theme_number].database,database);

   view->theme[theme_number].library = (char *)checkmalloc(strlen(library)+1);
   strcpy(view->theme[theme_number].library,library);

   view->theme[theme_number].coverage = (char *)checkmalloc(strlen(coverage)+1);
   strcpy(view->theme[theme_number].coverage,coverage);

   view->theme[theme_number].fc=(char *)checkmalloc(strlen(feature_class)+1);
   strcpy(view->theme[theme_number].fc,feature_class);

   view->theme[theme_number].expression = (char *)checkmalloc(
					      strlen(expression)+1);
   strcpy(view->theme[theme_number].expression,expression);

   strcpy(path,database);
   vpfcatpath(path,sep);
   vpfcatpath(path,library);
   vpfcatpath(path,sep);

   view->theme[theme_number].ftable = feature_class_table(path,coverage,
						      feature_class);

   view->theme[theme_number].primclass = feature_class_primitive_type(path,
					  coverage,feature_class);

   view->theme[theme_number].symbol = symbol;

   /* Write out new 'themes' file in view */
   sprintf(path,"%s%s%cthemes",view->path,view->name,DIR_SEPARATOR);
   fp = fopen(path,"wt");
   if (!fp) {
      fprintf(stderr,"create_theme: Error opening %s\n",path);
      vverr = 1;
      return 0;
   }
   for (i=0;i<view->nthemes;i++) {
      fprintf(fp,"%s\n%s\n%s\n%s\n%s\n%s\n",
	      view->theme[i].description,
	      view->theme[i].database,
	      view->theme[i].library,
	      view->theme[i].coverage,
	      view->theme[i].fc,
	      view->theme[i].expression);
   }

   fclose(fp);

   if (exprchanged) {
      /* Not saving the new selection set files for theme here, */
      /* but deleting the old ones, if present                  */
      for (i=EDGE;i<=CONNECTED_NODE;i++) {
         sprintf(path,"%s%s%c%s",
                 view->path,view->name,DIR_SEPARATOR,prname[i]);
         sprintf(num,"%d",theme_number);
         for (j=0;j<(5-strlen(num));j++)
            strcat(path,"0");
         strcat(path,num);
         if (access(path,0)==0) remove(path);
         strcat(path,".til");
         if (access(path,0)==0) remove(path);
      }
      set_delete(theme_number,view->displayed);
   }


   return 1;
} /* end modify_theme() */


/*************************************************************************
*
*N  change_theme_symbology
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function change just the symbology stored in memory for the
*     specified theme in the view.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     view                   <input>  == (view_type *) view structure.
*     theme_number           <input>  == (int) theme number.
*     symbol                 <input>  == (theme_symbol_type) symbol struct.
*     change_theme_symbology <output> == (int) status of the 
*  					 change_theme_symbology function
*                                        0 = failure
* 				         1 = success
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   External Variables:
*X
*     vverr       <output> ==  (int) Set only if change_theme_symbology 
*			       fails
*                              1 = invalid theme number
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*
*    Scott Simon	Oct 1992                         UNIX mdb version
*E
*************************************************************************/
int change_theme_symbology( view_type *view,
                            int theme_number,
                            theme_symbol_type symbol )
{
   if (theme_number < 0 || theme_number >= view->nthemes) {
      vverr = 1;
      return 0;
   }
   view->theme[theme_number].symbol = symbol;
   return 1;
}

/*************************************************************************
*
*N  assign_theme
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function copies a theme_type structure into a temp structure
*     and returns the the copied structure.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     theme        <input>  == (theme_type) theme_type structure to 
*			       be copied.
*     assign_theme <output> == (theme_type)  to the copied theme structure.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992                    UNIX mdb version
*E
*************************************************************************/
static theme_type assign_theme( theme_type theme )
{
  theme_type temp_theme;

  if (theme.description)
    temp_theme.description = (char *)strdup(theme.description);
  else
    temp_theme.description = NULL;
  if (theme.database)
    temp_theme.database = (char *)strdup(theme.database);
  else
    temp_theme.database = NULL;
  if (theme.library)
    temp_theme.library = (char *)strdup(theme.library);
  else
    temp_theme.library = NULL;
  if (theme.coverage)
    temp_theme.coverage = (char *)strdup(theme.coverage);
  else
    temp_theme.coverage = NULL;
  if (theme.fc)
    temp_theme.fc = (char *)strdup(theme.fc);
  else
    temp_theme.fc = NULL;
  if (theme.ftable)
    temp_theme.ftable = (char *)strdup(theme.ftable);
  else
    temp_theme.ftable = NULL;
  temp_theme.primclass = theme.primclass;
  if (theme.expression)
    temp_theme.expression = (char *)strdup(theme.expression);
  else
    temp_theme.expression = NULL;
  temp_theme.symbol = theme.symbol;

  return temp_theme;
}

/*************************************************************************
*
*N  delete_theme
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function deletes the specified theme from the view.  
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     view          <input>  == (view_type *) view structure to delete 
*				specified theme from.
*     theme_number  <input>  == (int) number of the theme to delete.
*     delete_theme  <output> == (int) status of the delete_theme function
*                               0 = failure
* 				1 = success
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   External Variables:
*X
*     vverr       <output> ==  (int) Set only if delete_theme fails
*                              1 = file I/O error
*                              2 = invalid theme number
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992                         UNIX mdb version
*E
*************************************************************************/
int delete_theme( view_type *view,
                  int theme_number )
{
  char path[255], cmd[511], copy[255], num[5]="    \0";
  char **symfiles, **fslfiles;
  FILE *fp, *symfp, *fslfp;
  int i,j,k,nsymfiles,nfslfiles,themenum;
  int point_color, point_symbol, line_color, line_symbol,
      area_color, area_symbol, text_color, text_symbol;
  position_type p;
  char *prname[] = {"","edg","fac","txt","end","cnd"};

  /* Check if theme_number is valid */

  if (theme_number < 0 || theme_number >= view->nthemes) {
    vverr = 2;
    return 0;
  }

  /* remove selected theme */

  if (set_member(theme_number,view->selected)) {
    set_delete(theme_number,view->selected);
    ll_delete(ll_locate(&theme_number,view->sellist));
  }

  /* remove theme if it is displayed */

  if (set_member(theme_number,view->displayed)) 
    set_delete(theme_number,view->displayed);

  /* move remaining themes up */

  for(i=theme_number;i<(view->nthemes-1);i++) {
    if (view->theme[i].description) free(view->theme[i].description);
    if (view->theme[i].database)    free(view->theme[i].database);
    if (view->theme[i].library)     free(view->theme[i].database);
    if (view->theme[i].coverage)    free(view->theme[i].database);
    if (view->theme[i].fc)          free(view->theme[i].database);
    if (view->theme[i].ftable)      free(view->theme[i].database);
    if (view->theme[i].expression)  free(view->theme[i].database);
    view->theme[i] = assign_theme( view->theme[i+1] );
    if (set_member(i+1,view->selected)) {
      set_delete(i+1,view->selected);
      set_insert(i,view->selected);
      k = i+1;
      ll_replace(&i,ll_locate(&k,view->sellist));
    }
    if (set_member(i+1,view->displayed)) {
      set_delete(i+1,view->displayed);
      set_insert(i,view->displayed);
    }
  }

  view->selected.size--;
  view->displayed.size--;

  /* Selection set list */
  /* First, delete the selected theme, if present */
  p = ll_first(view->sellist);
  while (!ll_end(p)) {
     ll_element(p,&themenum);
     if (themenum == theme_number) 
        ll_delete(p);
     else
        p = ll_next(p);
  }
  /* Then, adjust the theme numbers */
  p = ll_first(view->sellist);
  while (!ll_end(p)) {
     ll_element(p,&themenum);
     if (themenum >= theme_number) {
        themenum--;
        ll_replace(&themenum,p);
     }
     p = ll_next(p);
  }

  view->nthemes--;

  /* rewrite the theme file */

  strcpy(path,view->path);
  vpfcatpath(path,view->name);
  vpfcatpath(path,"/themes");
  fp = fopen(path,"w");
  if (!fp) {
    vverr = 2;
    return 0;
  }
  for(i=0;i<view->nthemes;i++) {
    fprintf(fp,"%s\n%s\n%s\n%s\n%s\n%s\n", 
            view->theme[i].description,
	    view->theme[i].database,
	    view->theme[i].library,
	    view->theme[i].coverage,
            view->theme[i].fc,
            view->theme[i].expression);
  }
  fclose(fp);

  /* write new symbol files */

  strcpy(path,view->path);
  vpfcatpath(path,view->name);
  vpfcatpath(path,"/");

  symfiles = findall(path,"^.*\\.sym$",&nsymfiles);

  for(i=0;i<nsymfiles;i++) {
    strcpy(path,view->path);
    vpfcatpath(path,view->name);
    vpfcatpath(path,"/");
    sprintf(cmd,"cp %s%s temp.sym",path,symfiles[i]);
    system(cmd);
    vpfcatpath(path,symfiles[i]);
    fp = fopen(path,"w");
    if (!fp) {
      vverr = 2;
      return 0;
    }
    symfp = fopen("temp.sym","r");
    for(j=0;j<=view->nthemes;j++){
      fscanf(symfp,"%d %d %d %d %d %d %d %d\n",
	     &point_color, &point_symbol,
	     &line_color, &line_symbol,
	     &area_color, &area_symbol, 
	     &text_color, &text_symbol);
      if (j != theme_number)
	fprintf(fp,"%3d %3d %3d %3d %3d %3d %3d %3d\n",
		point_color, point_symbol,
		line_color, line_symbol,
		area_color, area_symbol,
		text_color, text_symbol);
    }
    fclose(fp);
    fclose(symfp);
  }

  remove("temp.sym");


  /* Update any FSL files */

  strcpy(path,view->path);
  vpfcatpath(path,view->name);
  vpfcatpath(path,"/");

  fslfiles = findall(path,"^.*\\.fsl$",&nfslfiles);

  for(i=0;i<nfslfiles;i++) {
    strcpy(path,view->path);
    vpfcatpath(path,view->name);
    vpfcatpath(path,"/");
    sprintf(cmd,"cp %s%s temp.fsl",path,fslfiles[i]);
    system(cmd);
    vpfcatpath(path,fslfiles[i]);
    fp = fopen(path,"w");
    if (!fp) {
      vverr = 2;
      return 0;
    }
    fslfp = fopen("temp.fsl","r");
    while (!feof(fslfp)) {
       fread(&themenum,sizeof(themenum),1,fslfp);
       if (themenum < theme_number) {
          fwrite(&themenum,sizeof(themenum),1,fp);
       } else if (themenum > theme_number) {
          themenum--;
          fwrite(&themenum,sizeof(themenum),1,fp);
       }
    }
    fclose(fp);
    fclose(fslfp);
  }
 
  if (fileaccess("temp.fsl",0)==0) remove("temp.fsl");

  /* selection set file cleanup */

  for (i=EDGE; i<=CONNECTED_NODE; i++) {
     strcpy(path,view->path);
     vpfcatpath(path,view->name);
     vpfcatpath(path,"/");
     vpfcatpath(path,prname[i]);
     sprintf(num,"%d",theme_number);
     for(k=0;k<(5-strlen(num));k++) strcat(path,"0");
     strcat(path,num);
     if (access(path,0)==0) {
        sprintf(cmd,"rm -f %s",path);
        system(cmd);
     }
     strcat(path,".til");
     if (access(path,0)==0) {
        sprintf(cmd,"rm -f %s",path);
        system(cmd);
     }
  }

  for(i=theme_number;i<view->nthemes;i++) {
    for (j=EDGE;j<=CONNECTED_NODE;j++) {
      strcpy(path,view->path);
      vpfcatpath(path,view->name);
      vpfcatpath(path,"/");
      vpfcatpath(path,prname[j]);
      strcpy(copy,path);
      sprintf(num,"%d",i+1);
      for(k=0;k<(5-strlen(num));k++)strcat(path,"0");
      strcat(path,num);
      sprintf(num,"%d",i);
      for(k=0;k<(5-strlen(num));k++)strcat(copy,"0");
      strcat(copy,num);
      sprintf(cmd,"mv %s %s",path,copy);
      if (fileaccess(path,0)==0)
         system(cmd);
      strcat(path,".til");
      strcat(copy,".til");
      sprintf(cmd,"mv %s %s",path,copy);
      if (fileaccess(path,0)==0)
         system(cmd);
    }
  }
  for (i=EDGE;i<=CONNECTED_NODE;i++) {
    strcpy(path,view->path);
    vpfcatpath(path,view->name);
    vpfcatpath(path,"/");
    vpfcatpath(path,prname[i]);
    sprintf(num,"%d",view->nthemes);
    for(k=0;k<(5-strlen(num));k++)strcat(path,"0");
    strcat(path,num);
    if (fileaccess(path,0)==0) remove(path);
  }


  return 1;
}




