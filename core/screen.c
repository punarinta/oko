#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <locale.h>
#include <string.h>

#include "screen.h"

screen_t *screen_Init()
{
    struct winsize ws;
    screen_t *screen = malloc(sizeof(screen_t));
    memset(screen, 0, sizeof(screen_t));

    setlocale(LC_ALL, "");
    if (ioctl(0, TIOCGWINSZ, &ws) < 0)
    {
        perror("Could not get screen size");
        screen_Destroy(screen);
        exit(EXIT_FAILURE);
    }

    if ((screen->win = initscr()) == NULL)
    {
        fprintf(stderr, "NCurses initialization error");
        screen_Destroy(screen);
        exit(EXIT_FAILURE);
    }

    screen->width = ws.ws_col;
    screen->height = ws.ws_row;
    screen->buffer = malloc(sizeof(pixel_t) * ws.ws_col * ws.ws_row);

    // hide cursor
    curs_set(0);

    // enable color control
    start_color();

    // set up keyboard input
    noecho();
    keypad(screen->win, true);

    // flush ncurses
    refresh();

    return screen;
}

void screen_Destroy(screen_t *screen)
{
    if (screen)
    {
        if (screen->buffer) free(screen->buffer);
        delwin(screen->win);
        free(screen);
    }

    endwin();
    refresh();
}

void screen_Draw(screen_t *screen, unsigned int x, unsigned int y, wchar_t c, int colorPair)
{
	if (x >= 0 && x < screen->width && y >= 0 && y < screen->height)
	{
		screen->buffer[y * screen->width + x].c = c;
		screen->buffer[y * screen->width + x].cp = colorPair;
    }
}

void screen_Flush(screen_t *screen)
{
    wchar_t wc[1] = {0};
    for (int y = 0; y < screen->height; y++) for (int x = 0; x < screen->width; x++)
    {
        wc[0] = screen->buffer[y * screen->width + x].c;
        attron(COLOR_PAIR(screen->buffer[y * screen->width + x].cp));
        mvaddnwstr(y, x, wc, 1);
    }
}
