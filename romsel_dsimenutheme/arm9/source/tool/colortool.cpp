

unsigned short convertVramColorToGrayscale(unsigned short val) {
	unsigned char b,g,r,max,min;
	b = ((val)>>10)&31;
	g = ((val)>>5)&31;
	r = (val)&31;
	// Value decomposition of hsv
	max = (b > g) ? b : g;
	max = (max > r) ? max : r;

	// Desaturate
	min = (b < g) ? b : g;
	min = (min < r) ? min : r;
	max = (max + min) / 2;

	return (val & 32768)|(max<<10)|(max<<5)|(max);
}
