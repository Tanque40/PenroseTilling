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

private:
	ShaderProgramSource ParseShader( const std::string &filepath );
	unsigned int CompileShader( unsigned int type, const std::string &source );
	unsigned int CreateShader( const std::string &vertexShader, const std::string &fragmentShader, const std::string &geometryShader );
	int GetUniformLocation( const std::string &name );
};