#ifndef GIF_HPP
#define GIF_HPP

#include <nds/ndstypes.h>
#include <vector>

typedef unsigned int uint;

class Gif {
	struct Header {
		char signature[6];
		u16 width;
		u16 height;
		u8 gctSize: 3;
		u8 sortFlag: 1;
		u8 colorResolution: 3;
		u8 gctFlag: 1;
		u8 bgColor;
		u8 pixelAspectRatio;
	} __attribute__ ((__packed__)) header;
	static_assert(sizeof(Header) == 13);

	struct Frame {
		struct GraphicsControlExtension {
			u8 transparentColorFlag: 1;
			u8 userInputFlag: 1;
			u8 disposalMethod: 3;
			u8 reserved: 3;
			u16 delay; // In hundreths (1/100) of a second
			u8 transparentColor;
		} __attribute__ ((__packed__)) gce;
		static_assert(sizeof(GraphicsControlExtension) == 4);

		// Unsupported for now
		// struct PlainText {
		// 	u16 gridX;
		// 	u16 gridY;
		// 	u16 gridW;
		// 	u16 gridH;
		// 	u8 charW;
		// 	u8 charH;
		// 	u8 forgroundIndex;
		// 	u8 backgroundIndex;
		// } __attribute__ ((__packed__)) textDescriptor;
		// static_assert(sizeof(PlainText) == 12);

		struct Descriptor {
			u16 x;
			u16 y;
			u16 w;
			u16 h;
			u8 lctSize: 3;
			u8 reserved: 2;
			u8 sortFlag: 1;
			u8 interlaceFlag: 1;
			u8 lctFlag: 1;
		} __attribute__ ((__packed__)) descriptor;
		static_assert(sizeof(Descriptor) == 9);

		struct Image {
			u8 lzwMinimumCodeSize;
			std::vector<u8> imageData;
		} image;

		std::vector<u16> lct; // In DS format
		// std::string text;
		bool hasGCE = false;
		// bool hasText = false;
		bool hasImage = false;
	};

	std::vector<Frame> _frames;
	std::vector<u16> _gct; // In DS format
	u16 _loopCount = 0xFFFF;
	bool _top = false;
	bool _compressed = false;

	// Animation vairables
	static std::vector<Gif *> _animating;
	uint _currentFrame = 0;
	uint _currentDelay = 0;
	uint _currentDelayProgress = 0;
	u16 _currentLoop = 0;
	bool _paused = true;
	bool _finished = true;

	bool _waitingForInput = false;

	static void animate(bool top);

public:
	static void timerHandler(void);

	Gif () {}
	Gif (const char *path, bool top, bool animate) { load(path, top, animate); }
	~Gif () {}

	bool load(const char *path, bool top, bool animate);

	Frame &frame(int frame) { return _frames[frame]; }

	void displayFrame(void);

	bool paused() { return _paused; }
	void pause() { _paused = true; }
	void unpause() { _paused = false; }
	void toggle() { _paused = !_paused; }

	bool loopForever(void) { return _loopCount == 0xFFFF; }
	bool waitingForInput(void) { return _waitingForInput; }
	void resume(void) { _waitingForInput = false; _currentDelayProgress = _currentDelay; }
	bool finished(void) { return _finished; }

	int currentFrame(void) { return _currentFrame; }
};

#endif
