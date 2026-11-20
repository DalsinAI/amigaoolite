// Probe: does DWARF exception unwinding work in a GFOS hunk executable?
#include <stdio.h>
static int thrower(int x) { if (x) throw 42; return 0; }
int main(int argc, char **argv)
{
    (void)argv;
    int caught = 0;
    try { thrower(argc); } catch (int v) { caught = v; }
    printf("%s gfos_eh_probe (caught %d)\n", caught == 42 ? "PASS" : "FAIL", caught);
    return caught == 42 ? 0 : 10;
}
