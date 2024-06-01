#ifndef _UTILS_H_
#define _UTILS_H_

bool is_png(const char *filename);
char* add_infix(const char *filename, const char *infix);
char *args_shift(int *argc, char ***argv);
float to_luminance(int r, int g, int b);
float min(float a, float b);
float max(float a, float b);
float min3(float a, float b, float c);
float max3(float a, float b, float c);

#endif
