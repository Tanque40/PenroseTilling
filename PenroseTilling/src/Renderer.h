#pragma once

#include <GL/glew.h>
#include "IndexBuffer.h"

#define ASSERT(x) if ((!(x)) ) __debugbreak();
#define GLCall(x) GLClearError(); \
		x; \
		ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall( const char *fucntion, const char *file, int line );

class Renderer{
public:
	void Clear() const;
	void Draw( const Vertexarray &va, const IndexBuffer &ib, const Shader &shader ) const;
};