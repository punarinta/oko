#include <locale.h>
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
#define M_PI           3.14159265358979323846
#endif

#define TIMESTEP 16000

void setTimer(void);
void setSignals(void);
void getGeometry(int *rows, int *cols);
void handler(int signum);

wchar_t *pBuffer;
wchar_t *pBufferSky;
bool isRendering = false;

WINDOW *mainWin;

int nScreenWidth = 120;
int nScreenHeight = 40;
int nMapWidth = 16;
int nMapHeight = 16;

float fPlayerX = 8.0;
float fPlayerY = 8.0;
float fPlayerA = 0.0;
float fFOV = 3.14159 / 4.0;
float fDepth = 16.0 * 1.41;
float fSpeed = 0.1;
unsigned short frameCount = 0;
unsigned short frameDrop = 0;

char *map = "\
################\
#..............#\
#..............#\
#....#.........#\
#..............#\
#..............#\
#..............#\
#..............#\
#..............#\
#..............#\
#..............#\
#..............#\
#..............#\
#..............#\
#..............#\
## # # # # # ###\
";

typedef struct _pair
{
  float first;
  float second;
} pair;

void offsetMove(float xTest, float yTest)
{
    xTest += fPlayerX;
    yTest += fPlayerY;

    if (map[(int) xTest * nMapWidth + (int) yTest] != '#')
    {
        fPlayerX = xTest;
        fPlayerY = yTest;
    }
}

int main(void)
{
    setlocale(LC_ALL, "");
    getGeometry(&nScreenWidth, &nScreenHeight);

    if ((mainWin = initscr()) == NULL)
    {
        fprintf(stderr, "Renderer initialization error.\n");
        exit(-1);
    }

    start_color();

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_CYAN);

    noecho();
    keypad(mainWin, true);

    srand((unsigned) time(NULL));
    setTimer();
    setSignals();

    // init env
    curs_set(0);
    refresh();


    pBuffer = (wchar_t *) malloc(nScreenWidth * nScreenHeight * sizeof(wchar_t));
    pBufferSky = (wchar_t *) malloc(nScreenWidth/* * nScreenHeight*/ * sizeof(wchar_t));

    memset(pBufferSky, '.', sizeof(wchar_t) * nScreenWidth);

    float xTest = 0, yTest = 0;

    while (1)
    {
        switch (getch())
        {
            case KEY_UP:
            case 'w':
                offsetMove(sinf(fPlayerA) * fSpeed, cosf(fPlayerA) * fSpeed);
                break;

            case KEY_DOWN:
            case 's':
                offsetMove(-sinf(fPlayerA) * fSpeed, -cosf(fPlayerA) * fSpeed);
                break;

            case 'a':
                offsetMove(sinf(fPlayerA - M_PI / 2) * fSpeed, cosf(fPlayerA - M_PI / 2) * fSpeed);
                break;

            case 'd':
                offsetMove(sinf(fPlayerA + M_PI / 2) * fSpeed, cosf(fPlayerA + M_PI / 2) * fSpeed);
                break;

            case KEY_LEFT:
                fPlayerA -= 0.05;
                break;

            case KEY_RIGHT:
                fPlayerA += 0.05;
                break;
        }
    }

    return 0;
}

int pairComparator(pair *a, pair *b)
{
    return a->first < b->first;
}

void render()
{
    if (isRendering)
    {
        ++frameDrop;
        return;
    }

    if (frameCount++ > 60)
    {
        frameCount = 1;
        frameDrop = 0;
    }

    isRendering = true;

    for (int x = 0; x < nScreenWidth; x++)
    {
        // For each column, calculate the projected ray angle into world space
        float fRayAngle = (fPlayerA - fFOV / 2.0) + ((float)x / (float)nScreenWidth) * fFOV;

        // Find distance to wall
        float fStepSize = 0.05;          // Increment size for ray casting, decrease to increase
        float fDistanceToWall = 0.0; //              resolution

        bool bHitWall = false;        // Set when ray hits wall block
        bool bBoundary = false;        // Set when ray hits boundary between two wall blocks

        float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
        float fEyeY = cosf(fRayAngle);

        // Incrementally cast ray from player, along ray angle, testing for
        // intersection with a block
        while (!bHitWall && fDistanceToWall < fDepth)
        {
            fDistanceToWall += fStepSize;
            int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
            int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

            // Test if ray is out of bounds
            if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
            {
                bHitWall = true;            // Just set distance to maximum depth
                fDistanceToWall = fDepth;
            }
            else
            {
                // Ray is inbound so test to see if the ray cell is a wall block
                if (map[nTestX * nMapWidth + nTestY] == '#')
                {
                    // Ray has hit wall
                    bHitWall = true;

					/*pair pairs[4];

					// Test each corner of hit tile, storing the distance from
					// the player, and the calculated dot product of the two rays
					for (int tx = 0; tx < 2; tx++)
					{
						for (int ty = 0; ty < 2; ty++)
						{
							// Angle of corner to eye
							float vy = (float) nTestY + ty - fPlayerY;
							float vx = (float) nTestX + tx - fPlayerX;
							float d = sqrt(vx * vx + vy * vy);
							float dot = fEyeX * vx / d + fEyeY * vy / d;
							pair p = {d, dot};
							pairs[tx * 2 + ty] = p;
						}
					}

					// Sort Pairs from closest to furthest
					qsort(pairs, 4, sizeof(pair), pairComparator);

					// First two/three are closest (we will never see all four)
					float fBound = 0.003;
					if (acos(pairs[0].second) < fBound) bBoundary = true;
					if (acos(pairs[1].second) < fBound) bBoundary = true;
                    if (acos(pairs[2].second) < fBound) bBoundary = true;*/
                }
            }
        }

        // Calculate distance to ceiling and floor
        float nCeiling = (float) (nScreenHeight / 2.0) - nScreenHeight / ((float) fDistanceToWall);
        float nFloor = nScreenHeight - nCeiling;

        // Shader walls based on distance
        wchar_t nShade = ' ';

        if (fDistanceToWall <= fDepth / 4.0)             nShade = 0x2588;    // Very close
        else if (fDistanceToWall < fDepth / 3.0)         nShade = 0x2593;
        else if (fDistanceToWall < fDepth / 2.0)         nShade = 0x2592;
        else if (fDistanceToWall < fDepth)               nShade = 0x2591;
        else nShade = ' ';

        if (bBoundary)        nShade = 0x2502;             // Black it out

        for (int y = 0; y < nScreenHeight; y++)
        {
            // Each Row
            if (y <= nCeiling)
            {
                pBuffer[y * nScreenWidth + x] = ' ';
            }
            else if (y > nCeiling && y <= nFloor)
            {
                //if (fabs(y - nCeiling) < 0.75) pBuffer[y * nScreenWidth + x] = 0x2584;
                /*else*/ pBuffer[y * nScreenWidth + x] = nShade;
            }
            else // Floor
            {
                // Shade floor based on distance
                float xPers = 0.3 * asinf(fabs(nScreenWidth / 2 - x) / nScreenWidth);
                float walkEffect = ( ((int) ((fPlayerX + fPlayerY + fPlayerA * 50) * 10)) % 2) * 0.003;
                float b = 1.0 - (((float) y - nScreenHeight / 2.0) / ((float) nScreenHeight / 2.0)) + xPers - walkEffect;

                if (b < 0.05)           nShade = 0x2588;
                else if (b < 0.2)       nShade = 0x2587;
                else if (b < 0.35)      nShade = 0x2586;
                else if (b < 0.5)       nShade = 0x2585;
                else if (b < 0.7)       nShade = 0x2584;
                else if (b < 0.85)      nShade = 0x2583;
                else if (b < 1.0)       nShade = 0x2582;
                else if (b < 1.15)      nShade = 0x2581;
                else                    nShade = ' ';

                pBuffer[y * nScreenWidth + x] = nShade;
            }
        }
    }

    attron(COLOR_PAIR(1));
    mvaddnwstr(0, 0, pBuffer, nScreenWidth * nScreenHeight);

    attron(COLOR_PAIR(2));
    for (int y = 0; y < nScreenHeight / 2; y++)
    {
        int skyStartX = -1;
        for (int x = 0; x < nScreenWidth; x++)
        {
            if (skyStartX == -1 && pBuffer[y * nScreenWidth + x] == ' ')
            {
                skyStartX = x;
                continue;
            }
            if (skyStartX != -1 && pBuffer[y * nScreenWidth + x] != ' ')
            {
                mvaddnwstr(y, skyStartX, pBufferSky, x - skyStartX);
                skyStartX = -1;
            }
        }

        if (skyStartX != -1)
        {
            mvaddnwstr(y, skyStartX, pBufferSky, nScreenWidth - skyStartX);
        }
    }

    mvprintw(0, 0, "FPS = %d", 60 - frameDrop);

    refresh();

    isRendering = false;
}

wchar_t *compositor(wchar_t *low, wchar_t *high)
{
    for (unsigned int i = 0; i < nScreenWidth * nScreenHeight; i++)
    {

    }
}

void getGeometry(int *width, int *height)
{
    struct winsize ws;

    if (ioctl(0, TIOCGWINSZ, &ws) < 0)
    {
        perror("couldn't get window size");
        exit(EXIT_FAILURE);
    }

    *height = ws.ws_row;
    *width = ws.ws_col;
}

void setSignals(void)
{
    struct sigaction sa;

    // fill in sigaction struct
    sa.sa_handler = handler;
    sa.sa_flags   = 0;
    sigemptyset(&sa.sa_mask);

    // set signal handlers
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    // ignore SIGTSTP
    sa.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &sa, NULL);
}

void setTimer(void)
{
    struct itimerval it = {0};

    // set timer
    it.it_interval.tv_usec = TIMESTEP;
    it.it_value.tv_usec    = TIMESTEP;
    setitimer(ITIMER_REAL, &it, NULL);
}

void handler(int signum)
{
    switch (signum)
    {
        case SIGALRM:
            render();
            return;

        case SIGTERM:
        case SIGINT:
            delwin(mainWin);
            endwin();
            refresh();
            free(pBuffer);
            free(pBufferSky);
            exit(EXIT_SUCCESS);
    }
}
