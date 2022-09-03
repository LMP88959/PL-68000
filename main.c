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

#include "glib.h"

#include <stdio.h>

/* cc68 -O -o prog.bin main.c glib.c gfx.c imode.c math.c pl.c */
static init();
static int update();
static display();

static int pal[6] = {
  0000000,
  0172321,
  0440802, /* 8 and 9 were allowed in octal literals, they became 010 and 011 */
  0656353,
  0233124,
  0777775,
};

/* dist from cam */
#define DFC 400

main()
{
    extern vdch, vdcv;
    extern long dblbuf;
    long p, c, n;
    short fc = 0;
    auto dt, k = 1;

    puts("initializing");
    if (!init88(VDCM128_8)) {
        return;
    }
    
    setpal(pal, 6);
    swap();
    
    pinit((char *) dblbuf, vdch, vdcv);

    init();
    clrkb();
    p = clockms();
    n = clockms() + 1000;
    while (k) {
        c  = clockms();
        dt = c - p;
        if (dt < 40) {
            continue;
        }
        p = c;
        k = update();
        display();
        fc++;

        if (clockms() >= n) {
            putstr("FPS: ");
            printd(fc);
            n  =+ 1000;
            fc = 0;
        }
    }
    exit(0);
}

char *
umemgt(n, esz)
unsigned n, esz;
{
    return(alloc(n * esz));
}

umemfr(p)
char *p;
{
    free(p);
}

uerror(id, modnm, msg)
char *modnm, *msg;
{
    printx(id);
    puts(modnm);
    puts(msg);
    kill88();
    exit(0);
}

bufcpy(to, from, size)
register char *to, *from;
register unsigned size;
{
    while (size--) {
        *to++ = *from++;
    }
}

bufset(to, val, size)
register char *to;
register char val;
register unsigned size;
{
    while (size--) {
       *to++ = val;
    }
}

#define CUSZ    128 /* cube size */

static struct PL_OBJ *tcube;
static char chk[PTDIM * PTDIM];
static rot  1;
static decv 0;

static
maketex()
{
    auto i, j, t;
    int c;
    
    t = 0x10;

    for (i = 0; i < PTDIM; i++) {
        for (j = 0; j < PTDIM; j++) {
            if ((i < t) || (j < t) ||
               (i > ((PTDIM - 1) - t)) ||
               (j > ((PTDIM - 1) - t))) {
                /* border color */
                c = pal[1];
            } else {
                /* checkered pattern */
                c = (((i & t)) ^ ((j & t))) ? pal[1] : pal[3];
            }
            if ((i == j) || abs(i - j) < 3) {
                /* thick red line along diagonal */
                chk[i + j * PTDIM] = pal[2];
            } else {
                chk[i + j * PTDIM] = c;
            }
        }
    }
    
    for (i = 0; i < (PTDIM * PTDIM); i++) {
        chk[i] =& 07;
    }
}

static
init()
{
    maketex();

    imtex(chk);
    tcube = genbox(CUSZ, CUSZ, CUSZ, PALL, 
            pal[5] >> 12 & 077, 
            pal[5] >>  6 & 077, 
            pal[5] >>  0 & 077);
	
	cullm = PCBACK;
	rastm = PRTEX;
}

static int
update()
{
    switch (getch()) {
        case 033: return(0);
        case 040: rot =^ 1; break;
        case 'a': rastm = PRTEX; break;
        case 's': rastm = PRFLAT; break;
    }
    
    if (rot) { decv =- 4; }

	return(1);
}

static
display()
{
    extern long dblbuf;
    
    pvideo = (char *) dblbuf;

	pc();

    midt();
    mtrans(0, 0, DFC);
    mrotx(decv >> 2);
    mroty(decv >> 1);
    odraw(tcube);

    sync();

	swap();
}
