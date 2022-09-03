/*****************************************************************************/
/*
 * Early C implementation of a texture mapped software renderer that
 * runs in real time on a base Motorola 68000 processor.
 * 
 *   by EMMIR 2018-2022
 *   
 *   YouTube: https://www.youtube.com/c/LMP88
 *   
 * This software is released into the public domain.
 */
/*****************************************************************************/

/*  pl.h
 * 
 * Main header file for the PL library.
 * 
 */

/* maximum possible horizontal or vertical resolution */
#define PMAXDIM 256

/*****************************************************************************/
/********************************** ENGINE ***********************************/
/*****************************************************************************/

/* maximum number of vertices in object */
#define POMAXV  128

#define PRFLAT  1
#define PRTEX   0

#define PCNONE  0
#define PCFRNT  1
#define PCBACK  2

/* for storage size definition */
#define PVDIM       5  /* X Y Z U V */
#define PPOLY_VLEN  3  /* Idx U V */

#define VFOV    7

extern rastm; /* PRFLAT or PRTEX */
extern cullm; /* cull mode */

struct PL_POLY {
    char *tex;
    
    /* a user defined polygon may only have 3 or 4 vertices.  */
    
    /* [index, U, V] array of indices into obj verts array */
    short v[5 * PPOLY_VLEN]; 
    char color;
    char nv;
};

struct PL_OBJ {
    struct PL_POLY *p; /* list of polygons in the object */
    short *c;  /* coords: array of [x, y, z, 0] values */
    short  np; /* num polys */
    short  nc; /* num coords */
};

extern odraw(); /* draw object */
extern odel(); /* delete object */
extern ocpy(); /* copy object */

/*****************************************************************************/
/*********************************** IMODE ***********************************/
/*****************************************************************************/

#define PTRIS   0
#define PQUADS  1

extern imbeg(); /* begin primitive */
/* type is PTRIS or PQUADS */
extern imtype();

/* applies to the next polygon made. */
extern imtex(); /* texture */
/* last color defined before the poly is finished is used as the poly's color */
extern imcolr(); /* color */
extern imtexc(); /* texture coordinate */
extern imvtx(); /* vertex */

/* doesn't delete the previous object once called */
extern imend(); /* end primitive */

extern iminit(); /* initialize (only needed if not exporting) */
extern imdraw(); /* render (only needed if not exporting) */

/* save current model that has been defined in immediate mode */
extern imexport();

/*****************************************************************************/
/********************************* GRAPHICS **********************************/
/*****************************************************************************/

/* textures must be square with a dimension of PTDIM */
#define PTLOG   7
#define PTDIM   (1 << PTLOG)
#define TP      7

#define PPMAXV  4  /* max verts in a polygon */

#define PSFLAT  3  /* X Y Z */
#define PSTEX   5  /* X Y Z U V */

extern short hres;       /* horizontal resolution */
extern short vres;       /* vertical resolution */

extern char *pvideo;

/* Call this to initialize PL
 * 
 * video - pointer to target image
 * hres - horizontal resolution of image
 * vres - vertical resolution of image
 * 
 */
extern pinit();

/* clear entire screen color and depth */
extern pc();

/* Solid color polygon fill.
 * Expecting input stream of 3 values [X,Y,Z] */
extern pfpoly();

/* Affine (linear) texture mapped polygon fill.
 * Expecting input stream of 5 values [X,Y,Z,U,V] */
extern ptpoly();

/*****************************************************************************/
/*********************************** MATH ************************************/
/*****************************************************************************/

/* number of elements in PL_sin and PL_cos */
#define PTRIGMAX       256
#define PTRIGMSK       (PTRIGMAX - 1)

/* number of elements in a vector */
#define PVLEN          4

/* precision for fixed point math */
#define PP             7
#define PONE           (1 << PP)

extern char PL_sin[PTRIGMAX];
extern char PL_cos[PTRIGMAX];

/* vectors are assumed to be integer arrays of length PL_VLEN */
/* matrices are assumed to be integer arrays of length 16 */

extern int porder();

extern midt(); /* load identity transform */
extern mtrans(); /* translate by x, y, z */
extern mrotx(); /* rotate x */
extern mroty(); /* y */
extern mrotz(); /* z */

/* transform a stream of vertices by the current model+view */
extern xfvecs();

/*****************************************************************************/
/************************************ GEN ************************************/
/*****************************************************************************/

/* flags to specify the faces of the box to generate */
#define PTOP       001
#define PBOTTOM    002
#define PBACK      004
#define PFRONT     010
#define PLEFT      020
#define PRIGHT     040
#define PALL       077

/* generate a box */
extern struct PL_OBJ *genbox();

/*****************************************************************************/
/******************************* USER DEFINED ********************************/
/*****************************************************************************/

#define NULL    0

#define PERR_NO_MEM   0
#define PERR_MISC     1
/* error function (PL expects program to halt after calling this) */
extern uerror();

/* user memory allocation function, ideally a calloc or something similar */
extern char *umemgt();
/* user memory freeing function */
extern umemfr();

extern bufcpy(); /* (*to, *from, size) */
extern bufset(); /* (*to, val, size) */

