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

/*  imode.c
 * 
 * Simple immediate mode geometry interface.
 * 
 */

static struct PL_OBJ p; /* product */
static struct PL_OBJ wc;

static short vtx[POMAXV];             /* temp storage for vertices */
static struct PL_POLY ply[POMAXV / 4]; /* temp storage for polygons */

static ptype PTRIS;
static nv 0;     /* entered so far */
static np 0;     /* entered so far */
static char *curtex NULL;
static vnum 0;
static tnum 0;
static cr 077;
static cg 077;
static cb 077;
static cu 0;
static cv 0;

static int
addvtx(x, y, z)
{
	register i, v;

	for (i = 0; i < nv; i++) {
	    v = i * PVLEN;
        if ((vtx[v    ] == x) &&
            (vtx[v + 1] == y) &&
            (vtx[v + 2] == z)) {
            return(i);
        }
	}
	v = nv * PVLEN;
    vtx[v    ] = x;
    vtx[v + 1] = y;
    vtx[v + 2] = z;
	return(nv++);
}

imbeg()
{
    extern umemfr();
    register i;
    
    if (p.c) {
        umemfr(p.c);
    }
    p.c  = NULL;
    p.nc = 0;

    if (p.p) {
        for (i = 0; i < p.np; i++) {
            p.p[i].color = 0;
            p.p[i].nv    = 0;
            p.p[i].tex   = NULL;
        }
        umemfr(p.p);
    }
    p.p  = NULL;
    p.np = 0;

	nv = 0;
	np = 0;
	vnum = 0;
	tnum = 0;
}

imtype(type)
{
	/* reset when primitive type is changed */
	if (type != ptype) {
		vnum = 0;
		tnum = 0;
	}
	ptype = type;
}

imtex(tex)
char *tex;
{
	curtex = tex;
}

imcolr(r, g, b) { cr = r; cg = g; cb = b; }
imtexc(u, v) { cu = u; cv = v; }

imvtx(x, y, z)
{
    extern bufset();
    struct PL_POLY *t;
    register i, b, edges;
    static cvx[4];
    static ctc[2 * 4];
    
	cvx[vnum++] = addvtx(x, y, z);
	ctc[tnum++] = cu;
	ctc[tnum++] = cv;
	switch (ptype) {
		case PTRIS:
			if (vnum == 3) { goto addp; }
			break;
		case PQUADS:
			if (vnum == 4) { goto addp; }
			break;
		default:
			vnum = 0;
			tnum = 0;
			break;
	}
	return;
addp:
    vnum = 0;
    tnum = 0;
    t = &ply[np];
    bufset(t, 0, sizeof(struct PL_POLY));
    t->tex = curtex;
    edges = 3;
    
    if (ptype == PQUADS) {
        edges = 4;
        /* check for a quad with two identical vertices,
         * if it has any, turn it into a triangle
         */
        if (cvx[0] == cvx[1]) {
            edges = 3;
            cvx[1        ] = cvx[2        ];
            ctc[2 * 1    ] = ctc[2 * 2    ];
            ctc[2 * 1 + 1] = ctc[2 * 2 + 1];
            
            cvx[2        ] = cvx[3        ];
            ctc[2 * 2    ] = ctc[2 * 3    ];
            ctc[2 * 2 + 1] = ctc[2 * 3 + 1];
        }
        if (cvx[2] == cvx[3]) {
            edges = 3;
        }
    }
    
    t->color = cb & 07;
    
    for (i = 0; i < edges; i++) {
        b = i * PPOLY_VLEN;
        t->v[b    ] = cvx[i        ] * PVLEN;
        t->v[b + 1] = ctc[2 * i    ] << TP;
        t->v[b + 2] = ctc[2 * i + 1] << TP;
    }
    b = i * PPOLY_VLEN;
    t->v[b    ] = cvx[0] * PVLEN;
    t->v[b + 1] = ctc[0] << TP;
    t->v[b + 2] = ctc[1] << TP;
    t->nv = edges;
    
    np++;

}

imend()
{
    extern char *umemgt();
    extern bufcpy(), uerror();
    register i;

	if ((nv == 0) || (np == 0)) {
	    return;
	}
    
    if (p.c) {
        uerror(PERR_MISC, "imode", "end without beg v");
        return;
    }
    p.c = umemgt(nv * PVLEN, sizeof(short));
    if (p.c == NULL) { goto nomem; }
    for (i = 0; i < (nv * PVLEN); i++) {
        p.c[i] = vtx[i];
    }
    p.nc = nv;
    if (p.p) {
        uerror(PERR_MISC, "imode", "end without beg p");
        return;
    }
    p.p = umemgt(np + 1, sizeof(struct PL_POLY));
    if (p.p == NULL) { goto nomem; }
    for (i = 0; i < np; i++) {
        bufcpy(&p.p[i], &ply[i], sizeof(struct PL_POLY));
    }
    p.np = np;
    return;
nomem:
    uerror(PERR_NO_MEM, "imode", "no memory");  
}

iminit()
{
    extern ocpy();
    
	if (nv && np) {
	    ocpy(&wc, &p);
	}
}

imdraw()
{
    extern odraw();
    
	if (nv && np) {
	    odraw(&wc);
	}
}

imexport(dst)
struct PL_OBJ *dst;
{
    extern char *umemgt();
    extern bufcpy(), uerror();
    register i;
	
	dst->c = umemgt(p.nc * PVLEN, sizeof(short));
	if (dst->c == NULL) { goto nomem; }
	for (i = 0; i < (p.nc * PVLEN); i++) {
		dst->c[i] = p.c[i];
	}
	dst->nc = p.nc;
	dst->p = umemgt(p.np, sizeof(struct PL_POLY));
	if (dst->p == NULL) { goto nomem; }
	for (i = 0; i < np; i++) {
		bufcpy(&dst->p[i], &p.p[i], sizeof(struct PL_POLY));
	}
	dst->np = p.np;
	return;
nomem:
    uerror(PERR_NO_MEM, "imode", "no memory");	
}
