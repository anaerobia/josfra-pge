static char SCCS[] = "%Z% %M% %I% %G%";
/*************************************************************************
*
*N  Module VVUTIL
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     Contains functions that create, delete, copy, and rename "views".  
*     A view in the VPFVIEW application is a user-defined and 
*     customizable look at stored VPF data.  A view defines the data 
*     sources as well as user-defined feature selections and symbology.
*     It is physically implemented as a directory on the file system
*     with a predefined set of file structures.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*    N/A
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992                    UNIX mdb version
*E
*************************************************************************/
#include "vvutil.h"
#include "vvinit.h"

/*************************************************************************
*
*N  create_view
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function creates a new view in the given workspace, with the 
*     specified view name, using the list of database-library name pairs 
*     to be contained in the view. A symbol set path may be specified for
*     the view.
*     If the specified workspace and view name already exist, it will 
*     be overwritten.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     workspace   <input> == (char *) workspace path to create view in.
*     view_name   <input> == (char *) name of view to be created.
*     db_lib_list <input> == (db_lib_type) array of database-library name
*                            pairs that can be accessed by the view.
*     num_db_lib  <input> == (int) number of database-library name pairs
*                            contained in the db_lib_list.
*     sympath     <input> == (char *) optional path to symbol set.
*                             NULL defaults to the symbol set in $VVHOME.
*     create_view <output> == (int) status of the create_view function
*                             0 - failure
*                             1 - success
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   External Variables:
*X
*     vverr       <output> ==  (int) Set only if create_view fails
*                              1 = invalid workspace
*                              2 = access violation
*                              3 = general failure
*                              4 = sympath not present
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992                    UNIX mdb version
*E
*************************************************************************/
int create_view( char *workspace,
                 char *view_name,
                 db_lib_type *db_lib_list,
                 int num_db_lib,
                 char *sympath )
{
  extern int vverr;
  char *full_path,*del_path,*env_path;
  char **dbs,**libraries;
  int i,j,length,num_unique_dbs=0,match, liblen;
  FILE *fp;
  view_type view;

  /* Verify workspace exists */
  if (fileaccess(workspace,0))
  {
    vverr = 1;
    return 0;
  }

  /* Verify execute and write permissions */
  if ((fileaccess(workspace,1))||(fileaccess(workspace,2)))
  {
    vverr = 2;
    return 0;
  }

  /* Check if sympath exists */
  if (sympath)
    if (fileaccess(sympath,0))
    {
      vverr = 4;
      return 0;
    }

  /* Check for trailing separator character */
  full_path = (char *) checkmalloc(strlen(workspace)+strlen(view_name)+4);
  strcpy(full_path,workspace);
  length = strlen(full_path);
  if (full_path[length-1] != DIR_SEPARATOR) {
    full_path[length] = DIR_SEPARATOR;
    full_path[length+1] = '\0';
  }
  /* Check for trailing percent character */
  strcat(full_path,view_name);
  length = strlen(full_path);
  if (full_path[length-1] != '%') {
    full_path[length] = '%';
    full_path[length+1] = '\0';
  }
  /* If View already exists remove it */
  if (!fileaccess(full_path,0)) {
    del_path = (char *) checkmalloc(strlen(full_path)+10);
    sprintf(del_path,"rm -rf %s",full_path);
    system(del_path);
    free(del_path);
    /* if unable to delete view directory return error */
    if (!fileaccess(full_path,0)) {
      free(full_path);
      vverr = 3;
      return 0;
    }
  }

  /* Create the View directory */
  if (mkdir(full_path,493)) {
    free(full_path);
    vverr = 3;
    return 0;
  } 
 
  /* length of the concatenated library string should
     never get longer than:  */
  liblen = 20*num_db_lib;  

  match = 0;
  dbs = (char **) checkmalloc((num_db_lib+1)*sizeof(char *));
  libraries = (char **) checkmalloc((num_db_lib+1)*sizeof(char *));
  /* The following generates a list of unique database names with */
  /* corresponding library names.                                 */ 
  for (i=0;i<num_db_lib;i++) {
    /* Compare the db passed to list of unique dbs */
    for (j=0;j<num_unique_dbs;j++) {
      if (!stricmp(db_lib_list[i].db,dbs[j])) {
        /* if db names match, add the lib name to existing libraries list */
        strcat(libraries[j]," ");
        strcat(libraries[j],db_lib_list[i].lib);
        match = 1;
      }
    } /* for j */
    /* if no matching database name, add it and lib name to unique list */
    if (!match) {
      ++num_unique_dbs;
      dbs[num_unique_dbs-1]=(char *)checkmalloc(strlen(db_lib_list[i].db)+4);
      strcpy(dbs[num_unique_dbs-1],db_lib_list[i].db); 
      libraries[num_unique_dbs-1] = (char *) checkmalloc(liblen);
      strcpy(libraries[num_unique_dbs-1],db_lib_list[i].lib); 
    }
    match = 0;
  } /* for i */

  /* Create the ENV file */
  length = strlen(full_path);
  env_path = (char *) checkmalloc(length+8);
  strcpy(env_path,full_path);
  env_path[length] = DIR_SEPARATOR;
  env_path[length+1] = '\0';
  strcat(env_path,"env");
  fp = fopen(env_path,"w");
  if (fp == NULL) {
    vverr = 3;
    free(env_path);
    free(full_path);
    for (i=0;i<num_unique_dbs;i++) {
      free(dbs[i]);
      free(libraries[i]);
    }
    free(dbs);
    free(libraries);
    return 0;
  }
  /* Write out each database name and associated libraries to ENV file */
  for (i=0;i<num_unique_dbs;i++) {
    fprintf(fp,"%s %s\n",dbs[i],libraries[i]);
  }
  /* Write out the symbol path to ENV file if it exists */
  if (sympath)
    fprintf(fp,"%s\n",sympath);

  fclose(fp);

  /* free up any variables used */
  free(env_path);
  free(full_path);
  for (i=0;i<num_unique_dbs;i++) {
    free(dbs[i]);
    free(libraries[i]);
  }
  free(dbs);
  free(libraries);

  return 1;
}

/*************************************************************************
*
*N  delete_view
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function deletes the view at the specified path.  
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     viewpath    <input>  == (char *) path to view to be deleted.
*                             Assumed to have a valid view name with a
*                             percent trailing the name.
*     delete_view <output> == (int) status of the delete_view function
*                             0 - failure
*                             1 - success
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   External Variables:
*X
*     vverr       <output> ==  (int) Set only if delete_view fails
*                              1 = invalid path
*                              2 = access violation
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992                    UNIX mdb version
*E
*************************************************************************/
int delete_view( char *viewpath )
{
  extern int vverr;
  char *command;

  /* Check if viewpath exists */
  if (fileaccess(viewpath,0)) {
    vverr = 1;
    return 0;
  }

  /* Check if have write permissions */
  if (fileaccess(viewpath,2)) {
    vverr = 2;
    return 0;
  }

  command = (char *) checkmalloc(strlen(viewpath)+7);
  sprintf(command,"rm -rf %s",viewpath);
  /* Delete view */
  system(command);

  /* If view still exists, return error */
  if (!fileaccess(viewpath,0)) {
    vverr = 2;
    free(command);
    return 0;
  }

  free(command);
  return 1;
}

/*************************************************************************
*
*N  copy_view
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function copies a source view to a specified destination.
*     The calling routine is assumed to check the source and dest for
*     valid view names ending with a percent (%).
*     If the dest already exists, it will be overwritten.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     source    <input>  == (char *) source path and view name to be 
*                                    copied.
*     dest      <input>  == (char *) destination path and view name to 
*                                    copy source view to.
*     copy_view <output> == (int) status of the copy_view function
*                           0 - failure
*                           1 - success
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   External Variables:
*X
*     vverr       <output> ==  (int) Set only if copy_view fails
*                              1 = source not found
*                              2 = access violation for source
*                              3 = invalid destination path
*                              4 = access violation for destination
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992                    UNIX mdb version
*E
*************************************************************************/
int copy_view( char *source, 
               char *dest )
{
  extern int vverr;
  char wksp[255];
  char *dest_path,*command;
  int i;

  /* if source and dest are identical return */
  if (!strcmp(source,dest)) 
    return 1;

  /* Check if source is a valid path */
  if (fileaccess(source,0)) {
    vverr = 1;
    return 0;
  }

  /* Check for read permission at source path */
  if (fileaccess(source,4)) {
    vverr = 2;
    return 0;
  }
 
  /* Strip off the destination view name from dest */
  dest_path = (char *) checkmalloc(strlen(dest)+1);
  strcpy(dest_path,dest);
  rightjust(dest_path);
  i = strlen(dest_path)-1;
  while (dest_path[i] != '/' && i>0) i--;
  if (dest_path[i] == '/') {
    /* ignore the dest view name */
    dest_path[i] = '\0'; 
  }
  else {
    /* if there is no directory specified in dest get the */
    /* current workspace                                  */
    getcwd(wksp,255);
    rightjust(wksp);
    if (wksp[strlen(wksp)-1] != '/') strcat(wksp,"/");
    checkrealloc(dest_path,strlen(wksp));
    strcpy(dest_path,wksp);
  }

  /* Check if dest is a valid path */
  if (fileaccess(dest_path,0))
  {
    free(command);
    free(dest_path);
    vverr = 3;
    return 0;
  }

  /* Check for execute and write permission at dest path */
  if ((fileaccess(dest_path,1))||(fileaccess(dest_path,2)))
  {
    free(command);
    free(dest_path);
    vverr = 4;
    return 0;
  }

  /* If destination already exists delete it */
  if (!fileaccess(dest,0)) 
    if (!delete_view(dest)) {
      free(command);
      free(dest_path);
      vverr = 4;
      return 0;
    }

  /* Copy source to destination */
  command = (char *) checkmalloc(strlen(source)+strlen(dest)+12);
  sprintf(command,"cp -r %s %s",source,dest);
  system(command);
 
  /* Check if new dest does not exist */
  if (fileaccess(dest,0)) {
    free(command);
    free(dest_path);
    vverr = 4;
    return 0;
  }

  free(command);
  free(dest_path);
  return 1;
}

/*************************************************************************
*
*N  rename_view
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function renames the given view to the new name. The newname
*     may or may not contain a path. It is followed by a view name.
*     The calling routine is assumed to check the inpath and newname for
*     valid view names ending with a percent (%).
*     If newname already exists, it will be overwritten.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     inpath       <input>  == (char *) view to be renamed.
*     newname      <input>  == (char *) new name for view.
*     rename_view  <output> == (int) status of the rename_view function
*                              0 - failure
*                              1 - success 
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   External Variables:
*X
*     vverr       <output> ==  (int) Set only if rename_view fails
*                              1 = input path not found
*                              2 = access violation
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992                    UNIX mdb version
*E
*************************************************************************/
int rename_view( char *inpath,
                 char *newname )
{
  extern int vverr;
  char wksp[255];
  char *new_path;
  int i;

  /* Check if inpath exists */
  if (fileaccess(inpath,0)) {
    vverr = 1;
    return 0;
  }

  /* Check for read and write permission at inpath */
  if (fileaccess(inpath,6)) {
    vverr = 2;
    return 0;
  }

  /* Strip off the destination view name from newname */
  new_path = (char *) checkmalloc(strlen(newname)+1);
  strcpy(new_path,newname);
  rightjust(new_path);
  i = strlen(new_path)-1;
  while (new_path[i] != '/' && i>0) i--;
  if (new_path[i] == '/') {
    /* ignore the view name */
    new_path[i] = '\0'; 
  }
  else {
    /* if there is no directory specified in newname get the */
    /* current workspace                                     */
    getcwd(wksp,255);
    rightjust(wksp);
    if (wksp[strlen(wksp)-1] != '/') strcat(wksp,"/");
    checkrealloc(new_path,strlen(wksp));
    strcpy(new_path,wksp);
  }

  /* Check for write permission at new_path */
  if (fileaccess(new_path,2)) {
    free(new_path);
    vverr = 2;
    return 0;
  }

  /* If newname already exists delete it */
  if (!fileaccess(newname,0)) 
    if (!delete_view(newname)) {
      free(new_path);
      vverr = 2;
      return 0;
    }

  /* Rename inpath to newname */
  if (rename(inpath,newname)) {
    free(new_path);
    vverr = 2;
    return 0;
  }

  /* Check if inpath still exists and if newname does not */
  if ((!fileaccess(inpath,0)) || (fileaccess(newname,0))) {
    free(new_path);
    vverr = 2;
    return 0;
  }

  free(new_path);
  return 1;
}



