#include "Shader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"

Shader::Shader( const std::string &filepath )
	: m_FilePath(filepath), m_RenderID(0){
	ShaderProgramSource source = ParseShader( filepath );
	m_RenderID = CreateShader( source.VertexSource, source.FragmentSource, source.GeometrySource );
}

Shader::~Shader(){
	GLCall( glDeleteProgram( m_RenderID ) );
}

void Shader::Bind() const{
	GLCall( glUseProgram( m_RenderID ) );
}

void Shader::UnBind() const{
	GLCall( glUseProgram( 0 ) );
}

ShaderProgramSource Shader::ParseShader( const std::string &filepath ){
	std::fstream stream( filepath );

	enum class ShaderType{
		NONE = -1, VERTEX = 0, FRAGMENT= 1, GEOMETRY = 2
	};

	std::string line;
	std::stringstream ss[ 3 ];
	ShaderType type = ShaderType::NONE;

	while( getline( stream, line ) ){

		if( line.find( "#shader" ) != std::string::npos ){
			if( line.find( "vertex" ) != std::string::npos ){

				type = ShaderType::VERTEX;

			} else if( line.find( "fragment" ) != std::string::npos ){

				type = ShaderType::FRAGMENT;

			} else if( line.find( "geometry" ) != std::string::npos ){
				type = ShaderType::GEOMETRY;
			}
		} else{
			ss[ ( int ) type ] << line << '\n';
		}

	}

	return { ss[ 0 ].str(), ss[ 1 ].str(), ss[ 2 ].str() };
}

unsigned int Shader::CompileShader( unsigned int type, const std::string &source ){
	GLCall( unsigned int id = glCreateShader( type ) );
	const char *src = source.c_str();
	GLCall( glShaderSource( id, 1, &src, nullptr ) );
	GLCall( glCompileShader( id ) );

	int result;
	GLCall( glGetShaderiv( id, GL_COMPILE_STATUS, &result ) );
	if( result == GL_FALSE ){

		int length;
		GLCall( glGetShaderiv( id, GL_INFO_LOG_LENGTH, &length ) );
		char *message = ( char * ) malloc( length * sizeof( char ) );
		GLCall( glGetShaderInfoLog( id, length, &length, message ) );
		std::cout << "Failed to compile Shader" << ( type == GL_VERTEX_SHADER ? "vertex" : "fragment" ) << std::endl;
		std::cout << message << std::endl;
		GLCall( glDeleteShader( id ) );
		return 0;

	}

	return id;
}

unsigned int Shader::CreateShader( const std::string &vertexShader, const std::string &fragmentShader, const std::string &geometryShader ){
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader( GL_VERTEX_SHADER, vertexShader );
	unsigned int fs = CompileShader( GL_FRAGMENT_SHADER, fragmentShader );
	unsigned int gs = CompileShader( GL_GEOMETRY_SHADER, geometryShader );

	GLCall( glAttachShader( program, vs ) );
	GLCall( glAttachShader( program, fs ) );
	GLCall( glAttachShader( program, gs ) );
	GLCall( glLinkProgram( program ) );
	GLCall( glValidateProgram( program ) );

	GLCall( glDeleteShader( vs ) );
	GLCall( glDeleteShader( fs ) );
	GLCall( glDeleteShader( fs ) );

	return program;
}

int Shader::GetUniformLocation( const std::string &name ){
	if( m_UniformLocationCache.find( name ) != m_UniformLocationCache.end() )
		return m_UniformLocationCache[ name ];

	GLCall( int location = glGetUniformLocation( m_RenderID, name.c_str() ) );
	if( location == -1 )
		std::cout << "Warning: uniform '" << name << "' doesn't exists!" << std::endl;

	m_UniformLocationCache[ name ] = location;
	return location;
}

void Shader::Setuniforms1i( const std::string &name, int value ){
	GLCall( glUniform1i( GetUniformLocation( name ), value ) );
}

void Shader::Setuniforms1iv( const std::string &name, int value, int *values ){
	GLCall( glUniform1iv( GetUniformLocation( name ), value, values ) );
}

void Shader::SetUniformFloat( const std::string &name, float value ){
	GLCall( glUniform1f( GetUniformLocation( name ), value ) );
}

void Shader::Setuniforms4f( const std::string &name, float v0, float v1, float v2, float v3 ){
	GLCall( glUniform4f( GetUniformLocation( name ), v0, v1, v2, v3 ) );
}

void Shader::SetuniformsVec3( const std::string &name, glm::vec3 value ){
	GLCall( glUniform3fv( GetUniformLocation( name ), 1, &value[ 0 ] ) );
}

void Shader::SetUniformsMat4( const std::string &name, glm::mat4 uniform_1, int transpose ){
	
	if( transpose ){
		GLCall( glUniformMatrix4fv( GetUniformLocation( name ), 1, GL_TRUE, glm::value_ptr( uniform_1 ) ) );
	} else{
		GLCall( glUniformMatrix4fv( GetUniformLocation( name ), 1, GL_FALSE, glm::value_ptr( uniform_1 ) ) );
	}

}

void Shader::SetuniformsMat4f( const std::string &name, const glm::mat4 &mat4 ){
	GLCall( glUniformMatrix4fv( GetUniformLocation( name ), 1, GL_FALSE, &mat4[ 0 ][ 0 ] ) );
}