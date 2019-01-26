#include "object.h"

void object_Set(object_t *object, float x, float y, float vx, float vy, sprite_t *sprite)
{
    object->on = true;
    object->x = x;
    object->y = y;
    object->vx = vx;
    object->vy = vy;
    object->sprite = sprite;
}

object_t *object_InitArray(unsigned int size)
{
    object_t *objects;
    objects = malloc(sizeof(object_t) * size);
    memset(objects, 0, sizeof(object_t) * size);

    return objects;
}

void object_Destroy(object_t *object)
{
    // do not destroy sprites!
    if (object) free(object);
}
