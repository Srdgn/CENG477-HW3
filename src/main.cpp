#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <ft2build.h>
#include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint gProgram[2];
int gWidth, gHeight;

GLint modelingMatrixLoc[2];
GLint viewingMatrixLoc[2];
GLint projectionMatrixLoc[2];
GLint eyePosLoc[2];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);

int activeProgramIndex = 0;

int count_checkpoint = 0;
int score = 0;
bool rotationGameOver = false;
bool rotationCheckPoint = false;
glm::vec3 cameraPosition(0.0f,0.0f,0.0f);

glm::vec3 starting_position(0,-5,-10);
glm::vec3 position = starting_position;
glm::vec3 starting_positionBunny(0.0f,1.0f,0.0f);
glm::vec3 positionBunny = starting_positionBunny;
float starting_speed = 0.1;
float speed = starting_speed;
float starting_acceleration = 0.0001;
float acceleration = starting_acceleration;

struct Vertex
{
	Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Texture
{
	Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
	GLfloat u, v;
};

struct Normal
{
	Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
	GLuint vIndex[3], tIndex[3], nIndex[3];
};

vector<Vertex> gVertices1;
vector<Texture> gTextures1;
vector<Normal> gNormals1;
vector<Face> gFaces1;

vector<Vertex> gVertices2;
vector<Texture> gTextures2;
vector<Normal> gNormals2;
vector<Face> gFaces2;

GLuint gVertexAttribBuffer1, gIndexBuffer1;
GLint gInVertexLoc1, gInNormalLoc1;
int gVertexDataSizeInBytes1, gNormalDataSizeInBytes1;

GLuint gVertexAttribBuffer2, gIndexBuffer2;
GLint gInVertexLoc2, gInNormalLoc2;
int gVertexDataSizeInBytes2, gNormalDataSizeInBytes2;

bool ParseObj(const string& fileName,vector<Vertex> *gVertices,vector<Texture> *gTextures, vector<Normal> *gNormals, vector<Face> *gFaces)
{
	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			stringstream str(curLine);
			GLfloat c1, c2, c3;
			GLuint index[9];
			string tmp;

			if (curLine.length() >= 2)
			{
				if (curLine[0] == 'v')
				{
					if (curLine[1] == 't') // texture
					{
						str >> tmp; // consume "vt"
						str >> c1 >> c2;
						gTextures->push_back(Texture(c1, c2));
					}
					else if (curLine[1] == 'n') // normal
					{
						str >> tmp; // consume "vn"
						str >> c1 >> c2 >> c3;
						gNormals->push_back(Normal(c1, c2, c3));
					}
					else // vertex
					{
						str >> tmp; // consume "v"
						str >> c1 >> c2 >> c3;
						gVertices->push_back(Vertex(c1, c2, c3));
					}
				}
				else if (curLine[0] == 'f') // face
				{
					str >> tmp; // consume "f"
					char c;
					int vIndex[3], nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0];
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1];
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2];

					assert(vIndex[0] == nIndex[0] &&
						vIndex[1] == nIndex[1] &&
						vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

					gFaces->push_back(Face(vIndex, tIndex, nIndex));
				}
				else
				{
					cout << "Ignoring unidentified line in obj file: " << curLine << endl;
				}
			}

			//data += curLine;
			if (!myfile.eof())
			{
				//data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	/*
	for (int i = 0; i < gVertices.size(); ++i)
	{
		Vector3 n;

		for (int j = 0; j < gFaces.size(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (gFaces[j].vIndex[k] == i)
				{
					// face j contains vertex i
					Vector3 a(gVertices[gFaces[j].vIndex[0]].x,
							  gVertices[gFaces[j].vIndex[0]].y,
							  gVertices[gFaces[j].vIndex[0]].z);

					Vector3 b(gVertices[gFaces[j].vIndex[1]].x,
							  gVertices[gFaces[j].vIndex[1]].y,
							  gVertices[gFaces[j].vIndex[1]].z);

					Vector3 c(gVertices[gFaces[j].vIndex[2]].x,
							  gVertices[gFaces[j].vIndex[2]].y,
							  gVertices[gFaces[j].vIndex[2]].z);

					Vector3 ab = b - a;
					Vector3 ac = c - a;
					Vector3 normalFromThisFace = (ab.cross(ac)).getNormalized();
					n += normalFromThisFace;
				}

			}
		}

		n.normalize();

		gNormals.push_back(Normal(n.x, n.y, n.z));
	}
	*/

	assert(gVertices->size() == gNormals->size());

	return true;
}

bool ReadDataFromFile(
	const string& fileName, ///< [in]  Name of the shader file
	string& data)     ///< [out] The contents of the file
{
	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			data += curLine;
			if (!myfile.eof())
			{
				data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	return true;
}

GLuint createVS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shader, &length);
	glCompileShader(vs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(vs, 1024, &length, output);
	printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shader, &length);
	glCompileShader(fs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(fs, 1024, &length, output);
	printf("FS compile log: %s\n", output);

	return fs;
}

void initShaders()
{
	// Create the programs

	gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();

	// Create the shaders for both programs

	GLuint vs1 = createVS("vert.glsl");
	GLuint fs1 = createFS("frag.glsl");

	GLuint vs2 = createVS("vert2.glsl");
	GLuint fs2 = createFS("frag2.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	// Link the programs

	glLinkProgram(gProgram[0]);
	GLint status;
	glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[1]);
	glGetProgramiv(gProgram[1], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 2; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}
}

FT_Library ft;
FT_Face ft_face;

void initFreeType() {
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "Error: Could not initialize FreeType library" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (FT_New_Face(ft, "/usr/share/fonts/truetype/msttcorefonts/Arial_Bold.ttf", 0, &ft_face)) {
        std::cerr << "Error: Could not load font" << std::endl;
        exit(EXIT_FAILURE);
    }

    FT_Set_Pixel_Sizes(ft_face, 0, 48);
}
void renderText(const char *text, float x, float y) {
    glRasterPos2f(x, y);

    FT_GlyphSlot g = ft_face->glyph;

    for (const char *p = text; *p; p++) {
        if (FT_Load_Char(ft_face, *p, FT_LOAD_RENDER))
            continue;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g->bitmap.width, g->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

        float x2 = x + g->bitmap_left;
        float y2 = -y - g->bitmap_top;
        float w = g->bitmap.width;
        float h = g->bitmap.rows;

        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(x2, y2);
        glTexCoord2f(0, 1);
        glVertex2f(x2, y2 - h);
        glTexCoord2f(1, 1);
        glVertex2f(x2 + w, y2 - h);
        glTexCoord2f(1, 0);
        glVertex2f(x2 + w, y2);
        glEnd();
        x += (g->advance.x >> 6);
        y += (g->advance.y >> 6);
    }
}

void initVBO2()
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);
    cout << "vao = " << vao << endl;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBuffer2);
    glGenBuffers(1, &gIndexBuffer2);

    assert(gVertexAttribBuffer2 > 0 && gIndexBuffer2 > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer2);

    gVertexDataSizeInBytes2 = gVertices2.size() * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytes2 = gNormals2.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces2.size() * 3 * sizeof(GLuint);
    GLfloat* vertexData2 = new GLfloat[gVertices2.size() * 3];
    GLfloat* normalData2 = new GLfloat[gNormals2.size() * 3];
    GLuint* indexData2 = new GLuint[gFaces2.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < gVertices2.size(); ++i)
    {
        vertexData2[3 * i] = gVertices2[i].x;
        vertexData2[3 * i + 1] = gVertices2[i].y;
        vertexData2[3 * i + 2] = gVertices2[i].z;

        minX = std::min(minX, gVertices2[i].x);
        maxX = std::max(maxX, gVertices2[i].x);
        minY = std::min(minY, gVertices2[i].y);
        maxY = std::max(maxY, gVertices2[i].y);
        minZ = std::min(minZ, gVertices2[i].z);
        maxZ = std::max(maxZ, gVertices2[i].z);
    }

    std::cout << "minX = " << minX << std::endl;
    std::cout << "maxX = " << maxX << std::endl;
    std::cout << "minY = " << minY << std::endl;
    std::cout << "maxY = " << maxY << std::endl;
    std::cout << "minZ = " << minZ << std::endl;
    std::cout << "maxZ = " << maxZ << std::endl;

    for (int i = 0; i < gNormals2.size(); ++i)
    {
        normalData2[3 * i] = gNormals2[i].x;
        normalData2[3 * i + 1] = gNormals2[i].y;
        normalData2[3 * i + 2] = gNormals2[i].z;
    }

    for (int i = 0; i < gFaces2.size(); ++i)
    {
        indexData2[3 * i] = gFaces2[i].vIndex[0];
        indexData2[3 * i + 1] = gFaces2[i].vIndex[1];
        indexData2[3 * i + 2] = gFaces2[i].vIndex[2];
    }

    glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes2 + gNormalDataSizeInBytes2, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes2, vertexData2);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes2, gNormalDataSizeInBytes2, normalData2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData2, GL_STATIC_DRAW);

    // done copying to GPU memory; can free now from CPU memory
    delete[] vertexData2;
    delete[] normalData2;
    delete[] indexData2;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes2));
}
void initVBO1()
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	assert(vao > 0);
	glBindVertexArray(vao);
	cout << "vao = " << vao << endl;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &gVertexAttribBuffer1);
	glGenBuffers(1, &gIndexBuffer1);

	assert(gVertexAttribBuffer1 > 0 && gIndexBuffer1 > 0);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer1);

	gVertexDataSizeInBytes1 = gVertices1.size() * 3 * sizeof(GLfloat);
	gNormalDataSizeInBytes1 = gNormals1.size() * 3 * sizeof(GLfloat);
	int indexDataSizeInBytes = gFaces1.size() * 3 * sizeof(GLuint);
	GLfloat* vertexData1 = new GLfloat[gVertices1.size() * 3];
	GLfloat* normalData1 = new GLfloat[gNormals1.size() * 3];
	GLuint* indexData1 = new GLuint[gFaces1.size() * 3];

	float minX = 1e6, maxX = -1e6;
	float minY = 1e6, maxY = -1e6;
	float minZ = 1e6, maxZ = -1e6;

	for (int i = 0; i < gVertices1.size(); ++i)
	{
		vertexData1[3 * i] = gVertices1[i].x;
		vertexData1[3 * i + 1] = gVertices1[i].y;
		vertexData1[3 * i + 2] = gVertices1[i].z;

		minX = std::min(minX, gVertices1[i].x);
		maxX = std::max(maxX, gVertices1[i].x);
		minY = std::min(minY, gVertices1[i].y);
		maxY = std::max(maxY, gVertices1[i].y);
		minZ = std::min(minZ, gVertices1[i].z);
		maxZ = std::max(maxZ, gVertices1[i].z);
	}

	std::cout << "minX = " << minX << std::endl;
	std::cout << "maxX = " << maxX << std::endl;
	std::cout << "minY = " << minY << std::endl;
	std::cout << "maxY = " << maxY << std::endl;
	std::cout << "minZ = " << minZ << std::endl;
	std::cout << "maxZ = " << maxZ << std::endl;

	for (int i = 0; i < gNormals1.size(); ++i)
	{
		normalData1[3 * i] = gNormals1[i].x;
		normalData1[3 * i + 1] = gNormals1[i].y;
		normalData1[3 * i + 2] = gNormals1[i].z;
	}

	for (int i = 0; i < gFaces1.size(); ++i)
	{
		indexData1[3 * i] = gFaces1[i].vIndex[0];
		indexData1[3 * i + 1] = gFaces1[i].vIndex[1];
		indexData1[3 * i + 2] = gFaces1[i].vIndex[2];
	}


	glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes1 + gNormalDataSizeInBytes1, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes1, vertexData1);
	glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes1, gNormalDataSizeInBytes1, normalData1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData1, GL_STATIC_DRAW);

	// done copying to GPU memory; can free now from CPU memory
	delete[] vertexData1;
	delete[] normalData1;
	delete[] indexData1;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes1));
}

void init()
{
	ParseObj("quad.obj",  &gVertices2, &gTextures2, &gNormals2, &gFaces2);
	ParseObj("bunny.obj", &gVertices1, &gTextures1, &gNormals1, &gFaces1);



	glEnable(GL_DEPTH_TEST);
	initShaders();
	
    initVBO2();
    initVBO1();
    initFreeType();
}


void drawBunny()
{
    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes1));
	glDrawElements(GL_TRIANGLES, gFaces1.size() * 3, GL_UNSIGNED_INT, 0);
}

void drawGround()
{
    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes2));
	glDrawElements(GL_TRIANGLES, gFaces2.size() * 3, GL_UNSIGNED_INT, 0);
}



void displayBunny()
{
    activeProgramIndex = 0;
    static float angle = 0;

	float angleRad = (float)(angle / 180.0) * M_PI;

	// Compute the modeling matrix 
	glm::mat4 matT = glm::translate(glm::mat4(1.0), position+positionBunny);
	glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(0.5, 0.5, 0.5));
	glm::mat4 matR = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 matR2 = glm::rotate<float>(glm::mat4(1.0), (10. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	glm::mat4 matRz = glm::rotate(glm::mat4(1.0), angleRad, glm::vec3(0.0, 0.0, 1.0));
	modelingMatrix = matT * matRz * matR2* matR; // starting from right side, rotate around Y to turn back, then rotate around Z some more at each frame, then translate.

	// or... (care for the order! first the very bottom one is applied)
	//modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.f, 0.f, -3.f));
	//modelingMatrix = glm::rotate(modelingMatrix, angleRad, glm::vec3(0.0, 0.0, 1.0));
	//modelingMatrix = glm::rotate<float>(modelingMatrix, (-180. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram[activeProgramIndex]);
	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

    drawBunny();
}

void displayGround()
{
    activeProgramIndex = 1;
    static float angle = 0;

	float angleRad = (float)(angle / 180.0) * M_PI;

	// Compute the modeling matrix 
	glm::mat4 matT = glm::translate(glm::mat4(1.0), position);
	glm::mat4 matS = glm::scale(glm::mat4(1), glm::vec3(10, 5, 500));
	glm::mat4 matR = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	glm::mat4 matRz = glm::rotate(glm::mat4(1.0), angleRad, glm::vec3(0.0, 0.0, 1.0));
	modelingMatrix = matT * matS * matRz * matR; // starting from right side, rotate around Y to turn back, then rotate around Z some more at each frame, then translate.

	// or... (care for the order! first the very bottom one is applied)
	//modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.f, 0.f, -3.f));
	//modelingMatrix = glm::rotate(modelingMatrix, angleRad, glm::vec3(0.0, 0.0, 1.0));
	//modelingMatrix = glm::rotate<float>(modelingMatrix, (-180. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram[1]);
	glUniformMatrix4fv(projectionMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[1], 1, glm::value_ptr(eyePos));

    drawGround();
}

bool isHit(){
    return false;
}
bool isCheckPoint(){
    return false;
}

void gameOver()
{
    rotationGameOver = true;
    speed = 0;
    acceleration = 0;
}
void calculate_score(){
    score = int(-position.z + 1000*count_checkpoint);
	std::cout<<score<<std::endl;
}

void showScore()
{
    calculate_score();
    const std::string score_string = "Score: " + std::to_string(score);
    renderText("Test Text", 10, 10);

	//renderText(score_string.c_str(), 10, 10);
}

void display()
{

    if(isHit()) gameOver();
    else if(isCheckPoint()){
        count_checkpoint += 1;
        rotationCheckPoint = true;
    }
    glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    displayGround();
    displayBunny();
    showScore();

	// Draw the scene
	//drawModels();

}

void reshape(GLFWwindow* window, int w, int h)
{
	w = w < 1 ? 1 : w;
	h = h < 1 ? 1 : h;

	gWidth = w;
	gHeight = h;

	glViewport(0, 0, w, h);

	// Use perspective projection
	float fovyRad = (float)(90.0 / 180.0) * M_PI;
	projectionMatrix = glm::perspective(fovyRad, w / (float)h, 0.1f, 200.0f);

	// Assume default camera position and orientation (camera is at
	// (0, 0, 0) with looking at -z direction and its up vector pointing
	// at +y direction)
	// 
	//viewingMatrix = glm::mat4(1);
	viewingMatrix = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

}


void goLeft(){
    bool onGround = positionBunny.x > -7.5;
    if(onGround) positionBunny.x -= speed/2;
}
void goRight(){
    bool onGround = positionBunny.x < 7.5;
    if(onGround)positionBunny.x += speed/2;
}
void restart(){
    cameraPosition =glm::vec3(0.0f,0.0f,0.0f);
    viewingMatrix = glm::lookAt(cameraPosition, glm::vec3(0, 0, 0) + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    position = starting_position;
    speed = starting_speed;

    //rotation = 0; 
    positionBunny = starting_positionBunny;
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_A && (action == GLFW_REPEAT  ||action == GLFW_PRESS))
	{
		goLeft();
	}
	else if (key == GLFW_KEY_D && (action == GLFW_REPEAT  ||action == GLFW_PRESS))
	{
		goRight();
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		restart();
	}
	
}

double jump_value = 0.0f;
bool goingUp = true;
void jump()
{

    if(goingUp)
    {
        jump_value += speed;
        if(jump_value>=2){

            jump_value = 2;
            goingUp = false;
        }
    }
    else  //going down
    {
        jump_value -= speed;
        if(jump_value<=0){

            jump_value = 0;
            goingUp = true;
        }
    }

    
    positionBunny.y = starting_positionBunny.y+ jump_value;

}

void calculateNextValues()
{
    jump();
    cameraPosition.z -= speed;
    viewingMatrix = glm::lookAt(cameraPosition, glm::vec3(0, 0,position.z-200) + glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
    position.z -= speed;
    speed += acceleration;

}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		display();
        
        calculateNextValues();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
	GLFWwindow* window;
	if (!glfwInit())
	{
		exit(-1);
	}

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this if on MacOS

	int width = 1000, height = 800;
	window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	char rendererInfo[512] = { 0 };
	strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER)); // Use strcpy_s on Windows, strcpy on Linux
	strcat(rendererInfo, " - "); // Use strcpy_s on Windows, strcpy on Linux
	strcat(rendererInfo, (const char*)glGetString(GL_VERSION)); // Use strcpy_s on Windows, strcpy on Linux
	glfwSetWindowTitle(window, rendererInfo);

	init();

	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, reshape);

	reshape(window, width, height); // need to call this once ourselves
	mainLoop(window); // this does not return unless the window is closed

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
