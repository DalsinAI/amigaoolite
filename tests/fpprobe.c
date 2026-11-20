/* FPU float -> integer probe (Team 5, 20 Nov 2026). On the CM5 (Pi 5) Oolite's
   (int)viewSize.width printed 0x44A00000, the bits of 1280.0f, where the PC
   lab printed 1280. Each line: what the CPU gave, then what is right.
   MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <math.h>

static int asm_fmove_l(double d)     /* FMOVE.L FPn,Dn: rounds by FPCR (nearest) */
{
    int r;
    __asm__ volatile ("fmove.d %1,%%fp0\n\tfmove.l %%fp0,%0" : "=d"(r) : "m"(d) : "fp0");
    return r;
}

static int asm_fintrz(double d)      /* FINTRZ then FMOVE.L: what (int) compiles to */
{
    int r;
    __asm__ volatile ("fmove.d %1,%%fp0\n\tfintrz.x %%fp0,%%fp0\n\tfmove.l %%fp0,%0" : "=d"(r) : "m"(d) : "fp0");
    return r;
}

static int asm_fmove_s_l(float f)    /* single precision source */
{
    int r;
    __asm__ volatile ("fmove.s %1,%%fp0\n\tfmove.l %%fp0,%0" : "=d"(r) : "m"(f) : "fp0");
    return r;
}

static int fails;
static void line(const char *what, long got, long want)
{
    printf("%-28s %11ld (0x%08lx)  want %ld  %s\n", what, got, (unsigned long)got, want, got == want ? "ok" : "WRONG");
    if (got != want) fails++;
}

int main(void)
{
    volatile float f1280 = 1280.0f, fhalf = 0.5f, f720 = 720.0f, fneg = -3.75f;
    volatile double d1280 = 1280.0, d275 = 2.75, dneg = -3.75, dbig = 2147483000.0;
    line("(int)1280.0f", (int)f1280, 1280);
    line("(int)720.0f", (int)f720, 720);
    line("(int)0.5f", (int)fhalf, 0);
    line("(int)-3.75f", (int)fneg, -3);
    line("(int)1280.0", (int)d1280, 1280);
    line("(int)2.75", (int)d275, 2);
    line("(int)-3.75", (int)dneg, -3);
    line("(int)2147483000.0", (int)dbig, 2147483000L);
    line("(unsigned)1280.0f", (long)(unsigned)f1280, 1280);
    line("(short)720.0f", (short)f720, 720);
    line("lrint(2.75)", lrint(d275), 3);
    line("fmove.l 2.75", asm_fmove_l(2.75), 3);
    line("fmove.l 1280.0", asm_fmove_l(1280.0), 1280);
    line("fintrz+fmove.l 2.75", asm_fintrz(2.75), 2);
    line("fintrz+fmove.l -3.75", asm_fintrz(-3.75), -3);
    line("fmove.s->l 1280.0f", asm_fmove_s_l(1280.0f), 1280);
    line("fmove.s->l 0.5f", asm_fmove_s_l(0.5f), 0);
    printf("fpprobe: %d wrong\n", fails);
    return fails ? 10 : 0;
}
