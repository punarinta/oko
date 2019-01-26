#ifndef _OKO_SPRITE_H_
#define _OKO_SPRITE_H_ 1

typedef struct _sprite_t_
{
	int width;
    int height;
    short *glyphs;
    short *colors;
} sprite_t;

sprite_t *sprite_Load(char *);
short sprite_SampleGlyph(sprite_t *, float, float);
short sprite_SampleColor(sprite_t *, float, float);
void sprite_Destroy(sprite_t *);

#endif
