/* Native SGI GLU 9.0.3 for Oolite GFOS GL logging, no GL context. */
#include <GL/glu.h>
#include <GL/gl.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    const GLubyte *s=gluErrorString(GL_INVALID_ENUM);
    const GLubyte *s2=gluErrorString(GL_INVALID_VALUE);
    int ok=s && s2 && strlen((const char *)s)>0 && strlen((const char *)s2)>0;
    printf("OOLITE_GFOS_GLU enum=%s value=%s status=%s\n",
      s?(const char *)s:"NULL",s2?(const char *)s2:"NULL",ok?"PASS":"FAIL");
    fflush(stdout);
    return ok?0:2;
}
