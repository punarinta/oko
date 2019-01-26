#include <stdio.h>
#include <stdlib.h>
#include "sprite.h"

sprite_t *sprite_Load(char *filename)
{
    sprite_t *sprite = malloc(sizeof(sprite_t));
    FILE *fp = fopen(filename, "rb");

    if (!fp)
    {
        return NULL;
    }

    fread(&sprite->width, 4, 1, fp);
    fread(&sprite->height, 4, 1, fp);

    sprite->colors = malloc(2 * sprite->width * sprite->height);
    sprite->glyphs = malloc(2 * sprite->width * sprite->height);

    fread(sprite->colors, 2, sprite->width * sprite->height, fp);
    fread(sprite->glyphs, 2, sprite->width * sprite->height, fp);

    fclose(fp);

    return sprite;
}

short sprite_SampleGlyph(sprite_t *sprite, float x, float y)
{
    int sx = (int) (x * (float) sprite->width);
    int sy = (int) (y * (float) sprite->height - 1.0);
    if (sx < 0 || sx >= sprite->width || sy < 0 || sy >= sprite->height)
    {
        return ' ';
    }
    else
    {
        return sprite->glyphs[sy * sprite->width + sx];
    }
}

short sprite_SampleColor(sprite_t *sprite, float x, float y)
{
    int sx = (int) (x * (float) sprite->width);
    int sy = (int) (y * (float) sprite->height-1.0f);
    if (sx < 0 || sx >= sprite->width || sy < 0 || sy >= sprite->height)
    {
        return 0; // FG_BLACK;
    }
    else
    {
        return sprite->colors[sy * sprite->width + sx];
    }
}

void sprite_Destroy(sprite_t *sprite)
{
    if (sprite->glyphs) free(sprite->glyphs);
    if (sprite->colors) free(sprite->colors);
    if (sprite) free(sprite);
}
