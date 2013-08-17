#include "opengl.h"

void init_opengl()
{
#ifdef __GLEW_H__
	glewInit();
#endif
}

const char *strglerr(int err)
{
	static const char *errnames[] = {
		"GL_INVALID_ENUM",
		"GL_INVALID_VALUE",
		"GL_INVALID_OPERATION",
		"GL_STACK_OVERFLOW",
		"GL_STACK_UNDERFLOW",
		"GL_OUT_OF_MEMORY",
		"GL_INVALID_FRAMEBUFFER_OPERATION"
	};

	if(!err) {
		return "GL_NO_ERROR";
	}
	if(err < GL_INVALID_ENUM || err > GL_OUT_OF_MEMORY) {
		return "<invalid gl error>";
	}
	return errnames[err - GL_INVALID_ENUM];
}
