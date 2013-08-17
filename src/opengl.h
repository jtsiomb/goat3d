#ifndef OPENGL_H_
#define OPENGL_H_

#include <stdlib.h>

#ifdef __APPLE__
#include "TargetConditionals.h"

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
/* iOS */
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#define GL_CLAMP			GL_CLAMP_TO_EDGE
#define GL_DEPTH24_STENCIL8	GL_DEPTH24_STENCIL8_OES

#undef USE_OLDGL

#define GL_WRITE_ONLY	GL_WRITE_ONLY_OES
#define glMapBuffer		glMapBufferOES
#define glUnmapBuffer	glUnmapBufferOES

#else
/* MacOS X */
#include <GL/glew.h>
#include <GLUT/glut.h>

#define USE_OLDGL
#endif

#else
/* UNIX or Windows */
#include <GL/glew.h>
#include <GL/glut.h>

#define USE_OLDGL
#endif

#ifndef GL_RGB16F
#define GL_RGB16F	0x881b
#endif
#ifndef GL_RGBA16F
#define GL_RGBA16F	0x881a
#endif
#ifndef GL_RGB32F
#define GL_RGB32F	0x8815
#endif
#ifndef GL_RGBA32F
#define GL_RGBA32F	0x8814
#endif
#ifndef GL_LUMINANCE16F
#define GL_LUMINANCE16F	0x881e
#endif
#ifndef GL_LUMINANCE32F
#define GL_LUMINANCE32F	0x8818
#endif

#define CHECKGLERR	\
	do { \
		int err = glGetError(); \
		if(err) { \
			fprintf(stderr, "%s:%d: OpenGL error 0x%x: %s\n", __FILE__, __LINE__, err, strglerr(err)); \
			abort(); \
		} \
	} while(0)

void init_opengl();

const char *strglerr(int err);

#endif	// OPENGL_H_
