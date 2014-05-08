#ifndef OPENGL_H_
#define OPENGL_H_

#include <GL/glew.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int init_opengl(void);

#ifdef __cplusplus
}
#endif

#endif	/* OPENGL_G_ */
