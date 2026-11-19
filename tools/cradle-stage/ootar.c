/*
 * ootar: unpacks a ustar archive into the current drawer (files and drawers
 * only). Brings Oolite's Resources into a lab in one lab_put, since the lab
 * has no Shell unpacker. Usage: ootar <archive.tar>
 * Team 5, 19 Nov 2026. MIT.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static unsigned long octal(const char *p, int n)
{
    unsigned long v = 0;
    while (n-- > 0 && *p == ' ') p++, n--;
    while (n-- >= 0 && *p >= '0' && *p <= '7') v = v * 8 + (unsigned long)(*p++ - '0');
    return v;
}

static void makeParents(char *path)
{
    char *s;
    for (s = path; *s; s++) {
        if (*s == '/') { *s = 0; mkdir(path, 0777); *s = '/'; }
    }
}

int main(int argc, char **argv)
{
    static char hdr[512], buf[32768], name[260];
    FILE *in;
    long files = 0, dirs = 0, bytes = 0;
    if (argc != 2) { fprintf(stderr, "usage: ootar <archive.tar>\n"); return 20; }
    in = fopen(argv[1], "rb");
    if (!in) { fprintf(stderr, "ootar: can't open %s\n", argv[1]); return 20; }
    while (fread(hdr, 1, 512, in) == 512) {
        unsigned long size, left;
        char type = hdr[156];
        if (!hdr[0]) break;
        name[0] = 0;
        if (hdr[345]) { strncat(name, hdr + 345, 155); strcat(name, "/"); }
        strncat(name, hdr, 100);
        size = octal(hdr + 124, 12);
        if (type == '5') {
            size_t l = strlen(name);
            if (l && name[l - 1] == '/') name[l - 1] = 0;
            makeParents(name);
            mkdir(name, 0777);
            dirs++;
        } else if (type == '0' || type == 0) {
            FILE *out;
            makeParents(name);
            out = fopen(name, "wb");
            if (!out) { fprintf(stderr, "ootar: can't write %s\n", name); fclose(in); return 20; }
            for (left = size; left > 0;) {
                size_t n = left > sizeof buf ? sizeof buf : left;
                if (fread(buf, 1, n, in) != n) { fprintf(stderr, "ootar: short archive at %s\n", name); fclose(out); fclose(in); return 20; }
                fwrite(buf, 1, n, out);
                left -= n;
            }
            fclose(out);
            files++; bytes += (long)size;
            size = (512 - size % 512) % 512;   /* the padding */
            if (size) fread(buf, 1, size, in);
            continue;
        }
        /* anything else: skip its data */
        for (left = (size + 511) / 512 * 512; left > 0;) {
            size_t n = left > sizeof buf ? sizeof buf : left;
            if (fread(buf, 1, n, in) != n) break;
            left -= n;
        }
    }
    fclose(in);
    printf("ootar: %ld files, %ld drawers, %ld bytes\n", files, dirs, bytes);
    return 0;
}
