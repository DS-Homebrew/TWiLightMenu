// This file is licensed CC0-1.0 by Taiju Yamada.

#include <stdio.h>
#include <string.h>

// arbitrary number. Don't know if this can be changed, because other converters of this type uses this value too... -lifehackerhansol
#define S 433264

#define BUFLEN (1<<19)

int main(int argc, char **argv) {
    int size = S - 0x450, u;
    if(argc < 3) {
        fprintf(stderr, "r4isdhc in out\n");
        return 1;
    }
    FILE* in = fopen(argv[1], "rb");
    if(!in) {
        fprintf(stderr, "cannot open\n");
        return 2;
    }
    FILE* out = fopen(argv[2], "wb");
    if(!out) {
        fclose(in);
        fprintf(stderr, "cannot open\n");
        return 3;
    }
    unsigned char buf[BUFLEN];
    memset(buf, 0, 0x450);
    buf[0] = 0x12, buf[1] = 0x01, buf[2] = 0x00, buf[3] = 0xea;
    buf[0xec] = 0x49, buf[0xed] = 0x00, buf[0xee] = 0x00, buf[0xef] = 0xeb;
    fwrite(buf, 1, 0x450, out);
    while(u = fread(buf, 1, BUFLEN, in)) fwrite(buf, 1, u, out), size-=u;
    fclose(in);
    memset(buf, 0, BUFLEN);
    while(size > 0) fwrite(buf, 1, size < BUFLEN ? size : BUFLEN, out), size -= BUFLEN;
    fclose(out);
    return 0;
}
