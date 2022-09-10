#include "IndexBuffer.h"
#include "Renderer.h"

IndexBuffer::IndexBuffer( const unsigned int *data, unsigned int count )
	: m_Count(count){

	ASSERT( sizeof( unsigned int ) == sizeof( GLuint ) );
}
