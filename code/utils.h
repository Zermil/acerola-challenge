#ifndef UTILS_H
#define UTILS_H

#define internal static
#define global static

#define UNUSED(x) ((void)(x))

const char *load_entire_file(const char *filename);
void move_towards(float *val, float target, float dt, float rate_up, float rate_down = -1.0f);

#endif // UTILS_H
