#ifndef _OKO_GAME_H_
#define _OKO_GAME_H_ 1

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TIMESTEP 16000

#include "core/screen.h"
#include "core/colors.h"
#include "core/sprite.h"
#include "core/map.h"
#include "core/object.h"

screen_t *screen;

object_t *gameObjects = NULL;
unsigned int gameObjectsCount = 0;

void setTimer(void);
void setSignals(void);
void handler(int signum);

#endif
