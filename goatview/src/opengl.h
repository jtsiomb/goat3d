#ifndef OPENGL_H_
#define OPENGL_H_

#include <GL/glew.h>
#define QT_NO_OPENGL_ES_2

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#undef min
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

#endif	/* OPENGL_H_ */
