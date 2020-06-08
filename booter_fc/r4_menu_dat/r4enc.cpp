/*
Copyright(C) 2007 yasu
http://hp.vector.co.jp/authors/VA013928/
http://www.usay.jp/
http://www.yasu.nu/

2007/04/22 21:00 - First version
*/

#include <stdio.h>

#define BIT_AT(n, i) ((n >> i) & 1)

int main(int argc, char *argv[])
{
	puts("Yasu software - R4 Encrypt");
	if (argc < 2)
	{
		puts("Usage: r4enc in-file");
		return 1;
	}

	FILE *in = fopen(argv[1], "rb");
	if (in == NULL)
	{
		printf("Error: cannot open %s\n", argv[1]);
		return 1;
	}

	unsigned char buf[512];
	sprintf(buf, "%s.DAT", argv[1]);
	FILE *out = fopen(buf, "wb");
	if (out == NULL)
	{
		fclose(in);
		printf("Error: cannot open %s\n", buf);
		return 1;
	}

	int r, n = 0;
	while ((r = fread(buf, 1, 512, in)) > 0)
	{
		unsigned short key = n ^ 0x484A;
		for (int i = 0; i < 512; i ++)
		{
			unsigned char xor = 0;
			if (key & 0x4000) xor |= 0x80;
			if (key & 0x1000) xor |= 0x40;
			if (key & 0x0800) xor |= 0x20;
			if (key & 0x0200) xor |= 0x10;
			if (key & 0x0080) xor |= 0x08;
			if (key & 0x0040) xor |= 0x04;
			if (key & 0x0002) xor |= 0x02;
			if (key & 0x0001) xor |= 0x01;

			buf[i] ^= xor;

			unsigned int k = ((buf[i] << 8) ^ key) << 16;
			unsigned int x = k;
			for (int j = 1; j < 32; j ++)
				x ^= k >> j;
			key = 0x0000;
			if (BIT_AT(x, 23)) key |= 0x8000;
			if (BIT_AT(k, 22)) key |= 0x4000;
			if (BIT_AT(k, 21)) key |= 0x2000;
			if (BIT_AT(k, 20)) key |= 0x1000;
			if (BIT_AT(k, 19)) key |= 0x0800;
			if (BIT_AT(k, 18)) key |= 0x0400;
			if (BIT_AT(k, 17) != BIT_AT(x, 31)) key |= 0x0200;
			if (BIT_AT(k, 16) != BIT_AT(x, 30)) key |= 0x0100;
			if (BIT_AT(k, 30) != BIT_AT(k, 29)) key |= 0x0080;
			if (BIT_AT(k, 29) != BIT_AT(k, 28)) key |= 0x0040;
			if (BIT_AT(k, 28) != BIT_AT(k, 27)) key |= 0x0020;
			if (BIT_AT(k, 27) != BIT_AT(k, 26)) key |= 0x0010;
			if (BIT_AT(k, 26) != BIT_AT(k, 25)) key |= 0x0008;
			if (BIT_AT(k, 25) != BIT_AT(k, 24)) key |= 0x0004;
			if (BIT_AT(k, 25) != BIT_AT(x, 26)) key |= 0x0002;
			if (BIT_AT(k, 24) != BIT_AT(x, 25)) key |= 0x0001;
		}
		fwrite(buf, 1, r, out);
		n ++;
	}
	fclose(out);
	fclose(in);

	return 0;
}
