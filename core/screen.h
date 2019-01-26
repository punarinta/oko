#ifndef _OKO_SCREEN_H_
#define _OKO_SCREEN_H_ 1

#include <ncurses.h>

typedef struct _pixel_t_
{
    wchar_t c;
    int cp;
} pixel_t;

typedef struct _screen_t_
{
    int width;
    int height;
    pixel_t *buffer;
    WINDOW *win;
} screen_t;

screen_t *screen_Init(void);
void screen_Destroy(screen_t *);
void screen_Draw(screen_t *, unsigned int x, unsigned int y, wchar_t c, int colorPair);
void screen_Flush(screen_t *);

int mvaddnwstr(int, int, const wchar_t *, int);

#endif
