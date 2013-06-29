/*
 * Sean James
 *
 * main.h
 *
 * Raytracer main method
 *
 */

#ifndef _MAIN_H
#define _MAIN_H

#include "defs.h"

#include "camera.h"
#include "bitmap.h"
#include "raytracer.h"
#include "parseobj.h"
#include "kdtree.h"
#include "glimagedisplay.h"
#include "thread.h"

#include "sphereshape.h"
#include "polygonshape.h"
#include "aaplaneshape.h"
#include "aacubeshape.h"
#include "planeshape.h"

#include "directionallight.h"
#include "pointlight.h"

#include "surface.h"
#include "phongsurface.h"

/*
 * Main method
 */
int main(int argc, char *argv[]);

#endif