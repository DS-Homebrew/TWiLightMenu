#include "gif.hpp"
#include "myDSiMode.h"
#include "common/twlmenusettings.h"
#include "common/tonccpy.h"
#include "lzw.hpp"

#include <stdio.h>

extern u16* colorTable;

std::vector<Gif *> Gif::_animating;

void Gif::timerHandler(void) {
	for (auto gif : _animating) {
		gif->displayFrame();
	}
}

void Gif::displayFrame(void) {
	if (_paused || ++_currentDelayProgress < _currentDelay)
		return;

	_currentDelayProgress = 0;
	_waitingForInput = false;

	if (_currentFrame >= _frames.size()) {
		_currentFrame = 0;
		_currentLoop++;
	}

	if (_currentLoop > _loopCount) {
		_finished = true;
		_paused = true;
		_currentLoop = 0;
		return;
	}

	Frame &frame = _frames[_currentFrame++];

	if (frame.hasGCE) {
		_currentDelay = frame.gce.delay;
		if (frame.gce.delay == 0) {
			_finished = true;
			_paused = true;
		} else if (frame.gce.userInputFlag) {
			_waitingForInput = true;
		}
	}

	std::vector<u16> &gifColorTable = frame.descriptor.lctFlag ? frame.lct : _gct;
	u16* bgPalette = _top ? BG_PALETTE : BG_PALETTE_SUB;

	tonccpy(bgPalette, gifColorTable.data(), gifColorTable.size() * 2);

	// Disposal method 2 = fill with bg color
	if (frame.gce.disposalMethod == 2)
		toncset(_top ? BG_GFX : BG_GFX_SUB, header.bgColor, 256 * 192);

	if (_compressed) { // Was left compressed to be able to fit
		int x = 0, y = 0;
		u8 *dst = (u8*)(_top ? BG_GFX : BG_GFX_SUB) + (frame.descriptor.y + y + (192 - header.height) / 2) * 256 + frame.descriptor.x + (256 - header.width) / 2;
		u8 row[frame.descriptor.w];
		auto flush_fn = [&dst, &row, &x, &y, &frame](std::vector<u8>::const_iterator begin, std::vector<u8>::const_iterator end) {
			for (; begin != end; ++begin) {
				if (!frame.gce.transparentColorFlag || *begin != frame.gce.transparentColor)
					row[x] = *begin;
				else
					row[x] = *(dst + x);
				x++;
				if (x >= frame.descriptor.w) {
					tonccpy(dst, row, frame.descriptor.w);
					y++;
					x = 0;
					dst += 256;
				}
			}
		};

		LZWReader reader(frame.image.lzwMinimumCodeSize, flush_fn);
		reader.decode(frame.image.imageData.begin(), frame.image.imageData.end());
	} else { // Already decompressed, just copy
		auto it = frame.image.imageData.begin();
		for (int y = 0; y < frame.descriptor.h; y++) {
			u8 *dst = (u8*)(_top ? BG_GFX : BG_GFX_SUB) + (frame.descriptor.y + y + (192 - header.height) / 2) * 256 + frame.descriptor.x + (256 - header.width) / 2;
			u8 row[frame.descriptor.w];
			for (int x = 0; x < frame.descriptor.w; x++, it++) {
				if (!frame.gce.transparentColorFlag || *it != frame.gce.transparentColor)
					row[x] = *it;
				else
					row[x] = *(dst + x);
			}
			tonccpy(dst, row, frame.descriptor.w);
		}
	}
}

bool Gif::load(const char *path, bool top, bool animate, bool forceDecompress) {
	_top = top;

	FILE *file = fopen(path, "rb");
	if (!file)
		return false;

	if (forceDecompress) {
		_compressed = false;
	} else {
		fseek(file, 0, SEEK_END);
		_compressed = ftell(file) > (dsiFeatures() ? 1 << 20 : 1 << 18); // Decompress files bigger than 1MiB (256KiB in DS Mode) while drawing
		fseek(file, 0, SEEK_SET);
	}

	// Reserve space for 2,000 frames
	_frames.reserve(2000);

	// Read header
	fread(&header, 1, sizeof(header), file);

	// Check that this is a GIF
	if (memcmp(header.signature, "GIF87a", sizeof(header.signature)) != 0 && memcmp(header.signature, "GIF89a", sizeof(header.signature)) != 0) {
		fclose(file);
		return false;
	}

	// Load global color table
	if (header.gctFlag) {
		int numColors = (2 << header.gctSize);

		_gct = std::vector<u16>(numColors);
		for (int i = 0; i < numColors; i++) {
			const u8 r = fgetc(file);
			const u8 g = fgetc(file);
			const u8 b = fgetc(file);

			const u16 green = (g >> 2) << 5;
			_gct[i] = r >> 3 | (b >> 3) << 10;
			if (green & BIT(5)) {
				_gct[i] |= BIT(15);
			}
			for (int gBit = 6; gBit <= 10; gBit++) {
				if (green & BIT(gBit)) {
					_gct[i] |= BIT(gBit-1);
				}
			}
			if (colorTable) {
				_gct[i] = colorTable[_gct[i] % 0x8000];
			}
		}
	}

	// Set default loop count to 0, uninitialized default is 0xFFFF so it's infinite
	_loopCount = 0;

	Frame frame;
	while (1) {
		switch (fgetc(file)) {
			case 0x21: { // Extension
				switch (fgetc(file)) {
					case 0xF9: { // Graphics Control
						frame.hasGCE = true;
						fread(&frame.gce, 1, fgetc(file), file);
						if (frame.gce.delay < 2) // If delay is less then 2, change it to 10
							frame.gce.delay = 10;
						fgetc(file); // Terminator
						break;
					} case 0x01: { // Plain text
						// Unsupported for now, I can't even find a text GIF to test with
						// frame.hasText = true;
						// fread(&frame.textDescriptor, 1, sizeof(frame.textDescriptor), file);
						fseek(file, 12, SEEK_CUR);
						while (u8 size = fgetc(file)) {
							// char temp[size + 1];
							// fread(temp, 1, size, file);
							// frame.text += temp;
							fseek(file, size, SEEK_CUR);
						}
						// _frames.push_back(frame);
						// frame = Frame();
						break;
					} case 0xFF: { // Application extension
						if (fgetc(file) == 0xB) {
							char buffer[0xC] = {0};
							fread(buffer, 1, 0xB, file);
							if (strcmp(buffer, "NETSCAPE2.0") == 0) { // Check for Netscape loop count
								fseek(file, 2, SEEK_CUR);
								fread(&_loopCount, 1, sizeof(_loopCount), file);
								if (_loopCount == 0) // If loop count 0 is specified, loop forever
									_loopCount = 0xFFFF;
								fgetc(file); //terminator
								break;
							}
						}
					} case 0xFE: { // Comment
						// Skip comments and unsupported application extionsions
						while (u8 size = fgetc(file)) {
							fseek(file, size, SEEK_CUR);
						}
						break;
					}
				}
				break;
			} case 0x2C: { // Image desriptor
				frame.hasImage = true;
				fread(&frame.descriptor, 1, sizeof(frame.descriptor), file);
				if (frame.descriptor.lctFlag) {
					int numColors = 2 << frame.descriptor.lctSize;
					frame.lct = std::vector<u16>(numColors);
					for (int i = 0; i < numColors; i++) {
						const u8 r = fgetc(file);
						const u8 g = fgetc(file);
						const u8 b = fgetc(file);

						const u16 green = (g >> 2) << 5;
						frame.lct[i] = r >> 3 | (b >> 3) << 10;
						if (green & BIT(5)) {
							frame.lct[i] |= BIT(15);
						}
						for (int gBit = 6; gBit <= 10; gBit++) {
							if (green & BIT(gBit)) {
								frame.lct[i] |= BIT(gBit-1);
							}
						}
						if (colorTable) {
							frame.lct[i] = colorTable[frame.lct[i] % 0x8000];
						}
						_gct[i] = frame.lct[i];
					}
				}

				frame.image.lzwMinimumCodeSize = fgetc(file);
				if (_compressed) { // Leave compressed to fit more in RAM
					while (u8 size = fgetc(file)) {
						size_t end = frame.image.imageData.size();
						frame.image.imageData.resize(end + size);
						fread(frame.image.imageData.data() + end, 1, size, file);
					}
				} else { // Decompress now for faster draw
					frame.image.imageData = std::vector<u8>(frame.descriptor.w * frame.descriptor.h);
					auto it = frame.image.imageData.begin();
					auto flush_fn = [&it, &frame](std::vector<u8>::const_iterator begin, std::vector<u8>::const_iterator end) {
						std::copy(begin, end, it);
						it += std::distance(begin, end);
					};
					LZWReader reader(frame.image.lzwMinimumCodeSize, flush_fn);

					while (u8 size = fgetc(file)) {
						std::vector<u8> buffer(size);
						fread(buffer.data(), 1, size, file);
						reader.decode(buffer.begin(), buffer.end());
					}
				}

				_frames.push_back(frame);
				frame = Frame();
				break;
			} case 0x3B: { // Trailer
				goto breakWhile;
			}
		}
	}
	breakWhile:

	fclose(file);

	_paused = false;
	_finished = loopForever();
	_frames.shrink_to_fit();
	if (animate)
		_animating.push_back(this);

	return true;
}
