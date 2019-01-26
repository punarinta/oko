#include "game.h"

bool isRendering = false;

float fPlayerX = 8.0;
float fPlayerY = 8.0;
float fPlayerA = 0.0;
float fFOV = 3.14159 / 4.0;
float fDepth = 16.0 * 1.41;
float fSpeed = 0.1;

unsigned short frameCount = 0;
unsigned short frameDrop = 0;

sprite_t *spriteWall;

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
    screen = screen_Init();
    colors_Init();

    srand((unsigned) time(NULL));
    setTimer();
    setSignals();

    spriteWall = sprite_Load("data/sprites/wall1.spr");

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

    for (int x = 0; x < screen->width; x++)
    {
        // For each column, calculate the projected ray angle into world space
        float fRayAngle = (fPlayerA - fFOV / 2.0) + ((float)x / (float)screen->width) * fFOV;

        // Find distance to wall
        float fStepSize = 0.02;          // Increment size for ray casting, decrease to increase
        float fDistanceToWall = 0.0; //              resolution

        bool bHitWall = false;        // Set when ray hits wall block
        bool bBoundary = false;        // Set when ray hits boundary between two wall blocks

        float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
        float fEyeY = cosf(fRayAngle);

        float fSampleX = 0.0;
        bool bLit = false;

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

                    // Determine where ray has hit wall. Break Block boundary
                    // int 4 line segments
                    float fBlockMidX = (float) nTestX + 0.5;
                    float fBlockMidY = (float) nTestY + 0.5;

                    float fTestPointX = fPlayerX + fEyeX * fDistanceToWall;
                    float fTestPointY = fPlayerY + fEyeY * fDistanceToWall;

                    float fTestAngle = atan2f(fTestPointY - fBlockMidY, fTestPointX - fBlockMidX);

                    if (fTestAngle >= -M_PI * 0.25 && fTestAngle < M_PI * 0.25)  fSampleX = fTestPointY - (float) nTestY;
                    if (fTestAngle >= M_PI * 0.25 && fTestAngle < M_PI * 0.75)   fSampleX = fTestPointX - (float) nTestX;
                    if (fTestAngle < -M_PI * 0.25 && fTestAngle >= -M_PI * 0.75) fSampleX = fTestPointX - (float) nTestX;
                    if (fTestAngle >= M_PI * 0.75 || fTestAngle < -M_PI * 0.75)  fSampleX = fTestPointY - (float) nTestY;
                }
            }
        }

        // Calculate distance to ceiling and floor
        float nCeiling = (float) (screen->height / 2.0) - screen->height / ((float) fDistanceToWall);
        float nFloor = screen->height - nCeiling;

        // fDepthBuffer[x] = fDistanceToWall;

        for (int y = 0; y < screen->height; y++)
        {
            // Each Row
            if (y <= nCeiling)
            {
                screen_Draw(screen, x, y, PIXEL_SOLID, 117);
            }
            else if (y > nCeiling && y <= nFloor)
            {
                // Draw Wall
                if (fDistanceToWall < fDepth)
                {
                    float fSampleY = ((float) y - (float) nCeiling) / ((float) nFloor - (float) nCeiling);
                    screen_Draw(screen, x, y, sprite_SampleGlyph(spriteWall, fSampleX, fSampleY), sprite_SampleColor(spriteWall, fSampleX, fSampleY));
                }
                else
                    screen_Draw(screen, x, y, PIXEL_SOLID, 117);
            }
            else // Floor
            {
                // Shade floor based on distance
                float xPers = 0.3 * asinf(fabs(screen->width / 2 - x) / screen->width);
                float walkEffect = ( ((int) ((fPlayerX + fPlayerY + fPlayerA * 50) * 10)) % 2) * 0.003;
                float b = 1.0 - (((float) y - screen->height / 2.0) / ((float) screen->height / 2.0)) + xPers; //  - walkEffect;

                screen_Draw(screen, x, y, PIXEL_SOLID, 58);
            }
        }
    }

    screen_Flush(screen);
    mvprintw(0, 0, "FPS = %d", 60 - frameDrop);

    refresh();

    isRendering = false;
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
            sprite_Destroy(spriteWall);
            screen_Destroy(screen);
            exit(EXIT_SUCCESS);
    }
}
