/* Diagnostic only: declarations missing in target complex.h but required
 * by GCC16 tgmath generic selection even for real float/double calls.
 * The target libc does not provide these complex-long-double functions;
 * real-valued call sites must link to the existing real math functions.
 */
#include <complex.h>
#if defined(__amigaos__) && defined(__GNUC__)
extern long double _Complex csinl(long double _Complex);
extern long double _Complex ccosl(long double _Complex);
extern long double _Complex ctanl(long double _Complex);
extern long double _Complex cpowl(long double _Complex, long double _Complex);
extern long double _Complex cacosl(long double _Complex);
extern long double _Complex casinl(long double _Complex);
extern long double _Complex catanl(long double _Complex);
extern long double _Complex cexpl(long double _Complex);
extern long double _Complex clogl(long double _Complex);
extern long double _Complex csqrtl(long double _Complex);
#endif
