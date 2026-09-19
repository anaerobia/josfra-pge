static char SCCS[] = "%Z% %M% %I% %G%";
/*************************************************************************
*
*N  XTIFF.C
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     Tagged Image File Format Save and Restore for X Windows
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
*    Scott Simon	Oct 1992                    UNIX mdb version
*E
*************************************************************************/
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "xtiff.h" 
#include "tiffio.h"
#include "vvmisc.h"

/*************************************************************************
*
*N  X_save_tiff_image
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function saves the image in the given X window to the 
*     specified file.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     filename <input>  == (char *) full path and filename to save tiff 
*				    image in.
*     display  <input>  == (Display *) X display.
*     win      <input>  == (Window *) X Window.
*     gc       <input>  == (GC) X gc.
*     X_save_tiff_image <output> == (int) status of X_save_tiff_image 
*				    function.
*				    0 = successful    
*				    1 = file creation error
*				    2 = access violation
*				    3 = bad X object 
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992                    UNIX mdb version
*E
*************************************************************************/
int X_save_tiff_image( char *filename, Display *display,
                       Window win, GC gc )
{
  Window root;
  int i,x,y;
  unsigned int width,height,border_width,depth;
  Colormap cmap;
  XImage *image;
  XColor *colorcell_defs;
  TIFF *tif;
  unsigned char *buf,tempshort;
  unsigned long pixel,templong;
  TIFFDirEntry tag;
  char *fpath;
  extern char cworkspace[255];

  /* get the path to the filename */
  fpath = (char *) checkmalloc(strlen(filename));
  strcpy(fpath,filename);
  rightjust(fpath);
  i = strlen(fpath)-1;
  while (fpath[i] != '/' && i>0) i--;
  if (fpath[i] == '/') {
    /* parse out the path, ignoring the filename */
    fpath[i] = '\0';
  }
  else {
    /* no path given, use cworkspace */
    checkrealloc(fpath,strlen(cworkspace));
    strcpy(fpath,cworkspace);
  }

  if (fileaccess(fpath,2)) { /* access violation */
    free(fpath);
    return 2;   
  }

  if (win == NULL) {
    free(fpath);
    return 3; 		  /* bad X object */
  }

  /* open TIFF file */
  tif = TIFFOpen(filename,"w");
  if (tif == NULL) {
    free(fpath);
    return 1; 		  /* file creation error */
  }

  XGetGeometry(display,win,&root,&x,&y,&width,&height,&border_width,
	       &depth);
  cmap = DefaultColormap(display,DefaultScreen(display));
  image = XGetImage(display,win,0,0,width,height,AllPlanes,ZPixmap);

  /* allocate enough XColor structures for each pixel in a scanline */
  colorcell_defs = (XColor *)checkmalloc(sizeof(XColor)*width);

  /* allocate temporary buffer for scanline */
  buf = (unsigned char *)checkmalloc(sizeof(unsigned char)*3*width);

  /* set up all the required tags for RGB image */
  TIFFSetField(tif,TIFFTAG_SUBFILETYPE,0);
  TIFFSetField(tif,TIFFTAG_IMAGEWIDTH,width);
  TIFFSetField(tif,TIFFTAG_IMAGELENGTH,height);
  TIFFSetField(tif,TIFFTAG_BITSPERSAMPLE,8);
  TIFFSetField(tif,TIFFTAG_COMPRESSION,COMPRESSION_NONE);
  TIFFSetField(tif,TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_RGB);
  TIFFSetField(tif,TIFFTAG_ORIENTATION,ORIENTATION_TOPLEFT);
  TIFFSetField(tif,TIFFTAG_SAMPLESPERPIXEL,3,3,3); 
  TIFFSetField(tif,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG); 

  for (y=0;y<height;y++) { /* for each scanline */

    /* get pixel value for each pixel in scanline */
    for (x=0;x<width;x++) {
      pixel = XGetPixel(image,x,y);
      colorcell_defs[x].pixel = pixel;
    } 

    /* this sets the red, green, and blue values in          */ 
    /* colorcell_defs for each pixel value in colorcell_defs */
    XQueryColors(display,cmap,colorcell_defs,width);

    /* copy colorcell_defs rgb values into buf */
    for (i=0;i<width;i++) {
      buf[i*3] = (unsigned char)colorcell_defs[i].red;
      buf[(i*3)+1] = (unsigned char)colorcell_defs[i].green;
      buf[(i*3)+2] = (unsigned char)colorcell_defs[i].blue;
    }

    /* write the scanline from buf */
    TIFFWriteScanline(tif,(unsigned char *)buf,y,0);

  } /* end for */

  for (i=0;i<width;i++) {
    free(colorcell_defs[i]);
  }

  (void)XDestroyImage(image);
  free(buf);
  free(fpath);
  TIFFClose(tif);

  return 0; /* success */

} /* end X_save_tiff_image() */


/*
 * The following functions were copied from xtiff.c created by Chip 
 * Chapin, and modified by Scott Simon for the UNIX mdb version. The 
 * header of the original file is included below as noted.
 */

/**************************************************************************
* File:         xtiff.c
* RCS:          $Header: xtiff.c,v 1.5 90/10/19 11:24:32 chip Exp $
* Description:  Display a TIFF image in an X11 window
* Author:       Chip Chapin, HP/CLL, chip@hpcllcc.hp.com
* Created:      Tue Jul  4 18:45:47 1989
* Modified:     Thu Jun 28 16:35:56 1990 (Chip Chapin) chip@hpcllz2
* Language:     C
* Package:      TIFF utilities
* Status:       Experimental (Do Not Distribute)
****************************************************************************
*
* $Log:	xtiff.c,v $
* Revision 1.5  90/10/19  11:24:32  11:24:32  chip (Chip Chapin)
* Incorporate support for b&w monitors.
* 
* Eliminate the check for a color display.  Some folks want to display
* monochrome images on a monochrome display.  And why not?  Add check for
* R/W colormap before trying to create one (if Private colormap), and add
* pixmap packing kludge for monochrome.
*
* Added the CheckAndCorrectColormap() function, swiped from Dan and Chris
* Sears' version of xtiff.  This allows xtiff to display the many bogus files
* with colormap values of 0..255.   Here's what they say about it:
*
*   TIFFTAG_COLORMAP is often incorrectly written as ranging from
*   0 to 255 rather than from 0 to 65535.  CheckAndCorrectColormap()
*   takes care of this.
*
* Revision 1.4  90/01/27  08:22:35 salem (Jim Salem)
* (Note(cc): Jim Salem's version was released with the tiff library
* distribution, version 2.2, June 1990.)
*  Fixed Colormap handling.  It now conforms to the 5.0 spec by using the
*  16bit color values rather than just the lower 8 bits.  The maximum
*  srip value can now be 15 due to this change.  The stripping code now
*  is much slower (this could be fixed . . . in fact, alot could be fixed.)
*  Also, fixed calls to rgbi so that it is always passed 16 bit values.
*
* Revision 1.3  89/09/26  10:51:45  10:51:45  chip (Chip Chapin)
* Release 1.5 -- Fix silly bug: duplicated declarations from merging files
* caused 6-plane display failure (I'm surprised it worked at all).
* 
* Revision 1.2  89/09/20  14:30:58  14:30:58  chip (Chip Chapin)
* Release 1.4
* 
* Revision 1.1  89/09/20  12:52:26  12:52:26  chip (Chip Chapin)
* Initial revision
* 
*
* 890708 cc	Add -p (private colormap) option
* 890708 cc	Initial HP release, version 1.1
* 890710 cc	Print image size & tiff dir with # colors.
* 890718 cc	Use DefaultDepth instead of hard-wired 8-plane.
*		(thanks to Chris Christensen, John Silva, Walter Underwood)
* 890719 cc	Added wunder's color-by-color mod to stripping algorithm.
* 890719 cc	Make MAXCOLORS dynamic, instead of 256.
* 890720 cc	Fix order of TIFFClose/RemapColors.
* 890726 cc	Fix bug with expanded images in non-8-plane environment.
* 890726 cc	Release 1.2
* 890812 cc	Fix bug with BW tif's where width%8 != 0.
*		Also fix bug in photometric range mapping.
* 890812 cc	Release 1.3
* 890814 cc	Make errors in TIFFReadScanLine fatal.  Seems to corrupt heap.
*
*
* Derived from sgigt.c, as distributed with tifflib (4/89), by Sam Leffler
*		(sam@ucbvax.berkeley.edu), and
*         from xgif.c by John Bradley (University of Pennsylvania,
*		bradley@cis.upenn.edu), and
*	  from xgifload.c by John Bradley and Patrick Noughton.
*	  And now (6/90) also from xtiff.c by Dan and Chris Sears.
*
* The file sgigt.c bore no notice of any kind, but appears to originate with
* Leffler.  Some of the other files in his TIFF library distribution
* bear the following notice:
*
*	# TIFF Library Tools
*	#
*	# Copyright (c) 1988 by Sam Leffler.
*	# All rights reserved.
*	#
*	# This file is provided for unrestricted use provided that this
*	# legend is included on all tape media and as a part of the
*	# software program in whole or part.  Users may copy, modify or
*	# distribute this file at will.
*	#
*
* The file xgif.c bore no copyright notice, but acknowledged Bradley as author.
* The file xgifload.c bore the following notice:
*
* xgifload.c  -  based strongly on...
*
* gif2ras.c - Converts from a Compuserve GIF (tm) image to a Sun Raster image.
*
* Copyright (c) 1988, 1989 by Patrick J. Naughton
*
* Author: Patrick J. Naughton
* naughton@wind.sun.com
*
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose and without fee is hereby granted,
* provided that the above copyright notice appear in all copies and that
* both that copyright notice and this permission notice appear in
* supporting documentation.
*
* This file is provided AS IS with no warranties of any kind.  The author
* shall have no liability with respect to the infringement of copyrights,
* trade secrets or any patents by this file or any part thereof.  In no
* event will the author be liable for any lost revenue or profits or
* other special, indirect and consequential damages.
* <end of notice from xgifload.c>
*
* (whew!) This derivative program is subject to the same notices made by the
* authors of its precursors.
**************************************************************************/

#define CENTERX(f,x,str) ((x)-XTextWidth(f,str,strlen(str))/2)
#define CENTERY(f,y) ((y)-((f->ascent+f->descent)/2)+f->ascent)
#define MAXEXPAND 16
#define SCALE(x, s) (((x) * 65535L) / (s))
#define BITSPERBYTE 8

typedef unsigned char  byte;
typedef unsigned short RGBvalue;

/* X stuff */
static Display       *theDisp;
static int           theScreen, dispcells;
static Colormap      theCmap;
static GC            theGC;
static Visual        *theVisual;
static XImage        *theImage, *expImage;
static int  	      imageDepth; /* Depth (planes) of image in X window */
static int	      imagePad;	  /* 8, 16, 32.  Used as "bitmap_pad" */
  
static int            strip;
static byte	      *Image; /* raster image: assumes max depth 8 */

/* revised colormap/palette stuff */
static u_short	*redcmap, *greencmap, *bluecmap; /* colormap for palette images */
static u_short *Red, *Green, *Blue;	/* Derived color map.  Type should be same
				   as "redcmap", etc. above. */
static u_char	*Used;			/* True if a color has been used (only useful
				   with palette images) */
static int	HaveColorMap;		/* True if raw cmaps above are usable */
static int	NeedsMonoHack;		/* Monochrome image needs packing  */
static unsigned int  	numcols;	/* Number of colors used, or in colormap */
static unsigned int	MAXCOLORS;	/* # of colors possible on screen */

/* These all come from the original sgigt stuff */
static u_short	width, height;			/* image width & height */
static u_short	bitspersample;
static u_short	samplesperpixel;
static u_short	photometric;
static u_short	orientation;
static RGBvalue **BWmap;

/*************************************************************************
*
*N  rgbi
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     rgbi -- The original "sgigt", from which xtiff is derived,
*     calls the rgbi library routine from SGI.  This version works out OK, 
*     but is not a good substitute for doing a real color analysis in cases
*     where there are more than MAXCOLORS colors.  -- cc
*   
*     Takes R/G/B values (0..65535) and finds a matching colormap entry, or
*     allocates a new one up to MAXCOLORS.  If MAXCOLORS is exceeded, it
*     finds the nearest existing color and uses that.
*   
*     Note that these will all be remapped to X colormap entries in the end.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     r    <input>  == (RGBvalue) red value.
*     g    <input>  == (RGBvalue) green value.
*     b    <input>  == (RGBvalue) blue value.
*     rgbi <output> == (int) color entry in colormap.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992        UNIX mdb version, modified 
*					Chip Chapin's xtiff.c. 
*E
*************************************************************************/
static int rgbi(RGBvalue r,RGBvalue g,RGBvalue b)
{
    register int i;
    static int didWarning=0;
    int mdist, closest, d;
    
    for (i=0; i<numcols; i++)
	/* Look for RGB in our colormap */
	if (r == Red[i] && g == Green[i] && b == Blue[i]) return i;
    
    /* Didn't find a match */
    if (i < MAXCOLORS) {
	/* Inserting a new color */
	numcols++;
	Red[i] = r; Green[i] = g; Blue[i] = b;
	return i;
    } /* new color */
    
    /* No more room for new colors.  Find closest match */
    if (!didWarning) {
	fprintf(stderr,
"  Warning: Image contains more colors than your display supports (%d).\n",
MAXCOLORS);
	didWarning=1;
    }
    /* Look for closest match */
    mdist = 1000000; closest = -1;
    for (i=0; i<MAXCOLORS; i++) {
	d = abs(r - Red[i]) +
	    abs(g - Green[i]) +
	    abs(b - Blue[i]);
	if (d<mdist) { mdist=d; closest=i; }
    }
    return closest;

} /* end rgbi() */

#define	REPEAT8(op)	REPEAT4(op); REPEAT4(op)
#define	REPEAT4(op)	REPEAT2(op); REPEAT2(op)
#define	REPEAT2(op)	op; op

/*************************************************************************
*
*N  gtbw
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function uses the BWmap to copy the scanline into the raster 
*     buffer.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     bitspersample <input>  == (int) # of bits per sample in tiff image.
*     w             <input>  == (int) width of the tiff image.
*     cp	    <input>  == (byte) buffer for the raster image.
*     pp	    <input>  == (u_char) current scanline buffer.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992        UNIX mdb version, modified 
*					Chip Chapin's xtiff.c. 
*E
*************************************************************************/
static void gtbw(int bitspersample,int w,register byte *cp,register u_char *pp)
{
    register RGBvalue c, *bw;
    register int x;

    /* 890812 cc -- fix to correctly handle w%8 != 0 */
    switch (bitspersample) {
      case 1:
	for (x = w; x >= 8; x -= 8) {
	    bw = BWmap[*pp++];
	    REPEAT8(c = *bw++; *cp++ = rgbi(c, c, c));
	}
	break;
      case 2:
	for (x = w; x >= 4; x -= 4) {
	    bw = BWmap[*pp++];
	    REPEAT4(c = *bw++; *cp++ = rgbi(c, c, c));
	}
	break;
      case 4:
	for (x = w; x >= 2; x -= 2) {
	    bw = BWmap[*pp++];
	    REPEAT2(c = *bw++; *cp++ = rgbi(c, c, c));
	}
	break;
    }
    /* do remaining oddball pixels (incomplete byte) */
    if (x > 0) {	
	bw = BWmap[*pp];
	for (; x; x--) {
	    c = *bw++;
	    *cp++ = rgbi(c, c, c);
	}
    }
} /* end gtbw() */

/*************************************************************************
*
*N  setorientation
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function sets up the orientation of the tif image, by 
*     reading the ORIENTATION tag.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     tif            <input>  == (TIFF *) tiff image data structure.
*     h              <input>  == (int) height of the tiff image.
*     setorientation <output> == (int) first y position of image.
*				 0 = top orientation.
*				 (h - 1) = bottom orientation.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992        UNIX mdb version, modified 
*					xtiff.c by Chip Chapin. 
*E
*************************************************************************/
static int setorientation(TIFF *tif,int h)
{
    int y;
    
    if (!TIFFGetField(tif, TIFFTAG_ORIENTATION, &orientation))
	orientation = ORIENTATION_TOPLEFT;
    switch (orientation) {
      case ORIENTATION_BOTRIGHT:
      case ORIENTATION_RIGHTBOT: /* XXX */
      case ORIENTATION_LEFTBOT:	 /* XXX */
	orientation = ORIENTATION_BOTLEFT;
	/* fall thru... */
      case ORIENTATION_BOTLEFT:
	y = h-1;	/* 890706, cc. I reversed this. */
	break;
      case ORIENTATION_TOPRIGHT:
      case ORIENTATION_RIGHTTOP: /* XXX */
      case ORIENTATION_LEFTTOP:	 /* XXX */
	orientation = ORIENTATION_TOPLEFT;
	/* fall thru... */
      case ORIENTATION_TOPLEFT:
	y = 0;		/* 890706 see comment above */
	break;
    }
    return (y);
} /* end setorientation() */

/*************************************************************************
*
*N  gtcontig
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function reads each scanline of the tiff image and copies it
*     into the raster image. All sample values for each pixel are stored
*     contiguously, so there is only a single image plane.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     tif      <input>  == (TIFF *) tiff image data structure.
*     raster   <input>  == (byte *) memory area to write image into.
*     Map      <input>  == (RGBvalue *) range of sample values for tiff 
*					image.
*     h        <input>  == (int) height of the tiff image.
*     w        <input>  == (int) width of the tiff image.
*     gtcontig <output> == (int) status of gtcontig function
*			   0 = failure
*			   1 = success
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992        UNIX mdb version, modified 
*					xtiff.c by Chip Chapin. 
*E
*************************************************************************/
static int gtcontig(TIFF *tif,byte *raster,register RGBvalue *Map,int h,int w)
{
    register u_char *pp;
    register byte *cp;
    register int x;
    int row, y, e;
    u_char *buf;

    buf = (u_char *)checkmalloc((unsigned)TIFFScanlineSize(tif));
    if (buf == 0) {
	return 0;
    }
    y = setorientation(tif, h);
    e = 1;		/* default return value == no error */
    for (row = 0; row < h; row++) {
	if (TIFFReadScanline(tif, buf, row, 0) < 0) {
	    e = 0;	       /* return value == error */
	    break;
	}
	pp = buf;
	cp = raster + y*w;
	switch (photometric) {
	  case PHOTOMETRIC_RGB:
	    switch (bitspersample) {
	      case 8:
	      for (x = w; x-- > 0;) {
		*cp++ = rgbi(Map[pp[0]], Map[pp[1]], Map[pp[2]]);
		pp += samplesperpixel;
	      }
	      break;

    	      case 16: {
	        register u_short *wp;
	      
		if (Map) {
		      wp = (u_short *)pp;
		      for (x = w; x-- > 0;) {
			  wp[0] = Map[wp[0]];
			  wp[1] = Map[wp[1]];
			  wp[2] = Map[wp[2]];
			  wp += samplesperpixel;
		      }
		  }
		  wp = (u_short *)pp;
		  for (x = w; x-- > 0;) {
		      *cp++ = rgbi(wp[0], wp[1], wp[2]);
		      wp += samplesperpixel;
		  }
		  break;
	      }
	    }
	    break;
	  case PHOTOMETRIC_PALETTE:
	    if (HaveColorMap) {
		bcopy((char*)pp, (char*)cp, w);	/* copy scanline */
		for (x = w; x-- > 0;) Used[*pp++] = 1;
	    } else {
		/* If palette was bigger than MAXCOLORS, go here */
		for (x = w; x-- > 0;) {
		    RGBvalue c = *pp++;
		    *cp++ = rgbi(redcmap[c],
				 greencmap[c], bluecmap[c]);
		}
	    }
	    break;
	  case PHOTOMETRIC_MINISWHITE:
	  case PHOTOMETRIC_MINISBLACK:
	    if (bitspersample == 8) {
		register RGBvalue c;
		
		for (x = w; x-- > 0;) {
		    c = Map[*pp++];
		    *cp++ = rgbi(c, c, c);
		}
	    } else
		gtbw(bitspersample, w, cp, pp);
	    break;
	}
	y += (orientation == ORIENTATION_TOPLEFT ? 1 : -1);	/* 890706 cc */
    }
    free((char*)buf);
    return (e);
} /* end gtcontig() */

/*************************************************************************
*
*N  gtseparate
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function reads each scanline of the tiff image and copies it
*     into the raster image. All sample values for each pixel are stored
*     in separate "sample planes".
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     tif        <input>  == (TIFF *) tiff image data structure.
*     raster     <input>  == (byte *) memory area to write image into.
*     Map        <input>  == (RGBvalue *) range of sample values for 
*					  tiff image.
*     h          <input>  == (int) height of the tiff image.
*     w          <input>  == (int) width of the tiff image.
*     gtseparate <output> == (int) status of gtseparate function
*			     0 = failure
*			     1 = success
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992        UNIX mdb version, modified 
*					xtiff.c by Chip Chapin. 
*E
*************************************************************************/
static int gtseparate(TIFF *tif,byte *raster,register RGBvalue *Map,int h,int w)
{
    register byte *cp;
    register int x;
    u_char *red;
    int scanline, row, y, e;
    
    scanline = TIFFScanlineSize(tif);
    switch (samplesperpixel) {
      case 1:
	red = (u_char *)checkmalloc((unsigned)scanline);
	if (!red) {
	  return 0;
	}
	break;
      case 3: case 4:
	red = (u_char *)checkmalloc((unsigned)3*scanline);
	if (!red) {
	  return 0;
	}
	break;
    }
    y = setorientation(tif, h);
    e = 1;		/* default return value == no error */
    for (row = 0; row < h; row++) {
	cp = raster + y*w;
	if (TIFFReadScanline(tif, red, row, 0) < 0) {
	    e = 0;	/* return error */
  	    break;
  	}
	switch (photometric) {
	  case PHOTOMETRIC_RGB: {
	      register u_char *r, *g, *b;
	      
	      r = red;
	      if (TIFFReadScanline(tif, g = r + scanline, row, 1) < 0) {
		  e = 0;	/* return error */
  		  break;
  	      }
	      if (TIFFReadScanline(tif, b = g + scanline, row, 2) < 0) {
		  e = 0;	/* return error */
  		  break;
  	      }
	      switch (bitspersample) {
		case 8:
		  for (x = 0; x < w; x++)
		      *cp++ = rgbi(Map[*r++], Map[*g++], Map[*b++]);
		  break;
		case 16:
		  for (x = 0; x < w; x++)
#define	wp(x)	((u_short *)(x))
		      *cp++ = rgbi(wp(r)[x],wp(g)[x],wp(b)[x]);
		  break;
#undef	wp
	      }
	      break;
	  }
	  case PHOTOMETRIC_PALETTE: {
	      register u_char *pp = red;
	      if (HaveColorMap) {
		  bcopy((char*)pp, (char*)cp, w);	/* copy scanline */
		  for (x = w; x-- > 0;) Used[*pp++] = 1;
	      } else {
		  /* If palette was bigger than MAXCOLORS, go here */
		  for (x = w; x-- > 0;) {
		      RGBvalue c = *pp++;
		      *cp++ = rgbi(redcmap[c],
				   greencmap[c], bluecmap[c]);
		  }
	      }
	      break;
	  }
	  case PHOTOMETRIC_MINISWHITE:
	  case PHOTOMETRIC_MINISBLACK:
	    if (bitspersample == 8) {
		register u_short *pp = (u_short *)red;
		register RGBvalue c;
		
		for (x = w; x-- > 0;) {
		    c = Map[*pp++];
		    *cp++ = rgbi(c, c, c);
		}
	    } else
		gtbw(bitspersample, w, cp, red);
	    break;
	}
	y += (orientation == ORIENTATION_TOPLEFT ? 1 : -1);
    }
    if (red)
	free((char*)red);
    return (e);
} /* end gtseparate() */

/*************************************************************************
*
*N  makebwmap
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     Greyscale images with less than 8 bits/sample are handled
*     with a table to avoid lots of shifts and masks.  The table
*     is setup so that gtbw (below) can retrieve 8/bitspersample
*     pixel values simply by indexing into the table with one
*     number.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     Map       <input>  == (RGBvalue *) range of sample values for tiff 
*					 image.
*     makebwmap <output> == (int) status of makebwmap function
*			    0 = failure
*			    1 = success
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992        UNIX mdb version, modified 
*					xtiff.c by Chip Chapin. 
*E
*************************************************************************/
static int makebwmap(RGBvalue *Map)
{
    register int i;
    int nsamples = 8 / bitspersample;
    register RGBvalue *p;
    
    BWmap = (RGBvalue **)checkmalloc(256*sizeof (RGBvalue *)+(256*nsamples*sizeof(RGBvalue)));
    if (BWmap == NULL) {
	return (0);
    }
    p = (RGBvalue *)(BWmap + 256);
    for (i = 0; i < 256; i++) {
	BWmap[i] = p;
	switch (bitspersample) {
	  case 1:
	    *p++ = Map[i>>7];
	    *p++ = Map[(i>>6)&1];
	    *p++ = Map[(i>>5)&1];
	    *p++ = Map[(i>>4)&1];
	    *p++ = Map[(i>>3)&1];
	    *p++ = Map[(i>>2)&1];
	    *p++ = Map[(i>>1)&1];
	    *p++ = Map[i&1];
	    break;
	  case 2:
	    *p++ = Map[i>>6];
	    *p++ = Map[(i>>4)&3];
	    *p++ = Map[(i>>2)&3];
	    *p++ = Map[i&3];
	    break;
	  case 4:
	    *p++ = Map[i>>4];
	    *p++ = Map[i&0xf];
	    break;
	}
    }
    return (1);
} /* end makebwmap() */

/*************************************************************************
*
*N  gt
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function retrieves the photometricinterpretation and 
*     planarconfiguration tags of the tiff image. It then calls either
*     gtcontig or gtseparate to perform the actual copying of the image.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     tif    <input> == (TIFF *) tiff image data structure.
*     w      <input> == (int) width of the tiff image.
*     h      <input> == (int) height of the tiff image.
*     raster <input> == (byte *) memory area to write image into.
*     gt     <output> == (int) status of gt function
*			 0 = failure
*			 1 = success 
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992        UNIX mdb version, modified 
*					xtiff.c by Chip Chapin. 
*E
*************************************************************************/
static int gt(TIFF *tif,int w,int h,byte *raster)
{
    u_short minsamplevalue, maxsamplevalue, planarconfig;
    RGBvalue *Map;
    int e;
    
    if (!TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric)) {
	switch (samplesperpixel) {
	  case 1:
	    photometric = PHOTOMETRIC_MINISBLACK;
	    break;
	  case 3: case 4:
	    photometric = PHOTOMETRIC_RGB;
	    break;
	  default:
	    fprintf(stderr, "Missing needed \"%s\" tag for tiff image.\n",
		    "PhotometricInterpretation");
	    return (0);
	}
	printf("No \"PhotometricInterpretation\" tag, assuming %s.\n",
	       photometric == PHOTOMETRIC_RGB ? "RGB" : "min-is-black");
    }

    if (!TIFFGetField(tif, TIFFTAG_MINSAMPLEVALUE, &minsamplevalue))
	minsamplevalue = 0;
    if (!TIFFGetField(tif, TIFFTAG_MAXSAMPLEVALUE, &maxsamplevalue))
	maxsamplevalue = (1<<bitspersample)-1;
    Map = NULL;
    switch (photometric) {
      case PHOTOMETRIC_RGB:
	if (minsamplevalue == 0 && maxsamplevalue == 65535)
	    /* Does not require any mapping */
	    break;
	/* fall thru... */
      case PHOTOMETRIC_MINISBLACK:
      case PHOTOMETRIC_MINISWHITE: {
	  register int x, range;

	  /* map each sample value onto 0..65535 */
	  /* 890812 cc -- fix added to get correct values */
	  range = maxsamplevalue - minsamplevalue;
	  Map = (RGBvalue *)checkmalloc((range+1) * sizeof (RGBvalue));
	  if (Map == NULL) {
	      return (0);
	  }
	  if (photometric == PHOTOMETRIC_MINISWHITE) {
	      for (x = 0; x <= range; x++)
		  Map[x] = ((range - x) * 65535) / range;
	  } else {
	      for (x = 0; x <= range; x++)
		  Map[x] = (x * 65535) / range;
	  }
	  if (bitspersample != 8 && photometric != PHOTOMETRIC_RGB) {
	      if (!makebwmap(Map)) {
		  free((char *)Map);
		  return (0);
	      }
	      /* no longer need Map, free it */
	      free((char *)Map);
	      Map = NULL;
	  }
	  break;
      }
      case PHOTOMETRIC_PALETTE:
	if (!TIFFGetField(tif, TIFFTAG_COLORMAP,
			  &redcmap, &greencmap, &bluecmap)) {
	    fprintf(stderr, "Missing required \"Colormap\" tag.\n");
	    return (0);
	}
	/* 890706 cc -- We can probably just use their colormap, and
	   save a lot of time. */
	if (bitspersample == 8 && samplesperpixel == 1 && MAXCOLORS >= 256) {
	    /* We're home free.  Somebody's already done all the work */
	    HaveColorMap = 1;
	    numcols = 256;
	}
	break;
    }
    
    if (HaveColorMap == 0) {
	/* Allocate space for derived colormap */
	Red   = (u_short*)checkmalloc(MAXCOLORS*sizeof(u_short));
	Green = (u_short*)checkmalloc(MAXCOLORS*sizeof(u_short));
	Blue  = (u_short*)checkmalloc(MAXCOLORS*sizeof(u_short));
	if (!(Red && Green && Blue)) {
	    return 0;
	}
    }

    TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarconfig);
    if (planarconfig == PLANARCONFIG_SEPARATE)
	e = gtseparate(tif, raster, Map, h, w);
    else
	e = gtcontig(tif, raster, Map, h, w);
    if (Map)
	free((char *)Map);
    if (BWmap)
	free((char *)BWmap);
    return (e);
} /* end gt() */

/*************************************************************************
*
*N  CheckAndCorrectColormap
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     Many programs get TIFF colormaps wrong.  They use 8-bit colormaps 
*     instead of 16-bit colormaps.  This function is a heuristic to 
*     detect and correct this.
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
*    Scott Simon	Oct 1992        UNIX mdb version, modified 
*					xtiff.c by Chip Chapin. 
*E
*************************************************************************/
static void CheckAndCorrectColormap()
{
    register int i;

    for (i = 0; i < numcols; i++)
        if ((redcmap[i] > 255) || (greencmap[i] > 255) || (bluecmap[i] > 255))
            return;

    for (i = 0; i < numcols; i++) {
       redcmap[i] = SCALE(redcmap[i], 255);
       greencmap[i] = SCALE(greencmap[i], 255);
       bluecmap[i] = SCALE(bluecmap[i], 255);
    }

    return;
} /* end CheckAndCorrectColormap() */


/*************************************************************************
*
*N  RemapColors
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function allocates the colors needed for the picture.
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
*    Scott Simon	Oct 1992        UNIX mdb version, modified 
*					xtiff.c by Chip Chapin. 
*E
*************************************************************************/
static void RemapColors()
{
    register int   i,j;
    int		   numused;
    static short   lmasks[16]    = {0xffff, 0xfffe, 0xfffc, 0xfff8,
				    0xfff0, 0xffe0, 0xffc0, 0xff80,
				    0xff00, 0xfe00, 0xfc00, 0xf800,
				    0xf000, 0xe000, 0xc000, 0x8000};
    static short   roundvals[16] = {0x0000, 0x0000, 0x0001, 0x0002,
				    0x0004, 0x0008, 0x0010, 0x0020,
				    0x0040, 0x0080, 0x0100, 0x0200,
			    0x0400, 0x0800, 0x1000, 0x2000};
    short          rlmask, glmask, blmask;
    short          rroundval,groundval,broundval;
    byte           *ptr;
    u_char	   *Uptr;	/* Traipse through Used array */
    u_long	   *cols;	/* Array of pixel values.  Must be u_long */
    XColor  	   *defs;	/* Array of color definitions */

    cols = (u_long*)calloc(MAXCOLORS, sizeof(u_long));
    defs = (XColor*)calloc(MAXCOLORS, sizeof(XColor));
    if (!(cols && defs)) {
      if (cols != NULL) free(cols);
      if (defs != NULL) free(defs);
      fprintf(stderr,"RemapColors: Not enough memory\n");
      return;
    }

    if (HaveColorMap) {
	/*
	 * Use the colormap provided with the image.
	 * But first, use Chris and Dan Sears' heuristic to see if it is a
	 * bogus 8-bit colormap.
	 */
	CheckAndCorrectColormap();
	Red = redcmap; Green = greencmap; Blue = bluecmap;
	/* It may have unused entries, however.  I wonder how many? */
	for (numused=0,Uptr=Used+MAXCOLORS; Uptr != Used;)
	    if (*--Uptr) numused++;
    } else {
	/* All colors are used */
	memset((char*)Used, 1, MAXCOLORS*sizeof(u_char));
	numused = numcols;
    }

    /* Allocate the X colors for this picture */
   j = 0;
   while (strip<16) {
	int color;		/* 0:Blue, 1:Green, 2:Red */
	int laststrip;
	/* Walter Underwood's revised color stripping scheme ... */
	/* We strip one color at a time, in an attempt to do the least
	   amount of damage.  It is not a great color quantization
	   algorithm, but it gives decent results on a 6-plane display. */
	for (color=0 ; color<=2 ; color++ ) {
	    laststrip = (strip==0) ? strip : strip-1;
	    switch(color) {
	      case 0:		/* strip Blue */
		rlmask = lmasks[laststrip];
		rroundval = roundvals[laststrip];
		glmask = lmasks[laststrip];
		groundval = roundvals[laststrip];
		blmask = lmasks[strip];
		broundval = roundvals[strip];
		break;
	      case 1:		/* strip Green and Blue */
		rlmask = lmasks[laststrip];
		rroundval = roundvals[laststrip];
		glmask = lmasks[strip];
		groundval = roundvals[strip];
		blmask = lmasks[strip];
		broundval = roundvals[strip];
		break;
	      case 2:		/* strip Red, Green, and Blue */
		rlmask = lmasks[strip];
		rroundval = roundvals[strip];
		glmask = lmasks[strip];
		groundval = roundvals[strip];
		blmask = lmasks[strip];
		broundval = roundvals[strip];
		break;
	    }
	    for (i=0; i<numcols; i++)
		if (Used[i]) {
		    defs[i].red   = ((Red[i]  &rlmask)+rroundval);
		    defs[i].green = ((Green[i]&glmask)+groundval);
		    defs[i].blue  = ((Blue[i] &blmask)+broundval);
		    defs[i].flags = DoRed | DoGreen | DoBlue;
		    if (!XAllocColor(theDisp,theCmap,&defs[i])) break;
		    cols[i] = defs[i].pixel;
		}
	    
	    if (i<numcols) {		/* failed */
		if (color==2) {
		    /* tried all colors, strip another bit */
		    strip++;
		    j++;
		}
		for (i--; i>=0; i--)
		    if (Used[i])
			XFreeColors(theDisp,theCmap,cols+i,1,0L);
	    }
	    else goto done;
	} /* for */
    } /* while */
    
	  done:
/*
	    if (j && strip<16)
	      fprintf(stderr,"X_read_tiff_image:  %s stripped %d bits\n",fname,strip);
*/
	    if (strip==16) {
/*	      fprintf(stderr,"UTTERLY failed to allocate the desired colors.\n"); */
              for (i=0; i<numcols; i++) cols[i]=i;
	    }


    
	/* Now remap all the colors in Image to the X colormap entries */
	ptr = Image;
	for (i=0; i<height; i++)
	    for (j=0; j<width; j++,ptr++) 
		*ptr = (byte) cols[*ptr];

      if (cols != NULL) free(cols);
      if (defs != NULL) free(defs);

} /* end RemapColors() */

/*************************************************************************
*
*N  DrawWindow
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function actually copies the Image into the pixmap.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     x   <input> == (int) left most position of the image to be copied.
*     y   <input> == (int) upper most position of the image to be copied.
*     w   <input> == (int) width of the image to be copied.
*     h   <input> == (int) height of the image to be copied.
*     pix <input> == (Pixmap) pixmap to be copied.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992        UNIX mdb version, modified 
*					xtiff.c by Chip Chapin. 
*E
*************************************************************************/
static void DrawWindow(int x,int y,int w,int h,Pixmap pix)
{
    byte *p1,*pend;
    unsigned int pkd, *p2, *pkdata;
    int line, i, bpw, pkwidth;

    if (NeedsMonoHack) {
	/* Fix up the Pixmap before we try to draw it.
	 * Since bits_per_pixel is one, X is going to assume that the
	 * Pixmap is bit-packed, but it's not!  Each pixel occupies a byte.
	 * So pack each scan line.
	 */
	
	bpw = sizeof(int)*BITSPERBYTE;		/* Bits per Word */
	
	/* Width (in WORDS) of a packed scan line */
	pkwidth = expImage->width/bpw; 
	if (expImage->width%bpw)
	    pkwidth++;

	/* New image data buffer.  Can't just clobber the unpacked buffer,
	 * since it could be the [unexpanded] original image, which will be
	 * needed if we attempt to resize.
	 */
	pkdata = (unsigned int *)checkmalloc(pkwidth*sizeof(int)*expImage->height);
	if (!pkdata) return;

	for (line= expImage->height-1; line>=0; line--) {
	    /* For each scan line */
	    p1= (unsigned char *)(expImage->data +(line*expImage->width));
	    p2 = pkdata +(line*pkwidth);
	    pend = p1+expImage->width;
	    while (p1<pend) {
		/* Pack the scan line a word at a time */
		pkd=0;
		for (i=bpw-1; i>=0; i--) {
		    /* This assumes that consecutive pixels start from
		     * the MSB of an integer, and run to the LSB.
		     */
		    pkd |= (*p1++ << i);
		} /* pixel */
		*p2++ = pkd;
	    } /* word */
	} /* scan line */
	
	if (expImage != theImage)
	    /* This is expanded data, not the original, so we should
	     * just throw it away.  The original data can still be found
	     * in the "Image" buffer.
	     */
	    free(expImage->data);
	expImage->data = (char *)pkdata;
	expImage->bitmap_pad = sizeof(int)*BITSPERBYTE;
	expImage->bytes_per_line = pkwidth*sizeof(int);
	NeedsMonoHack = 0;
    } else { /* if NeedsMonoHack */
      expImage = theImage;
    }
    XPutImage(theDisp,pix,theGC,expImage,x,y,x,y,w,h);
} /* end DrawWindow() */

/*************************************************************************
*
*N  X_read_tiff_image
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function reads a TIFF image from the specified file and displays 
*     it in the specified X window.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     filename <input>  == (char *) tiff image to read and display.
*     display  <input>  == (Display *) X display.
*     win      <input>  == (Window *) X Window.
*     gc       <input>  == (GC) X gc.
*     X_read_tiff_image <output> == (int) status of X_read_tiff_image 
*				    function.
*				    0 = successful    
*				    1 = file not found
*				    2 = access violation
*				    3 = invalid TIFF file
*				    4 = bad X object
*				    5 = general error
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*    Scott Simon	Oct 1992        UNIX mdb version, modified 
*					Chip Chapin's xtiff.c:main(). 
*E
*************************************************************************/
int X_read_tiff_image( char *filename, Display *display,
                       Pixmap pix, GC gc )
{
    int i;
    char *cp;
    TIFF *tif;
    char *geom,*fname;
    XEvent event;

    theDisp = display;
    theGC = gc;
    fname = (char *)checkmalloc(strlen(filename));
    strcpy(fname,filename);

    geom = NULL;
    redcmap = greencmap = bluecmap = NULL;
    Red     = Green     = Blue     = NULL;
    BWmap = NULL;
    expImage = NULL;
    strip = 0; numcols = 0;
    HaveColorMap = 0;			/* Default, no colormap from file */

    if (fileaccess(fname,0)) {
      free(fname);
      return 1;
    }

    if (fileaccess(fname,4)) {
      free(fname);
      return 2;
    }

    tif = TIFFOpen(fname, "r");
    if (tif == NULL) {
      free(fname);
      return 3; /* Invalid tiff file */
    }

    if (pix == NULL)  {
      free(fname);
      return 4; /* Invalid X object */
    }

    if (!TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample))
	bitspersample = 1;

    switch (bitspersample) {
      case 1: case 2: case 4:
      case 8: case 16:
	break;
      default:
	fprintf(stderr, "X_read_tiff_image: Sorry, can't handle %d-bit pictures\n",
		bitspersample);
        free(fname);
	return 3; /* Invalid tiff file */
    }
    if (!TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel))
	samplesperpixel = 1;
    switch (samplesperpixel) {
      case 1: case 3: case 4:
	break;
      default:
	fprintf(stderr, "X_read_tiff_image: Sorry, can't handle %d-channel images\n",
		samplesperpixel);
        free(fname);
	return 3; /* Invalid tiff file */
    }
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    
    cp = rindex(fname, '/');
    if (cp == NULL)
	cp = fname;
    else
	cp++;
    fname=cp;

    theScreen = DefaultScreen(theDisp);

    /* The whole business of selecting a visual needs to be revamped.
       For example, the default visual may not offer as much depth as the
       hardware is capable of supporting.  We should really use XGetVisualInfo
       and look for the best visual we can use.  Meantime...
     */
    theVisual = DefaultVisual(theDisp,theScreen);
    dispcells = DisplayCells(theDisp, theScreen); /* colormap size */
    MAXCOLORS = dispcells;		
    imageDepth =DefaultDepth(theDisp,theScreen);  /* Bits per pixel */
    imagePad = 8;

    NeedsMonoHack = (imageDepth == 1);

    theCmap   = DefaultColormap(theDisp, theScreen);
    
    /* Allocate the X Image */
    Image = (byte *)checkmalloc(width*height*sizeof(byte));
    if (!Image) {
      free(fname);
      return 5; /* general error */
    }
    
    theImage = XCreateImage(theDisp,theVisual,(unsigned int)imageDepth,
			    ZPixmap,0,(char *)Image, width,height,
			    imagePad,0);
    if (!theImage) {
      fprintf(stderr,"X_read_tiff_image: Unable to create XImage\n");
      free(fname);
      return 5; /* general error */
    }

    Used = (u_char*)calloc(MAXCOLORS, sizeof(u_char));
    if (!Used) {
      fprintf(stderr,"X_read_tiff_image: Not enough memory\n");
      free(fname);
      return;
    }

    if (gt(tif, width, height, Image)) {
	RemapColors();
	TIFFClose(tif);  /* AFTER RemapColors, please.  Otherwise this can
			    deallocate our palette. */
        DrawWindow(0,0,width,height,pix);
    } else {
      free(fname);
      return 5; /* general error */
    }
    free(fname);
    return 0;
} /* end X_read_tiff_image() */

