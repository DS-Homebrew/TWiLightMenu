#include "lzw.hpp"

u16 LZWReader::readLSB(std::vector<u8>::iterator &begin, const std::vector<u8>::iterator &end) {
	while (nBits < width) {
		if (begin == end) {
			err = true;
			return 0;
		}
		u8 x = *(begin++);
		bits |= x << nBits;
		nBits += 8;
	}
	u16 code = bits & ((1 << width) - 1);
	bits >>= width;
	nBits -= width;
	return code;
}

bool LZWReader::decode(std::vector<u8>::iterator begin, std::vector<u8>::iterator end) {
	o = 0;
	err = false;
	// Loop over the code stream, converting codes into decompressed bytes.
	while (begin != end) {
		u16 code = readLSB(begin, end);
		if (err) {
			flush();
			return false;
		}

		if (code < clear) { // Literal
			output[o++] = code;
			if (last != DECODER_INVALID_CODE) {
				// Save what the hi code expands to.
				suffix[hi] = code;
				prefix[hi] = last;
			}
		} else if (code == clear) { // Clear
			width = 1 + litWidth;
			hi = eof;
			overflow = 1 << width;
			last = DECODER_INVALID_CODE;
			continue;
		} else if (code == eof) { // End
			flush();
			return true;
		} else if (code <= hi) {
			u16 c = code;
			uint i = output.size() - 1;
			if (code == hi && last != DECODER_INVALID_CODE) {
				// code == hi is a special case which expands to the last expansion
				// followed by the head of the last expansion. To find the head, we walk
				// the prefix chain until we find a literal code.
				c = last;
				while (c >= clear)
					c = prefix[c];
				output[i] = c;
				i--;
				c = last;
			}
			// Copy the suffix chain into output and then write that to w.
			while (c >= clear) {
				output[i] = suffix[c];
				i--;
				c = prefix[c];
			}
			output[i] = c;
			std::copy(output.begin() + i, output.end(), output.begin() + o);
			o += std::distance(output.begin() + i, output.end());
			if (last != DECODER_INVALID_CODE) {
				// Save what the hi code expands to
				suffix[hi] = c;
				prefix[hi] = last;
			}
		} else { // Error
			flush();
			return false;
		}

		last = code;
		hi++;
		if (hi >= overflow) {
			if (hi > overflow) {
				flush();
				return false;
			}

			if (width == MAX_WIDTH) {
				last = DECODER_INVALID_CODE;
				// Undo the d.hi++ a few lines above, so that (1) we maintain
				// the invariant that d.hi < d.overflow, and (2) d.hi does not
				// eventually overflow a uint16.
				hi--;
			} else {
				width++;
				overflow = 1 << width;
			}
		}
		if (o >= FLUSH_BUFFER) {
			flush();
		}
	}

	flush();
	return true;
}

LZWReader::LZWReader(int minCodeSize, std::function<void(u8_itr, u8_itr)> flushFunction) : litWidth(minCodeSize), flushFn(flushFunction) {
	width = 1 + litWidth;
	clear = 1 << litWidth;
	eof = clear + 1;
	hi = clear + 1;
	overflow = 1 << width;
	last = DECODER_INVALID_CODE;

	suffix = std::vector<u8>(1 << MAX_WIDTH);
	prefix = std::vector<u16>(1 << MAX_WIDTH);
	output = std::vector<u8>(2 * (1 << MAX_WIDTH));
}

void LZWReader::flush(void) {
	if (flushFn && o > 0) {
		flushFn(output.begin(), output.begin() + o);
	}
	o = 0;
}
