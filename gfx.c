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

#include "pl.h"

/*  gfx.c
 * 
 * Scans and rasterizes polygons.
 * 
 */

short hres 0, vres 0;
short hhres 0, hvres 0;
char *pvideo NULL;
static short scrsz 0;

#define TXMSK  ((1 << (PTLOG + TP)) - 1)
#define ATTR   4
#define ATTRB  2
#define SP     7               /* scan conversion precision */
#define SPRND  (1 << (SP - 1))  /* rounding */
#define DIVP   15

static short scan_miny, scan_maxy;

static short x_L[PMAXDIM];
static short x_R[PMAXDIM];
static short abuf[ATTR * PMAXDIM]; /* attribute buffer */

static short idiv[PMAXDIM];
static int mtab[PMAXDIM];
short invz[1024];

pinit(video, h, v)
char *video;
{
    extern pminit();
    register i;
    
	hres   = h;
	vres   = v;
	hhres  = hres >> 1;
	hvres  = vres >> 1;
	scrsz  = (hres * vres) >> (2 + 6);
	pvideo = video;
	
    idiv[0] = 1;
    mtab[0] = 0;
	for (i = 1; i < PMAXDIM; i++) {
	    idiv[i] = (1 << DIVP) / i;
	    mtab[i] = i * hres;
        x_L[i] = 077777;
        x_R[i] = -077777;
	}
	invz[0] = 1;
    /* we can use shorts for this super special 
     * case since the cube is not close enough to
     * access overflowed elements of the table
     */
	for (i = 1; i < 1024; i++) {
	    invz[i] = (1 << (VFOV + 12)) / i;
	}
	pminit();
}

pc()
{    
    register int *s;
    register short n;
    /* unrolled since simply clearing the screen takes an eternity */
    s = pvideo;
    n = scrsz;
    while (n--) {
        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;
        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;

        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;
        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;

        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;
        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;

        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;
        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;
        
        
        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;
        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;

        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;
        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;

        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;
        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;

        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;
        *s++ = 0; *s++ = 0; *s++ = 0; *s++ = 0;
    }
}

static int
pscanf(vB, len)
register short *vB;
char len;
{
    register short *vA;
    register short x, y, dx, dy;
    register short sx, sy;
    register short mjr, ady;
    
    scan_miny =  077777;
    scan_maxy = -077777;

    while (len--) {
        vA = vB;
        vB =+ (PSFLAT-1); /* no more Z */
        x  = vA[0];
        y  = vA[1];
        dx = vB[0] - x;
        dy = vB[1] - y;
        if (y < scan_miny) { scan_miny = y; }
        if (y > scan_maxy) { scan_maxy = y; }
        if (vB[1] < scan_miny) { scan_miny = vB[1]; }
        if (vB[1] > scan_maxy) { scan_maxy = vB[1]; }
        mjr = dx;
        ady = dy;
        if (dx < 0) { mjr = -dx; }
        if (dy < 0) { ady = -dy; }
        if (ady > mjr) {
            mjr = ady;
        }
        if (mjr <= 0) {
            continue;
        }
        ady = idiv[mjr];
        x  = (x  << SP) + SPRND;
        y  = (y  << SP) + SPRND;
        dx = (dx << SP) * ady >> DIVP;
        dy = (dy << SP) * ady >> DIVP;
        do {
            sx = x >> SP;
            sy = y >> SP;
            if (x_L[sy] > sx) {
                x_L[sy] = sx;
            }
            if (x_R[sy] < sx) {
                x_R[sy] = sx;
            }
            x =+ dx;
            y =+ dy;
        } while (mjr--);
    }
    return(scan_miny >= scan_maxy);
}

pfpoly(stream, len, rgb)
short *stream;
register short len;
register char rgb;
{
    register short miny, maxy, rm;
    register char *vbuf;
    
    if (pscanf(stream, len)) { return; }
    miny = scan_miny;
    maxy = scan_maxy - miny + 1;
    while (maxy--) {
        len  = x_L[miny];
        vbuf = pvideo + mtab[miny] + len;
        len  = x_R[miny] - len + 1;
        x_L[miny] =  077777;
        x_R[miny] = -077777;
        rm = len & 7;
        len =>> 3;
        while (len--) {
            *vbuf++ = rgb;
            *vbuf++ = rgb;
            *vbuf++ = rgb;
            *vbuf++ = rgb;
            
            *vbuf++ = rgb;
            *vbuf++ = rgb;
            *vbuf++ = rgb;
            *vbuf++ = rgb;
        }
        while (rm--) {
            *vbuf++ = rgb;
        }
        miny++;
    }
}

static int
pscantt(vB, len)
register short *vB;
char len;
{
    register short *vA;
    register short x, y, dx, dy;
    register short sx, sy;
    register short mjr, ady;
    register short AT0, DT0;
    register short AT1, DT1;
    
    scan_miny =  077777;
    scan_maxy = -077777;

    while (len--) {
        vA = vB;
        vB =+ (PSTEX-1); /* no more Z */
        x  = vA[0];
        y  = vA[1];
        dx = vB[0] - x;
        dy = vB[1] - y;
        if (y < scan_miny) { scan_miny = y; }
        if (y > scan_maxy) { scan_maxy = y; }
        if (vB[1] < scan_miny) { scan_miny = vB[1]; }
        if (vB[1] > scan_maxy) { scan_maxy = vB[1]; }
        mjr = dx;
        ady = dy;
        if (dx < 0) { mjr = -dx; }
        if (dy < 0) { ady = -dy; }
        if (ady > mjr) {
            mjr = ady;
        }
        if (mjr <= 0) {
            continue;
        }
        ady = idiv[mjr];
        AT0 = vA[2];
        DT0 = (vB[2] - vA[2]) * ady >> DIVP;
        AT1 = vA[3];
        DT1 = (vB[3] - vA[3]) * ady >> DIVP;
        x  = (x  << SP) + SPRND;
        y  = (y  << SP) + SPRND;
        dx = (dx << SP) * ady >> DIVP;
        dy = (dy << SP) * ady >> DIVP;
        do {
            sx = x >> SP;
            sy = y >> SP;
            if (x_L[sy] > sx) {
                x_L[sy] = sx;
                abuf[0 + (sy << ATTRB)] = AT0;
                abuf[2 + (sy << ATTRB)] = AT1;
            }
            if (x_R[sy] < sx) {
                x_R[sy] = sx;
                abuf[1 + (sy << ATTRB)] = AT0;
                abuf[3 + (sy << ATTRB)] = AT1;
            }
            x =+ dx;
            y =+ dy;
            AT0 =+ DT0;
            AT1 =+ DT1;
        } while (mjr--);
    }
    return(scan_miny >= scan_maxy);
}

ptpoly(stream, len, texels)
short *stream;
register short len;
register char *texels;
{
    register short du, dv;
    register short su, sv;
    short miny, maxy;
    register char *vbuf;
    short yt, il, rm;
    
    if (pscantt(stream, len)) { return; }
    miny = scan_miny;
    maxy = scan_maxy - miny + 1;
    while (maxy--) {
        len  = x_L[miny];
        vbuf = pvideo + mtab[miny] + len;
        len  = x_R[miny] - len + 1;
        x_L[miny] =  077777;
        x_R[miny] = -077777;
        il = idiv[len];
        yt = (miny << ATTRB);
        su = abuf[yt + 0];
        du = (short)(abuf[yt + 1] - su) * il >> DIVP;
        sv = abuf[yt + 2];
        dv = (short)(abuf[yt + 3] - sv) * il >> DIVP;
        rm = len & 3;
        len =>> 2;
        /* 4x unrolled */
        while (len--) {
            *vbuf++ = texels[(su >> TP) | ((sv >> TP) << PTLOG)];
            su =+ du;
            sv =+ dv;
            su =& TXMSK;
            sv =& TXMSK;
            *vbuf++ = texels[(su >> TP) | ((sv >> TP) << PTLOG)];
            su =+ du;
            sv =+ dv;
            su =& TXMSK;
            sv =& TXMSK;
            *vbuf++ = texels[(su >> TP) | ((sv >> TP) << PTLOG)];
            su =+ du;
            sv =+ dv;
            su =& TXMSK;
            sv =& TXMSK;
            *vbuf++ = texels[(su >> TP) | ((sv >> TP) << PTLOG)];
            su =+ du;
            sv =+ dv;
            su =& TXMSK;
            sv =& TXMSK;
        }
        
        while (rm--) {
            *vbuf++ = texels[(su >> TP) | ((sv >> TP) << PTLOG)];
            su =+ du;
            sv =+ dv;
            su =& TXMSK;
            sv =& TXMSK;
        }
        
        miny++;
    }
}
