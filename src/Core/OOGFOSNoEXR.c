/* GFOS: no HDR snapshots. MyOpenGLView saves an .exr beside the normal one
 * only when the display outputs HDR, which GFOS's never does, so the game
 * links no tinyexr or miniz. Returns failure, as tinyexr does when it can't.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
int SaveEXRSnapshot(const char *outfilename, int width, int height, const float *rgb)
{
    (void)outfilename; (void)width; (void)height; (void)rgb;
    return -1;
}
