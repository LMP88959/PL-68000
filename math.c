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

/*  math.c
 * 
 * Integer-only math using fixed point numbers.
 * 
 */

char PL_sin[PTRIGMAX] = {
	0x00,0x03,0x06,0x09,0x0c,0x0f,0x12,0x15,
	0x18,0x1c,0x1f,0x22,0x25,0x28,0x2b,0x2e,
	0x30,0x33,0x36,0x39,0x3c,0x3f,0x41,0x44,
	0x47,0x49,0x4c,0x4e,0x51,0x53,0x55,0x58,
	0x5a,0x5c,0x5e,0x60,0x62,0x64,0x66,0x68,
	0x6a,0x6c,0x6d,0x6f,0x70,0x72,0x73,0x75,
	0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7c,
	0x7d,0x7e,0x7e,0x7f,0x7f,0x7f,0x7f,0x7f,
	0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7e,0x7e,
	0x7d,0x7c,0x7c,0x7b,0x7a,0x79,0x78,0x77,
	0x76,0x75,0x73,0x72,0x70,0x6f,0x6d,0x6c,
	0x6a,0x68,0x66,0x64,0x62,0x60,0x5e,0x5c,
	0x5a,0x58,0x55,0x53,0x51,0x4e,0x4c,0x49,
	0x47,0x44,0x41,0x3f,0x3c,0x39,0x36,0x33,
	0x30,0x2e,0x2b,0x28,0x25,0x22,0x1f,0x1c,
	0x18,0x15,0x12,0x0f,0x0c,0x09,0x06,0x03
};

char PL_cos[PTRIGMAX];

static struct CF {
        int ctx, cty, ctz;
    int crx, cry, crz;
} xfc; /* current transform */

pminit()
{
    register i;
  
    /* sine is mirrored over X after PI */
    for (i = 0; i < (PTRIGMAX >> 1); i++) {
        PL_sin[(PTRIGMAX >> 1) + i] = -PL_sin[i];
    }
    /* construct cosine table by copying sine table */
    for (i = 0; i < ((PTRIGMAX >> 1) + (PTRIGMAX >> 2)); i++) {
        PL_cos[i] = PL_sin[i + (PTRIGMAX >> 2)];
    }
    for (i = 0; i < (PTRIGMAX >> 2); i++) {
        PL_cos[i + ((PTRIGMAX >> 1) + (PTRIGMAX >> 2))] = PL_sin[i];
    }
}

midt()
{	
    xfc.ctx = 0;
    xfc.cty = 0;
    xfc.ctz = 0;
    xfc.crx = 0;
    xfc.cry = 0;
    xfc.crz = 0;
}

mtrans(x,y,z) { xfc.ctx =+ x; xfc.cty =+ y; xfc.ctz =+ z; }
mrotx(rx) { xfc.crx =+ rx; }
mroty(ry) { xfc.cry =+ ry; }
mrotz(rz) { xfc.crz =+ rz; }

xfvecs(v, o, len)
register short *v, *o;
register short len;
{
    register short xx, yy, zz, ww;
    short tsx, tsy, tsz;
    short tcx, tcy, tcz;

    tcx = PL_cos[xfc.crx & PTRIGMSK];
    tsx = PL_sin[xfc.crx & PTRIGMSK];
    tcy = PL_cos[xfc.cry & PTRIGMSK];
    tsy = PL_sin[xfc.cry & PTRIGMSK];
    tcz = PL_cos[xfc.crz & PTRIGMSK];
    tsz = PL_sin[xfc.crz & PTRIGMSK];

    while ((len--) > 0) {
        xx = v[0];
        yy = v[1];
        zz = v[2];
        
        ww = (yy * tsz + xx * tcz) >> PP;
        yy = (yy * tcz - xx * tsz) >> PP;
        xx = ww;
        
        ww = (zz * tsy + xx * tcy) >> PP;
        zz = (zz * tcy - xx * tsy) >> PP;
        xx = ww;
        
        ww = (yy * tcx - zz * tsx) >> PP;
        zz = (yy * tsx + zz * tcx) >> PP;
        yy = ww;
        
        o[0] = xx + xfc.ctx;
        o[1] = yy + xfc.cty;
        o[2] = zz + xfc.ctz;
        v =+ PVLEN;
        o =+ PVLEN;
    }
}

int
porder(a, b, c)
register short *a, *b, *c;
{
    return (short)(b[0] - a[0]) * (short)(c[1] - a[1]) <
           (short)(b[1] - a[1]) * (short)(c[0] - a[0]);
}
