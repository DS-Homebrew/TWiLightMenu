#include "launchDots.h"
#include "ThemeTextures.h"
#include "common/gl2d.h"
#include <algorithm>

s16 launchDotX[12];
s16 launchDotY[12];

bool launchDotXMove[12]; // false = left, true = right
bool launchDotYMove[12]; // false = up, true = down

LaunchDots::LaunchDots() {

	launchDotXMove[0] = false;
	launchDotYMove[0] = true;
	launchDotX[0] = 44;
	launchDotY[0] = 0;
	launchDotXMove[1] = false;
	launchDotYMove[1] = true;
	launchDotX[1] = 28;
	launchDotY[1] = 16;
	launchDotXMove[2] = false;
	launchDotYMove[2] = true;
	launchDotX[2] = 12;
	launchDotY[2] = 32;
	launchDotXMove[3] = true;
	launchDotYMove[3] = true;
	launchDotX[3] = 4;
	launchDotY[3] = 48;
	launchDotXMove[4] = true;
	launchDotYMove[4] = true;
	launchDotX[4] = 20;
	launchDotY[4] = 64;
	launchDotXMove[5] = true;
	launchDotYMove[5] = true;
	launchDotX[5] = 36;
	launchDotY[5] = 80;
	launchDotXMove[6] = true;
	launchDotYMove[6] = false;
	launchDotX[6] = 52;
	launchDotY[6] = 80;
	launchDotXMove[7] = true;
	launchDotYMove[7] = false;
	launchDotX[7] = 68;
	launchDotY[7] = 64;
	launchDotXMove[8] = true;
	launchDotYMove[8] = false;
	launchDotX[8] = 84;
	launchDotY[8] = 48;
	launchDotXMove[9] = false;
	launchDotYMove[9] = false;
	launchDotX[9] = 76;
	launchDotY[9] = 36;
	launchDotXMove[10] = false;
	launchDotYMove[10] = false;
	launchDotX[10] = 60;
	launchDotY[10] = 20;
	launchDotX[11] = 44;
	launchDotY[11] = 0;
}

// F(p, t) = (R(t), p*pi/6 + Vt)
// 32 degrees = 3276 brads

#define DOT_INTERVAL 2730
#define DOT_INIT_RADIUS 36
#define BRAD_90_DEG 8192

int getRadius(int frame)
{
    return DOT_INIT_RADIUS + std::min(2 * frame, 16);
}

int getVelocity(int frame)
{
    if (frame < 16) return 0;
    return 182; // 182 - 2 degrees of rotation at frame.
}

int getDotRadiusFrame(int dotIndex, int frame)
{
    return 5;
    // return ((frame % 4) + dotIndex) % 5; return 5
}

int getDotX(int dotIndex, int frame)
{
    return getRadius(frame) * fixedToFloat(cosLerp((dotIndex * DOT_INTERVAL) - BRAD_90_DEG - (frame * getVelocity(frame))), 12);
}

int getDotY(int dotIndex, int frame)
{
    return getRadius(frame) * fixedToFloat(sinLerp((dotIndex * DOT_INTERVAL) - BRAD_90_DEG - (frame * getVelocity(frame))), 12);
}

static int radFrame = 0;

void LaunchDots::draw() {
	for (int i = 0; i < 12; i++) {
        int X = getDotX(i, radFrame);
        int Y =  getDotY(i, radFrame);
        int dotFrame = getDotRadiusFrame(i, radFrame);
        if (dotFrame == -1) continue;
		glSprite((128 - (getRadius(0) >> 2)) + X, (96 + (getRadius(0) >> 2) + (getRadius(0) >> 3)) + Y, GL_FLIP_NONE,
             &tex().launchdotImage()[dotFrame & 15]);
	}
   
    if (radFrame < 64) {
        radFrame++;
    } else {
        radFrame = 0;
    }    
}