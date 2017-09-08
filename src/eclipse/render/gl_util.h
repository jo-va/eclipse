#pragma once

#include <iostream>
#include <GL/glew.h>

#define CHECK_GL_ERROR() {   															\
    GLenum err; bool failure = false;													\
	while ((err = glGetError()) != GL_NO_ERROR) {										\
        failure = true;																	\
        std::cerr << glewGetErrorString(err) << " ["								    \
				  << err << " - " << __FILE__ << ":" << __LINE__ << "]" << std::endl;   \
	}																					\
    if (failure) exit(EXIT_FAILURE);                                                    \
}

void print_log(GLuint object)
{
    GLint log_length = 0;
    if (glIsShader(object))
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else if (glIsProgram(object))
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else {
        std::cerr << "print_log: Not a shader or program" << std::endl;
        return;
    }
    char log[256];

    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, &log[0]);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, &log[0]);

    std::cerr << log << std::endl;
}

GLuint create_shader(GLenum type, const char* shader_source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    GLint compiled_ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled_ok);
    if (compiled_ok == GL_FALSE) {
        print_log(shader);
        return 0;
    }
    return shader;
}

