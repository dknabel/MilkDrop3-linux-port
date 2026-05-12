#pragma once

#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

// GLAD loader function
typedef void* (*GLADloadproc)(const char *name);

int gladLoadGL(void);
int gladLoadGLLoader(GLADloadproc load);

#ifdef __cplusplus
}
#endif
