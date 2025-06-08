#include "../public/shader.h"


Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	// check ifstream objects if they can throw exceptions
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;

		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	// vertex shader
	unsigned int vertex;
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);

	// debugging vertex shader compilation
	int success;
	char infoLog[512];
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// fragment shader
	unsigned int fragment;
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

Shader::Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;

	// check ifstream objects if they can throw exceptions
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		gShaderFile.open(geometryPath);
		std::stringstream vShaderStream, fShaderStream, gShaderStream;

		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		gShaderStream << gShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();
		gShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		geometryCode = gShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	const char* gShaderCode = geometryCode.c_str();

	// vertex shader
	unsigned int vertex;
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);

	// debugging vertex shader compilation
	int success;
	char infoLog[512];
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// fragment shader
	unsigned int fragment;
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// geometry shader
	unsigned int geometry;
	geometry = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(geometry, 1, &gShaderCode, NULL);
	glCompileShader(geometry);

	glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(geometry, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, geometry);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
	glDeleteShader(geometry);
}

Shader::Shader(const char* computePath)
{
	std::string computeCode;
	std::ifstream cShaderFile;
	cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		cShaderFile.open(computePath);
		std::stringstream cShaderStream;
		cShaderStream << cShaderFile.rdbuf();
		cShaderFile.close();
		computeCode = cShaderStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: "
			<< computePath << std::endl;
	}

	const char* cShaderCode = computeCode.c_str();

	unsigned int computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &cShaderCode, NULL);
	glCompileShader(computeShader);

	int success;
	char infoLog[512];
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n"
			<< infoLog << std::endl;
	}

	ID = glCreateProgram();
	glAttachShader(ID, computeShader);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
			<< infoLog << std::endl;
	}

	glDeleteShader(computeShader);
}

Shader::Shader(const char* vertexPath, const char* tesCtrlPath, const char* tesEvalPath, const char* fragmentPath)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::string tesControlCode;
	std::string tesEvaluationCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream tcShaderFile;
	std::ifstream teShaderFile;

	// check ifstream objects if they can throw exceptions
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	tcShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	teShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		tcShaderFile.open(tesCtrlPath);
		teShaderFile.open(tesEvalPath);

		std::stringstream vShaderStream, tcShaderStream, teShaderStream, fShaderStream;

		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		tcShaderStream << tcShaderFile.rdbuf();
		teShaderStream << teShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();
		tcShaderFile.close();
		teShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		tesControlCode = tcShaderStream.str();
		tesEvaluationCode = teShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	const char* tcShaderCode = tesControlCode.c_str();
	const char* teShaderCode = tesEvaluationCode.c_str();

	// vertex shader
	unsigned int vertex;
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);

	// debugging vertex shader compilation
	int success;
	char infoLog[512];
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// tessellation control shader
	unsigned int tesControl;
	tesControl = glCreateShader(GL_TESS_CONTROL_SHADER);
	glShaderSource(tesControl, 1, &tcShaderCode, NULL);
	glCompileShader(tesControl);

	glGetShaderiv(tesControl, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(tesControl, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::TESSELLATION CONTROL::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// tessellation evaluation shader
	unsigned int tesEvaluation;
	tesEvaluation = glCreateShader(GL_TESS_EVALUATION_SHADER);
	glShaderSource(tesEvaluation, 1, &teShaderCode, NULL);
	glCompileShader(tesEvaluation);

	glGetShaderiv(tesEvaluation, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(tesEvaluation, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::TESSELLATION EVALUATION::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// fragment shader
	unsigned int fragment;
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, tesControl);
	glAttachShader(ID, tesEvaluation);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(tesControl);
	glDeleteShader(tesEvaluation);
	glDeleteShader(fragment);
}

void Shader::use()
{
	glUseProgram(ID);
}
void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(getUniformLocation(name), (int)value);
}
void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(getUniformLocation(name), value);
}
void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& vec) const
{
	glUniform2f(getUniformLocation(name), vec.x, vec.y);
}

void Shader::setIVec2(const std::string& name, float x, float y) const
{
	glUniform2i(getUniformLocation(name), x, y);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
	glUniform3f(getUniformLocation(name), x, y, z);
}

void Shader::setVec3(const std::string& name, const glm::vec3& vec) const
{
	glUniform3f(getUniformLocation(name), vec.x, vec.y, vec.z);
}

void Shader::setVec4(const std::string& name, const glm::vec4& vec) const
{
	glUniform4f(getUniformLocation(name), vec.x, vec.y, vec.z, vec.w);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setSamplerArray(const std::string& name, const std::vector<GLuint> texIDs, int firstUnit, GLenum target) const
{
	std::vector<GLint> units(texIDs.size());
	for (std::size_t i = 0; i < texIDs.size(); ++i)
		units[i] = static_cast<GLint>(firstUnit + i);

	GLint location = glGetUniformLocation(ID, name.c_str());
	glUniform1iv(location, static_cast<GLsizei>(units.size()), units.data());

	for (unsigned int i = 0; i < texIDs.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + units[i]);
		glBindTexture(target, texIDs[i]);
	}
}

GLint Shader::getUniformLocation(const std::string& name) const
{
	if (uniformLocationCache.find(name) != uniformLocationCache.end()) return uniformLocationCache[name];
	GLint location = glGetUniformLocation(ID, name.c_str());
	uniformLocationCache[name] = location;
	return location;
}


