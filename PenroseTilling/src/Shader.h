#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct ShaderProgramSource{
	std::string VertexSource;
	std::string FragmentSource;
	std::string GeometrySource;
};

class Shader{
private:
	std::string m_FilePath;
	unsigned int m_RenderID;
	std::unordered_map<std::string, int> m_UniformLocationCache;

public:
	Shader( const std::string &filepath );
	~Shader();

	void Bind() const;
	void UnBind() const;

	// Set Uniforms
	// Set Uniforms
	void Setuniforms1i( const std::string &name, int value );
	void Setuniforms1iv( const std::string &name, int value, int *values );
	void SetUniformFloat( const std::string &name, float value );
	void Setuniforms4f( const std::string &name, float v0, float v1, float v2, float v3 );
	void SetuniformsVec3( const std::string &name, glm::vec3 value );
	void SetUniformsMat4( const std::string &name, glm::mat4 uniform_1, int transpose );
	void SetuniformsMat4f( const std::string &name, const glm::mat4 &mat4 );

private:
	ShaderProgramSource ParseShader( const std::string &filepath );
	unsigned int CompileShader( unsigned int type, const std::string &source );
	unsigned int CreateShader( const std::string &vertexShader, const std::string &fragmentShader, const std::string &geometryShader );
	int GetUniformLocation( const std::string &name );
};