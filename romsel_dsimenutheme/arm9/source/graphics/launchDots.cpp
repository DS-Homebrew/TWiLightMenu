#include "launchDots.h"
#include "ThemeTextures.h"
#include <gl2d.h>
#include <algorithm>
#include <cmath>

// F(p, t) = (R(t), p*pi/6 + Vt)
// 32 degrees = 3276 brads

// Spacing between dots in binary radians 
#define DOT_INTERVAL 2730

// Initial radius of the circle
#define DOT_INIT_RADIUS 36

// 90 degrees in binary radians
#define BRAD_90_DEG 8192

// number of timesteps to animate
#define DOTS_NUM_TIMESTEPS 32

/*
 * getRadius, getVelocity, and getDotRadiusFrame are the curves that 
 * define the behaviour of the circle
 */

// Gets the radius for the given frame
inline int getRadius(int frame) {
     return DOT_INIT_RADIUS + std::min((int)(4 * frame * log(frame)), 10);
}

// Gets the angular velocity of the radius in brads for the given frame
inline int getVelocity(int frame) {
	if (frame < 8)
		return 182 >> 2; //rotate slowly 
	return 91 + (4 * frame); // 91 - 1 degrees of rotation at frame.
}

// This curve could be improved...
// Gets the weight/ frame of the dot from 0 (empty) to 5 (largest) for the given index and frame
inline int getDotRadiusFrame(int dotIndex, int frame) {
    if (frame - dotIndex < 2) return 5;
    if (frame - dotIndex > 8) return 0;
    // Sin between [-1. 1] => [0, 2]
    float SIN = fixedToFloat(sinLerp((dotIndex * DOT_INTERVAL) - BRAD_90_DEG - (frame * getVelocity(frame))), 12) + 1; 

    return std::min((SIN * 10 / 4), (float)5);
    
	// return ((frame % 4) + dotIndex) % 5; return 5
}


inline int getDotX(int dotIndex, int frame) {
	return getRadius(frame) *
	       fixedToFloat(cosLerp((dotIndex * DOT_INTERVAL) - BRAD_90_DEG - (frame * getVelocity(frame))), 12);
}

inline int getDotY(int dotIndex, int frame) {
	return getRadius(frame) *
	       fixedToFloat(sinLerp((dotIndex * DOT_INTERVAL) - BRAD_90_DEG - (frame * getVelocity(frame))), 12);
}

static int radFrame = 0;

void LaunchDots::drawFrame(int frame) {
	for (int i = 0; i < 12; i++) {
		int X = getDotX(i, frame);
		int Y = getDotY(i, frame);
		int dotFrame = getDotRadiusFrame(i, frame);
		if (dotFrame == -1)
			continue;
		glSprite((128 - (DOT_INIT_RADIUS >> 2)) + X, (96 + (DOT_INIT_RADIUS  >> 2) + (DOT_INIT_RADIUS >> 3)) + Y,
			 GL_FLIP_NONE, &tex().launchdotImage()[dotFrame & 15]);
	}
}
void LaunchDots::drawAuto() {
    drawFrame(radFrame);
	if (radFrame < DOTS_NUM_TIMESTEPS) {
		radFrame++;
	} else {
		radFrame = DOTS_NUM_TIMESTEPS;
	}
}