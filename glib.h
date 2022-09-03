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

#include <stdio.h>

#define VDCMLOW_8 0
#define VDCMLOWTC 1
#define VDCM128_8 2
#define VDCM128TC 3
#define VDCM256_8 4
#define VDCM256TC 5
#define VDCM320_8 6
#define VDCM320TC 7
#define VDCM480_8 8
#define VDCM512_8 9
#define VDCM640_8 10
#define NUMMODES  11

int init88();
kill88();

/* set palette, swap front and back buffers, sync (wait for vertical retrace */
setpal(), swap(), sync();

long clockms();

int getch(), kbhit();
clrkb();
