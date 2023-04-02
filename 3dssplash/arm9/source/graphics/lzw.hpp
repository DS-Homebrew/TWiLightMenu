#ifndef LZW_HPP
#define LZW_HPP

#include <nds.h>
#include <functional>
#include <vector>

typedef unsigned int uint;
typedef std::vector<u8>::const_iterator u8_itr;

class LZWReader {
	constexpr static u16 MAX_WIDTH = 12;
	constexpr static u16 DECODER_INVALID_CODE = 0xFFFF;
	constexpr static u16 FLUSH_BUFFER = 1 << MAX_WIDTH;

	int litWidth;
	std::function<void(u8_itr, u8_itr)> flushFn;
	u32 bits = 0;
	uint nBits = 0;
	uint width;
	bool err = false;

	u16 clear, eof, hi, overflow, last;

	std::vector<u8> suffix;
	std::vector<u16> prefix;

	std::vector<u8> output;
	int o = 0;
	// std::vector<u8> toRead;

	u16 readLSB(std::vector<u8>::iterator &it, const std::vector<u8>::iterator &end);

	int read(std::vector<u8> &buffer);

	void flush(void);

public:
	LZWReader(int minCodeSize, std::function<void(u8_itr, u8_itr)> flushFunction);

	bool decode(std::vector<u8>::iterator begin, std::vector<u8>::iterator end);
};

#endif
