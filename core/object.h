#ifndef _OKO_OBJECT_H_
#define _OKO_OBJECT_H_ 1

#define MAX_OBJECTS 256

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "sprite.h"

typedef struct _object_t_
{
    bool on;
    float x;
    float y;
    float vx;
    float vy;
    sprite_t *sprite;
} object_t;

void object_Set(object_t *, float, float, float, float, sprite_t *);
object_t *object_InitArray(unsigned int);
void object_Destroy(object_t *);

#endif
