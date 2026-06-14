#include "stdafx.h"
#include "Renderer.h"
#include "LoadPng.h"
#include <assert.h>
#include <Windows.h>

Renderer::Renderer(int windowSizeX, int windowSizeY)
{
	Initialize(windowSizeX, windowSizeY);
}


Renderer::~Renderer()
{
}

void Renderer::Initialize(int windowSizeX, int windowSizeY)
{
	//Set window size
	m_WindowSizeX = windowSizeX;
	m_WindowSizeY = windowSizeY;

	//Load shaders
	m_SolidRectShader = CompileShaders("./Shaders/SolidRect.vs", "./Shaders/SolidRect.fs");
	m_TriangleShader = CompileShaders("./Shaders/Triangle.vs", "./Shaders/Triangle.fs");
	m_FSShader = CompileShaders("./Shaders/FS.vs", "./Shaders/FS.fs");
	m_DummyShader = CompileShaders("./Shaders/Dummy.vs", "./Shaders/Dummy.fs");
	m_TextureShader = CompileShaders("./Shaders/Texture.vs", "./Shaders/Texture.fs");
	m_BlurH_Shader = CompileShaders("./Shaders/Bloom.vs", "./Shaders/BloomH.fs");
	m_BlurV_Shader = CompileShaders("./Shaders/Bloom.vs", "./Shaders/BloomV.fs");
	m_AccumShader = CompileShaders("./Shaders/Accum.vs", "./Shaders/Accum.fs");
	
	// Load Texture
	m_RgbTexture = CreatePngTexture("./Textures/rgb.png", GL_NEAREST);
	m_NumsTexture = CreatePngTexture("./Textures/numbers.png", GL_NEAREST);
	m_ParticleTexture = CreatePngTexture("./Textures/particle.png", GL_NEAREST);
	m_ParticleSpriteTexture = CreatePngTexture("./Textures/explosion.png", GL_NEAREST);
	m_BaeTexture = CreatePngTexture("./Textures/bae.png", GL_NEAREST);
	for (int i = 0; i < 10; ++i) {
		std::string path{ "./Textures/" + std::to_string(i) + ".png" };
		m_NumTexture[i] = CreatePngTexture((char*)path.c_str(), GL_NEAREST);
	}

	//Create VBOs
	CreateVertexBufferObjects();

	// Create Dummy
	GenDummyMesh(200, 200);

	GenFBOs();

	if (m_SolidRectShader > 0 && m_VBORect > 0)
	{
		m_Initialized = true;
	}
}

void Renderer::GenFBOs()
{
	for (int i = 0; i < 3; ++i) {
		// Gen Texture
		GLuint textureId; 
		glGenTextures(1, &m_FBO_Texture[i]);
		glBindTexture(GL_TEXTURE_2D, m_FBO_Texture[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		// Gen Render buffer
		GLuint depthBuffer;
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// GenFBO
		glGenFramebuffers(1, &m_FBO[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FBO_Texture[i], 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			assert(0);
		}
	}

	// MRT
	// Gen Texture
	for (int i = 0; i < 3; ++i) {
		GLuint textureId;
		glGenTextures(1, &m_MRT_FBO_Texture[i]);
		glBindTexture(GL_TEXTURE_2D, m_MRT_FBO_Texture[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}

	{
		// Gen Render buffer
		GLuint MRTdepthBuffer;
		glGenRenderbuffers(1, &MRTdepthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, MRTdepthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// GenFBO
		glGenFramebuffers(1, &m_MRT_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_MRT_FBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_MRT_FBO_Texture[0], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_MRT_FBO_Texture[1], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_MRT_FBO_Texture[2], 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, MRTdepthBuffer);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			assert(0);
		}
	}
	
	{
		// MRT_HDR
		glGenTextures(1, &m_MRT_HDR_FBO_High_Texture);
		glBindTexture(GL_TEXTURE_2D, m_MRT_HDR_FBO_High_Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, NULL);

		glGenTextures(1, &m_MRT_HDR_FBO_Low_Texture);
		glBindTexture(GL_TEXTURE_2D, m_MRT_HDR_FBO_Low_Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, NULL);

		glGenFramebuffers(1, &m_MRT_HDR_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_MRT_HDR_FBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_MRT_HDR_FBO_Low_Texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_MRT_HDR_FBO_High_Texture, 0);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			assert(0);
		}
	}

	glGenFramebuffers(2, m_PingpongFBO);
	glGenTextures(2, m_PingpongTexture);

	for (int i = 0; i < 2; ++i) {
		glBindTexture(GL_TEXTURE_2D, m_PingpongTexture[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindFramebuffer(GL_FRAMEBUFFER, m_PingpongFBO[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PingpongTexture[i], 0);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			assert(0);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint Renderer::CreatePngTexture(char* filePath, GLuint samplingMethod)
{
	//Load Png
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filePath);
	if (error != 0) {
		std::cout << "PNG image loading failed:" << filePath << std::endl;
		assert(0);
	}

	GLuint temp;
	glGenTextures(1, &temp);
	glBindTexture(GL_TEXTURE_2D, temp);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, &image[0]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, samplingMethod);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, samplingMethod);

	return temp;
}

bool Renderer::IsInitialized()
{
	return m_Initialized;
}

void Renderer::CreateVertexBufferObjects()
{
	float rect[]
		=
	{
		-1.f / m_WindowSizeX, -1.f / m_WindowSizeY, 0.f, -1.f / m_WindowSizeX, 1.f / m_WindowSizeY, 0.f, 1.f / m_WindowSizeX, 1.f / m_WindowSizeY, 0.f, //Triangle1
		-1.f / m_WindowSizeX, -1.f / m_WindowSizeY, 0.f,  1.f / m_WindowSizeX, 1.f / m_WindowSizeY, 0.f, 1.f / m_WindowSizeX, -1.f / m_WindowSizeY, 0.f, //Triangle2
	};

	glGenBuffers(1, &m_VBORect);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);


	float centerX = 0;
	float centerY = 0;
	float size = 0.1f;
	float halfSize = size / 2;
	float mass = 1.0f;
	float vx, vy;
	vx = 1;
	vy = 3;

	float triangle[] = {
		centerX - halfSize, centerY - halfSize, 0, mass, vx, vy,
		centerX + halfSize, centerY - halfSize, 0, mass, vx, vy,
		centerX + halfSize, centerY + halfSize, 0, mass, vx, vy,

		centerX - halfSize, centerY - halfSize, 0, mass, vx, vy,
		centerX + halfSize, centerY + halfSize, 0, mass, vx, vy,
		centerX - halfSize, centerY + halfSize, 0, mass, vx, vy,
	};
	glGenBuffers(1, &m_VBOTriangle);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOTriangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

	m_ParticleCount = 1000;
	CreateParticle(m_ParticleCount);

	float ndcRect[] = {
		-1, -1, 0, 0, 1,
		1, 1, 0, 1, 0,
		-1, 1, 0, 0, 0,

		-1, -1, 0, 0, 1,
		1, -1, 0, 1, 1,
		1, 1, 0,  1, 0
	};
	glGenBuffers(1, &m_VBOFS);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOFS);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ndcRect), ndcRect, GL_STATIC_DRAW);

	// GenDrop info
	int index{};
	for (int i = 0; i < 1000; ++i) {
		float x = ((float)rand() / (float)RAND_MAX);
		float y = ((float)rand() / (float)RAND_MAX);
		float sTime = 3 * ((float)rand() / (float)RAND_MAX);
		float lTime = ((float)rand() / (float)RAND_MAX);

		m_DropPoints[index++] = x;
		m_DropPoints[index++] = y;
		m_DropPoints[index++] = sTime;
		m_DropPoints[index++] = lTime;
	}

	float texRect[]
		=
	{
		-1.f, -1.f , 0.f, 
		-1.f , 1.f , 0.f, 
		1.f , 1.f , 0.f, //Triangle1
		
		-1.f, -1.f , 0.f,  
		1.f , 1.f , 0.f, 
		1.f , -1.f , 0.f, //Triangle2
	};

	glGenBuffers(1, &m_VBO_Texture);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Texture);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texRect), texRect, GL_STATIC_DRAW);

}

void Renderer::CreateParticle(const int num)
{
	std::vector<float> vertices;

	float centerX = 0;
	float centerY = 0;
	float size = 0.05f;
	float halfSize = size / 2;
	float mass = 1.0f;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	std::uniform_real_distribution<float> rdist(0.0f, 1.0f);
	
	for (int i = 0; i < num; ++i) {
		float vx = dist(gen);
		float vy = dist(gen);

		float RV = rdist(gen);
		float RV1 = rdist(gen);
		float RV2 = rdist(gen); // 시험 atrribute 추가
		float r = rdist(gen);
		float g = rdist(gen);
		float b = rdist(gen);

		float quad[] = {
			centerX - halfSize, centerY - halfSize, 0, mass, vx, vy, RV, RV1, RV2, 0.0f, 1.0f, r, g, b,
			centerX + halfSize, centerY - halfSize, 0, mass, vx, vy, RV, RV1, RV2, 1.0f, 1.0f, r, g, b,
			centerX + halfSize, centerY + halfSize, 0, mass, vx, vy, RV, RV1, RV2, 1.0f, 0.0f, r, g, b,

			centerX - halfSize, centerY - halfSize, 0, mass, vx, vy, RV, RV1, RV2, 0.0f, 1.0f, r, g, b,
			centerX + halfSize, centerY + halfSize, 0, mass, vx, vy, RV, RV1, RV2, 1.0f, 0.0f, r, g, b,
			centerX - halfSize, centerY + halfSize, 0, mass, vx, vy, RV, RV1, RV2, 0.0f, 0.0f, r, g, b,
		};

		vertices.insert(vertices.end(), std::begin(quad), std::end(quad));
	}
	
	glGenBuffers(1, &m_VBOParticle);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOParticle);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
}

void Renderer::GenDummyMesh(int resolX, int resolY)
{
	float basePosX = -0.5f;
	float basePosY = -0.5f;
	float targetPosX = 0.5f;
	float targetPosY = 0.5f;
	int pointCountX = resolX;
	int pointCountY = resolY;

	float width = targetPosX - basePosX;
	float height = targetPosY - basePosY;

	float* point = new float[pointCountX * pointCountY * 2];
	float* vertices = new float[(pointCountX - 1) * (pointCountY - 1) * 2 * 3 * 3];

	m_DummyVertexCount = (pointCountX - 1) * (pointCountY - 1) * 2 * 3;

	//Prepare points
	for (int x = 0; x < pointCountX; x++)
	{
		for (int y = 0; y < pointCountY; y++)
		{
			point[(y * pointCountX + x) * 2 + 0] = basePosX + width * (x / (float)(pointCountX - 1));
			point[(y * pointCountX + x) * 2 + 1] = basePosY + height * (y / (float)(pointCountY - 1));
		}
	}

	//Make triangles
	int vertIndex = 0;
	for (int x = 0; x < pointCountX - 1; x++)
	{
		for (int y = 0; y < pointCountY - 1; y++)
		{
			//Triangle part 1
			vertices[vertIndex] = point[(y * pointCountX + x) * 2 + 0];
			vertIndex++;
			vertices[vertIndex] = point[(y * pointCountX + x) * 2 + 1];
			vertIndex++;
			vertices[vertIndex] = 0.f;
			vertIndex++;
			vertices[vertIndex] = point[((y + 1) * pointCountX + (x + 1)) * 2 + 0];
			vertIndex++;
			vertices[vertIndex] = point[((y + 1) * pointCountX + (x + 1)) * 2 + 1];
			vertIndex++;
			vertices[vertIndex] = 0.f;
			vertIndex++;
			vertices[vertIndex] = point[((y + 1) * pointCountX + x) * 2 + 0];
			vertIndex++;
			vertices[vertIndex] = point[((y + 1) * pointCountX + x) * 2 + 1];
			vertIndex++;
			vertices[vertIndex] = 0.f;
			vertIndex++;

			//Triangle part 2
			vertices[vertIndex] = point[(y * pointCountX + x) * 2 + 0];
			vertIndex++;
			vertices[vertIndex] = point[(y * pointCountX + x) * 2 + 1];
			vertIndex++;
			vertices[vertIndex] = 0.f;
			vertIndex++;
			vertices[vertIndex] = point[(y * pointCountX + (x + 1)) * 2 + 0];
			vertIndex++;
			vertices[vertIndex] = point[(y * pointCountX + (x + 1)) * 2 + 1];
			vertIndex++;
			vertices[vertIndex] = 0.f;
			vertIndex++;
			vertices[vertIndex] = point[((y + 1) * pointCountX + (x + 1)) * 2 + 0];
			vertIndex++;
			vertices[vertIndex] = point[((y + 1) * pointCountX + (x + 1)) * 2 + 1];
			vertIndex++;
			vertices[vertIndex] = 0.f;
			vertIndex++;
		}
	}

	glGenBuffers(1, &m_VBO_DummyMesh);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_DummyMesh);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (pointCountX - 1) * (pointCountY - 1) * 2 * 3 * 3, vertices, GL_STATIC_DRAW);
}


void Renderer::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	//쉐이더 오브젝트 생성
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	//쉐이더 코드를 쉐이더 오브젝트에 할당
	glShaderSource(ShaderObj, 1, p, Lengths);

	//할당된 쉐이더 코드를 컴파일
	glCompileShader(ShaderObj);

	GLint success;
	// ShaderObj 가 성공적으로 컴파일 되었는지 확인
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];

		//OpenGL 의 shader log 데이터를 가져옴
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		printf("%s \n", pShaderText);
	}

	// ShaderProgram 에 attach!!
	glAttachShader(ShaderProgram, ShaderObj);
}

bool Renderer::ReadFile(char* filename, std::string *target)
{
	std::ifstream file(filename);
	if (file.fail())
	{
		std::cout << filename << " file loading failed.. \n";
		file.close();
		return false;
	}
	std::string line;
	while (getline(file, line)) {
		target->append(line.c_str());
		target->append("\n");
	}
	return true;
}

GLuint Renderer::CompileShaders(char* filenameVS, char* filenameFS)
{
	GLuint ShaderProgram = glCreateProgram(); //빈 쉐이더 프로그램 생성

	if (ShaderProgram == 0) { //쉐이더 프로그램이 만들어졌는지 확인
		fprintf(stderr, "Error creating shader program\n");
	}

	std::string vs, fs;

	//shader.vs 가 vs 안으로 로딩됨
	if (!ReadFile(filenameVS, &vs)) {
		printf("Error compiling vertex shader\n");
		return -1;
	};

	//shader.fs 가 fs 안으로 로딩됨
	if (!ReadFile(filenameFS, &fs)) {
		printf("Error compiling fragment shader\n");
		return -1;
	};

	// ShaderProgram 에 vs.c_str() 버텍스 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	// ShaderProgram 에 fs.c_str() 프레그먼트 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	//Attach 완료된 shaderProgram 을 링킹함
	glLinkProgram(ShaderProgram);

	//링크가 성공했는지 확인
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (Success == 0) {
		// shader program 로그를 받아옴
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error linking shader program\n" << ErrorLog;
		return -1;
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error validating shader program\n" << ErrorLog;
		return -1;
	}

	glUseProgram(ShaderProgram);
	std::cout << filenameVS << ", " << filenameFS << " Shader compiling is done.";

	return ShaderProgram;
}

void Renderer::DrawSolidRect(float x, float y, float z, float size, float r, float g, float b, float a)
{
	float newX, newY;

	GetGLPosition(x, y, &newX, &newY);

	//Program select
	glUseProgram(m_SolidRectShader);

	glUniform4f(glGetUniformLocation(m_SolidRectShader, "u_Trans"), newX, newY, 0, size);
	glUniform4f(glGetUniformLocation(m_SolidRectShader, "u_Color"), r, g, b, a);

	int attribPosition = glGetAttribLocation(m_SolidRectShader, "a_Position");
	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
}

float g_time{};

void Renderer::DrawTriangle()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	g_time += 0.005f;
	//Program select
	glUseProgram(m_TriangleShader);

	int uTime = glGetUniformLocation(m_TriangleShader, "u_Time");
	glUniform1f(uTime, g_time);

	int uParticle = glGetUniformLocation(m_TriangleShader, "u_ParticleTex");
	glUniform1i(uParticle, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ParticleTexture);

	int uParticleSprite = glGetUniformLocation(m_TriangleShader, "u_ParticleSpriteTex");
	glUniform1i(uParticleSprite, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_ParticleSpriteTexture);

	int attribPosition = glGetAttribLocation(m_TriangleShader, "a_Position");
	int attribMass = glGetAttribLocation(m_TriangleShader, "a_Mass");
	int attribVel = glGetAttribLocation(m_TriangleShader, "a_Vel");
	int attribRV = glGetAttribLocation(m_TriangleShader, "a_RV");
	int attribRV1 = glGetAttribLocation(m_TriangleShader, "a_RV1");
	int attribRV2 = glGetAttribLocation(m_TriangleShader, "a_RV2");
	int attribTex = glGetAttribLocation(m_TriangleShader, "a_Tex");
	int attribRGB = glGetAttribLocation(m_TriangleShader, "a_RGB");
	glEnableVertexAttribArray(attribPosition);
	glEnableVertexAttribArray(attribMass);
	glEnableVertexAttribArray(attribVel);
	glEnableVertexAttribArray(attribRV);
	glEnableVertexAttribArray(attribRV1);
	glEnableVertexAttribArray(attribRV2);
	glEnableVertexAttribArray(attribTex);
	glEnableVertexAttribArray(attribRGB);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOParticle);

	unsigned int stride{ sizeof(float) * 14 };
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);
	glVertexAttribPointer(attribMass, 1, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(float) * 3));
	glVertexAttribPointer(attribVel, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(float) * 4));
	glVertexAttribPointer(attribRV, 1, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(float) * 6));
	glVertexAttribPointer(attribRV1, 1, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(float) * 7));
	glVertexAttribPointer(attribRV2, 1, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(float) * 8));
	glVertexAttribPointer(attribTex, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(float) * 9));
	glVertexAttribPointer(attribRGB, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(float) * 11));

	glDrawArrays(GL_TRIANGLES, 0, m_ParticleCount * 6);

	glDisable(GL_BLEND);
}

int g_CurrNum = 0;

void Renderer::DrawFS()
{
	g_time += 0.005f;
	//Program select
	GLuint shader = m_FSShader;
	glUseProgram(shader);

	int uTime = glGetUniformLocation(shader, "u_Time");
	glUniform1f(uTime, g_time);

	int uPoints = glGetUniformLocation(shader, "u_DropInfo"); 
	glUniform4fv(uPoints, 1000, m_DropPoints);

	// texture uniform
	int uRGBTex = glGetUniformLocation(shader, "u_RGBTex");
	glUniform1i(uRGBTex, 0);
	int uCurrNumTex = glGetUniformLocation(shader, "u_CurrNumTex");
	glUniform1i(uCurrNumTex, 2 + (g_CurrNum % 10));

	int uNumsTex = glGetUniformLocation(shader, "u_NumsTex");
	glUniform1i(uNumsTex, 1);
	int uInputNum = glGetUniformLocation(shader, "u_InputNum");
	glUniform1i(uInputNum, g_CurrNum++);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_RgbTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_NumsTexture);
	for (int i = 0; i < 10; ++i) {
		glActiveTexture(GL_TEXTURE2 + i);
		glBindTexture(GL_TEXTURE_2D, m_NumTexture[i]);
	}

	int attribPosition = glGetAttribLocation(shader, "a_Pos");
	int attribtPos = glGetAttribLocation(shader, "a_tPos");
	glEnableVertexAttribArray(attribPosition);
	glEnableVertexAttribArray(attribtPos);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOFS);

	unsigned int stride{ sizeof(float) * 5 };
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);
	glVertexAttribPointer(attribtPos, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(float) * 3));

	glDrawArrays(GL_TRIANGLES, 0, 6);

}

void Renderer::DrawDummy()
{
	g_time += 0.005f;

	glUseProgram(m_DummyShader);

	int uTime = glGetUniformLocation(m_DummyShader, "u_Time");
	glUniform1f(uTime, g_time);

	int uBaeTex = glGetUniformLocation(m_DummyShader, "u_BaeTex");
	glUniform1i(uBaeTex, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_BaeTexture);

	int uPoints = glGetUniformLocation(m_DummyShader, "u_DropInfo");
	glUniform4fv(uPoints, 1000, m_DropPoints);

	int attribPosition = glGetAttribLocation(m_DummyShader, "a_Pos");
	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_DummyMesh);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	glDrawArrays(GL_TRIANGLES, 0, m_DummyVertexCount);

	glDisableVertexAttribArray(attribPosition);

	DrawTexture(m_RgbTexture, 0, 0, 0.1, true);
}

void Renderer::DrawDummy_FBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO[0]);
	glViewport(0, 0, 512, 512); 
	DrawDummy();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 1024, 1024);
	DrawTexture(m_FBO_Texture[0], 0, 0, 0.5, false);
}

void Renderer::DrawAll_FBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO[0]);
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 512, 512);
	DrawFS();

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO[1]);
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 512, 512);
	DrawTriangle();

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO[2]);
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 512, 512);
	DrawDummy();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 1024, 1024);
	DrawTexture(m_FBO_Texture[0], -0.5, 0, 0.3, false);
	DrawTexture(m_FBO_Texture[1], 0.0, 0, 0.3, false);
	DrawTexture(m_FBO_Texture[2], 0.5, 0, 0.3, false);
}

void Renderer::DrawMultipleRenderTarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_MRT_FBO);
	GLenum DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, DrawBuffers);
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 512, 512);

	DrawFS();
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 1024, 1024);
	GLenum ResetDrawBuffers[1] = { GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, ResetDrawBuffers);

	DrawTexture(m_MRT_FBO_Texture[0], -0.5, 0, 0.3, false);
	DrawTexture(m_MRT_FBO_Texture[1], 0.0, 0, 0.3, false);
	DrawTexture(m_MRT_FBO_Texture[2], 0.5, 0, 0.3, false);
}

void Renderer::DrawTriangle_Bloom()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_MRT_HDR_FBO);
	GLenum DrawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, DrawBuffers);
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 1024, 1024);

	DrawTriangle();

	DrawGaussianBlur(m_MRT_HDR_FBO_High_Texture, m_PingpongFBO[0], m_BlurH_Shader);
	for (int i = 0; i < 50; i++)
	{
		DrawGaussianBlur(m_PingpongTexture[0], m_PingpongFBO[1], m_BlurV_Shader);
		DrawGaussianBlur(m_PingpongTexture[1], m_PingpongFBO[0], m_BlurH_Shader);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 1024, 1024);
	GLenum ResetDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, ResetDrawBuffers);

	DrawAccumResult(m_MRT_HDR_FBO_Low_Texture, m_PingpongTexture[0], false);

	DrawTexture(m_MRT_HDR_FBO_Low_Texture, -0.5, -0.8, 0.2, false);
	//DrawTexture(m_MRT_HDR_FBO_High_Texture, 0.5, 0.5, 0.5, false);

	DrawTexture(m_PingpongTexture[0], 0.5, -0.8, 0.2, true);
	//DrawTexture(m_PingpongTexture[1], 0.5, -0.5, 0.5, false);
}

void Renderer::DrawGaussianBlur(GLuint texID, GLuint targetFBOID, GLuint shader)
{
	glBindFramebuffer(GL_FRAMEBUFFER, targetFBOID);
	glUseProgram(shader);

	GLuint posLoc = glGetAttribLocation(shader, "a_Pos");
	glEnableVertexAttribArray(posLoc);
	GLuint texLoc = glGetAttribLocation(shader, "a_Tex");
	glEnableVertexAttribArray(texLoc);
	glUniform1i(glGetUniformLocation(shader, "u_Texture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBOFS);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
	glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (GLvoid*)(sizeof(float) * 3));
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawAccumResult(GLuint texOri, GLuint texBlurred, bool flip)
{
	int shader = m_AccumShader;
	glUseProgram(shader);

	int uFlip = glGetUniformLocation(shader, "u_Flip");
	glUniform1i(uFlip, flip);
	int uTex = glGetUniformLocation(shader, "u_Tex");
	glUniform1i(uTex, 0);
	int uTexBlurred = glGetUniformLocation(shader, "u_TexBlurred");
	glUniform1i(uTexBlurred, 1);
	int uExposure = glGetUniformLocation(shader, "u_Exposure");
	glUniform1i(uExposure, 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texOri);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texBlurred);

	int aPos = glGetAttribLocation(shader, "a_Pos");
	glEnableVertexAttribArray(aPos);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Texture);
	glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::DrawTexture(GLuint texID, float x, float y, float scale, bool flip)
{
	//Program select
	glUseProgram(m_TextureShader);

	int uTex = glGetUniformLocation(m_TextureShader, "u_Tex");
	glUniform1i(uTex, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);

	int uTrans = glGetUniformLocation(m_TextureShader, "u_Trans");
	glUniform4f(uTrans, x, y, 1, scale);

	int uFlip = glGetUniformLocation(m_TextureShader, "u_Flip");
	glUniform1i(uFlip, flip);

	int attribPosition = glGetAttribLocation(m_TextureShader, "a_Pos");
	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Texture);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
}

void Renderer::GetGLPosition(float x, float y, float *newX, float *newY)
{
	*newX = x * 2.f / m_WindowSizeX;
	*newY = y * 2.f / m_WindowSizeY;
}