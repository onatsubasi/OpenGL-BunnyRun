#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <GL/glew.h>   // The GL Header File
#include <GL/gl.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H




#include <random>

#define _USE_MATH_DEFINES
#define STB_IMAGE_IMPLEMENTATION
#include <math.h>

//#include <OpenGL/gl3.h>   // The GL Header File
#include "stb_image.h"


#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint gProgram[6];

GLint gIntensityLoc;
float gIntensity = 1000;
int gWidth = 640, gHeight = 480;
float maxDepth = 40.f; //Maximum distance from the bunny.
bool hit; // checks if the bunny is inside the yellow checkpoint
bool moveRight, moveLeft;
bool turning; // checks if the bunny is turning around (hit the yellow checkpoint.)
bool dead; // is bunny dead.
int yellowIndex;
int redIndex;
float depthOffset = 0.0;
float horizontalOffset = 0.0; // this is to control the movement of the bunny in x axis.
double gameSpeed = 0.15f; // The game's speed. Increases at each frame;
double speedIncrease = 0.0002;
bool randomize = true;
float score = 0; //score
float angle = 0;
float height = 0;
bool falling = false;
GLint modelingMatrixLoc[6];
GLint viewingMatrixLoc[6];
GLint projectionMatrixLoc[6];
GLint eyePosLoc[6];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);

int activeProgramIndex = 0;

// vector<Vertex> gVertices;
// vector<Texture> gTextures;
// vector<Normal> gNormals;
// vector<Face> gFaces;

GLuint gTextVBO;
GLuint vaoText;
GLint gInVertexLoc, gInNormalLoc;
// int gVertexDataSizeInBytes, gNormalDataSizeInBytes;

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

struct Mesh
{
	string tag;
	vector<Vertex> gVertices;
	vector<Texture> gTextures;
	vector<Normal> gNormals;
	vector<Face> gFaces;
	GLuint gVertexAttribBuffer, gIndexBuffer;
	int gVertexDataSizeInBytes, gNormalDataSizeInBytes;
};


vector<Mesh> meshes;

//GLint gInVertexLoc, gInNormalLoc;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;


bool ParseObj(const string& fileName, Mesh &mesh)
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
                if (curLine[0] == '#') // comment
                {
                    continue;
                }
                else if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        mesh.gTextures.push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        mesh.gNormals.push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        mesh.gVertices.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
					char c;
					int vIndex[3],  nIndex[3], tIndex[3];
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

                    mesh.gFaces.push_back(Face(vIndex, tIndex, nIndex));
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

	assert(mesh.gVertices.size() == mesh.gNormals.size());

    return true;
}

bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
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

void createVS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

    glAttachShader(program, vs);
}

void createFS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

    glAttachShader(program, fs);
}

void initShaders()
{
    gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();
	gProgram[2] = glCreateProgram();
	gProgram[3] = glCreateProgram();
	gProgram[4] = glCreateProgram();
	gProgram[5] = glCreateProgram();

    createVS(gProgram[0],"bunny_shader.glsl");
	createFS(gProgram[0],"frag.glsl");

	createVS(gProgram[1],"quad_v_shader.glsl");
	createFS(gProgram[1],"quad_f_shader.glsl");

 	createVS(gProgram[2],"cube_shader.glsl");
    createFS(gProgram[2],"frag.glsl");

	createVS(gProgram[3],"cube_shader.glsl");
    createFS(gProgram[3],"frag.glsl");

    createVS(gProgram[4],"cube_shader_y.glsl");
    createFS(gProgram[4],"frag.glsl");

	createVS(gProgram[5],"vert_text.glsl");
	createFS(gProgram[5],"frag_text.glsl");

    
    glBindAttribLocation(gProgram[0], 0, "inVertex");
    glBindAttribLocation(gProgram[0], 1, "inNormal");
    glBindAttribLocation(gProgram[1], 0, "aPos");
    glBindAttribLocation(gProgram[2], 0, "inVertex");
    glBindAttribLocation(gProgram[2], 1, "inNormal");
    glBindAttribLocation(gProgram[3], 0, "inVertex");
    glBindAttribLocation(gProgram[3], 1, "inNormal");
    glBindAttribLocation(gProgram[4], 0, "inVertex");
    glBindAttribLocation(gProgram[4], 1, "inNormal");
    glBindAttribLocation(gProgram[5], 0, "vertex");
    
    
    

    // Link the programs
	for (int i =0 ; i< meshes.size(); i++)
	{
		glLinkProgram(gProgram[i]);
		GLint status;
		glGetProgramiv(gProgram[i], GL_LINK_STATUS, &status);

		if (status != GL_TRUE)
		{
			cout << "Program link failed" << endl;
			exit(-1);
		}
	}

	glLinkProgram(gProgram[5]);
	GLint status;
	glGetProgramiv(gProgram[5], GL_LINK_STATUS, &status);

	

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	
	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < meshes.size(); ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}

    glUseProgram(gProgram[0]);
}

void initVBO(Mesh &mesh)
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &mesh.gVertexAttribBuffer);
    glGenBuffers(1, &mesh.gIndexBuffer);

    assert(mesh.gVertexAttribBuffer > 0 && mesh.gIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.gIndexBuffer);

    mesh.gVertexDataSizeInBytes = mesh.gVertices.size() * 3 * sizeof(GLfloat);
    mesh.gNormalDataSizeInBytes = mesh.gNormals.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = mesh.gFaces.size() * 3 * sizeof(GLuint);
    GLfloat* vertexData = new GLfloat [mesh.gVertices.size() * 3];
    GLfloat* normalData = new GLfloat [mesh.gNormals.size() * 3];
    GLuint* indexData = new GLuint [mesh.gFaces.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < mesh.gVertices.size(); ++i)
    {
        vertexData[3*i] = mesh.gVertices[i].x;
        vertexData[3*i+1] = mesh.gVertices[i].y;
        vertexData[3*i+2] = mesh.gVertices[i].z;

        minX = std::min(minX, mesh.gVertices[i].x);
        maxX = std::max(maxX, mesh.gVertices[i].x);
        minY = std::min(minY, mesh.gVertices[i].y);
        maxY = std::max(maxY, mesh.gVertices[i].y);
        minZ = std::min(minZ, mesh.gVertices[i].z);
        maxZ = std::max(maxZ, mesh.gVertices[i].z);
    }

    std::cout << "minX = " << minX << std::endl;
    std::cout << "maxX = " << maxX << std::endl;
    std::cout << "minY = " << minY << std::endl;
    std::cout << "maxY = " << maxY << std::endl;
    std::cout << "minZ = " << minZ << std::endl;
    std::cout << "maxZ = " << maxZ << std::endl;

    for (int i = 0; i < mesh.gNormals.size(); ++i)
    {
        normalData[3*i] = mesh.gNormals[i].x;
        normalData[3*i+1] = mesh.gNormals[i].y;
        normalData[3*i+2] = mesh.gNormals[i].z;
    }

    for (int i = 0; i < mesh.gFaces.size(); ++i)
    {
        indexData[3*i] = mesh.gFaces[i].vIndex[0];
        indexData[3*i+1] = mesh.gFaces[i].vIndex[1];
        indexData[3*i+2] = mesh.gFaces[i].vIndex[2];
    }


    glBufferData(GL_ARRAY_BUFFER, mesh.gVertexDataSizeInBytes + mesh.gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh.gVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, mesh.gVertexDataSizeInBytes, mesh.gNormalDataSizeInBytes, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(mesh.gVertexDataSizeInBytes));

}

void initFonts(int windowWidth, int windowHeight)
{
    // Set OpenGL options
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
    glUseProgram(gProgram[5]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[5], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/liberation/LiberationSerif-Italic.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //
    // Configure VBO for texture quads
    //

    glGenVertexArrays(1, &vaoText);
    glGenBuffers(1, &gTextVBO);

    glBindVertexArray(vaoText);
    glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);    

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void init() 
{
    Mesh mesh0, mesh1, mesh2, mesh3, mesh4;
	meshes.push_back(mesh0);
	meshes.push_back(mesh1);
	meshes.push_back(mesh2);
	meshes.push_back(mesh3);
	meshes.push_back(mesh4);

	ParseObj("bunny.obj", meshes[0]);
	glEnable(GL_DEPTH_TEST);

	ParseObj("quad.obj", meshes[1]);
	glEnable(GL_DEPTH_TEST);

	ParseObj("cube.obj", meshes[2]);
	glEnable(GL_DEPTH_TEST);

	ParseObj("cube.obj", meshes[3]);
	glEnable(GL_DEPTH_TEST);

	ParseObj("cube.obj", meshes[4]);
	glEnable(GL_DEPTH_TEST);

	initShaders();

    initFonts(gWidth, gHeight);
	
	initVBO(meshes[0]);
	initVBO(meshes[1]);
	initVBO(meshes[2]);
	initVBO(meshes[3]);
	initVBO(meshes[4]);

	
}

void drawModel(Mesh &mesh)
{
	glBindBuffer(GL_ARRAY_BUFFER, mesh.gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.gIndexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(mesh.gVertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, mesh.gFaces.size() * 3, GL_UNSIGNED_INT, 0);
}

void displayDeadBunny()
{
	activeProgramIndex = 0;

	modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(horizontalOffset, -1.8f, -3.f));
	modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
	modelingMatrix = glm::scale(modelingMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	
	glUseProgram(gProgram[activeProgramIndex]);
	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

	// Draw the scene
	drawModel(meshes[0]);
}
void onYellowCheckpoint()
{
	cout << "Hit Yellow!" << endl;
	turning = true;
	hit = true;
	score += 1000;
}

void onRedCheckpoint(int i)
{
	dead = true;
	redIndex = i+2 ;
	displayDeadBunny();

}
void checkCollision()
{
	if (!(depthOffset >= maxDepth - 3.4f && !hit))
		return;

	for(int i = 0; i < 3; i++)
	{
		if(horizontalOffset > -4.f + 3.0 * i && horizontalOffset < -2.f + 3.0 * i)
		{
			if (yellowIndex == i + 2)
				onYellowCheckpoint();
			else
				onRedCheckpoint(i);
		}
	}
}
void displayBunny()
{
	activeProgramIndex = 0;

	float maxheight = 1.5f;

	float angleRad = (float)(angle / 180.0) * M_PI;

	checkCollision();
	if (dead) return;

    if(moveRight)
    {
        horizontalOffset += 0.7*gameSpeed;
        if(horizontalOffset > 3) horizontalOffset = 3; // 4.6f
    }
    if(moveLeft)
    {
        horizontalOffset -= 0.7*gameSpeed;
        if(horizontalOffset < -3) horizontalOffset = -3; // -4.6f
    }

	modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(horizontalOffset, -2.f + height, -3.f));
	modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
	modelingMatrix = glm::scale(modelingMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	if(angle <= 360.f && turning)
		modelingMatrix = glm::rotate<float>(modelingMatrix, angleRad, glm::vec3(0.0, 1.0, 0.0));
	else
	{
		turning = false;
		angle = 0.0;

	}
	if(falling){
		if(height <= 0){
			height += gameSpeed/3; // 2
			falling = false;
		}
		else{
			height -= gameSpeed/3;
		}
			
	}
	else{
		if (height >= maxheight){

			height -= gameSpeed/3;
			falling = true;
		}
		else
		{
			height += gameSpeed/3;
		}
	}
	//modelingMatrix = glm::rotate(modelingMatrix, angleRad, glm::vec3(0.0, -1.0, 0.0));
	// modelingMatrix = glm::rotate(modelingMatrix, angleRad, glm::vec3(0.0, 0.0, 1.0));
	//modelingMatrix = glm::rotate<float>(modelingMatrix, (-180. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram[activeProgramIndex]);
	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

	// Draw the scene
	angle += gameSpeed*20.f; //40f
	drawModel(meshes[0]);
}

void displayQuad()
{
	activeProgramIndex = 1;

	GLuint ap = gProgram[activeProgramIndex];
	glUseProgram(ap);

	
	modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.f, -2.1f, -10.f));
	modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
	modelingMatrix = glm::scale(modelingMatrix, glm::vec3(5.01f, 50.f, 1.f));

	
	GLint offsetLocationX = glGetUniformLocation(ap, "offsetX");
	GLint offsetLocationZ = glGetUniformLocation(ap, "offsetZ");
	GLint scaleLocation = glGetUniformLocation(ap, "scale");
	GLint color1Location = glGetUniformLocation(ap, "color1");
	GLint color2Location = glGetUniformLocation(ap, "color2");
	glUniform1f(offsetLocationX, 1000.f);
	glUniform1f(offsetLocationZ, 1000.f + (-maxDepth + depthOffset)*1.17f);
	glUniform1f(scaleLocation, 0.49f);
	glUniform3f(color1Location, 1.0f, 1.0f, 1.0f); //White
	glUniform3f(color2Location, 1.0f, 0.753f, 0.796f); // Pink *UwU*


	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));
	drawModel(meshes[1]);

}


float randomizeCubes()
{
	//srand(static_cast<unsigned int>(time(nullptr)));
	int randomIndex = (rand() % 3) + 2;
	return randomIndex;

}
void displayCube(int cubeIndex, int xIndex, bool yellow = false)
{
	activeProgramIndex = cubeIndex;

	modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(-3.f + 3.0 * (xIndex - 2), -0.8f, -maxDepth + depthOffset));
	//modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
	modelingMatrix = glm::scale(modelingMatrix, glm::vec3(0.5f, 1.f, 0.5f));
	
	//modelingMatrix = glm::translate(modelingMatrix, glm::vec3(0.0, height, 0.0));

	//modelingMatrix = glm::rotate(modelingMatrix, angleRad, glm::vec3(0.0, -1.0, 0.0));
	// modelingMatrix = glm::rotate(modelingMatrix, angleRad, glm::vec3(0.0, 0.0, 1.0));
	//modelingMatrix = glm::rotate<float>(modelingMatrix, (-180. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));

	// Set the active program and the values of its uniform variables
	glUseProgram(gProgram[activeProgramIndex]);
	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

	// Draw the scene
	// activeProgramIndex == 4 ? activeProgramIndex = 0 : activeProgramIndex++ ; // cycle the active program index in range [0,4]

	if(hit && yellow)
	{
		return;
	}
	drawModel(meshes[cubeIndex]);
}

void renderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
    glUseProgram(gProgram[5]);
    glUniform3f(glGetUniformLocation(gProgram[5], "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vaoText);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        
        //glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)

        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void display(vector<Mesh> meshes)
{
    if (dead){
        gameSpeed = 0;
        speedIncrease = 0;
    } 
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if( depthOffset >= maxDepth - 1.f){
		randomize = true;
		depthOffset = 0.f;
		hit = false;
	}
	int x2, x3;
	if (randomize)
	{
		yellowIndex = randomizeCubes();
		randomize = false;
	}
	switch(yellowIndex)
	{
		case 2:
			x2 = 3;
			x3= 4;
			break;
		case 3:
			x2 = 2;
			x3= 4;
			break;
		case 4:
			x2 = 3;
			x3= 2;
			break;
	}
	// drawModel();
	displayBunny();
	displayQuad();
	if(dead)
	{
		if(x2 != redIndex)
			displayCube(2, x2);
		if(x3 != redIndex)
			displayCube(3, x3);
	}
	else
	{
		displayCube(2, x2);
		displayCube(3, x3);
	}
		
	displayCube(4, yellowIndex, true);

	
	if(dead)
    {
        renderText("Score: ", 0, 440, 1, glm::vec3(1, 0, 0));
        renderText(to_string((int) score), 130, 440,  1, glm::vec3(1, 0, 0));
    }
    else
    {
       renderText("Score: ", 0, 440, 1, glm::vec3(1, 1, 0));
       renderText(to_string((int) score), 130, 440,  1, glm::vec3(1, 1, 0)); 
    }
    assert(glGetError() == GL_NO_ERROR);
	
	gameSpeed += speedIncrease;
    depthOffset += gameSpeed;
	score += gameSpeed;

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
	projectionMatrix = glm::perspective(fovyRad, w / (float)h, 1.0f, 100.0f);

	// Assume default camera position and orientation (camera is at
	// (0, 0, 0) with looking at -z direction and its up vector pointing
	// at +y direction)
	// 
	//viewingMatrix = glm::mat4(1);
	viewingMatrix = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

}
void Reload()
{
    score = 0.0f;
    dead = false;
    randomize = true;
    depthOffset = 0.f;
	hit = false;
    gameSpeed = 0.15f;
    speedIncrease = 0.0002f;
    horizontalOffset = 0.0f;
    angle = 0;
    height = 0;
    falling = false;
    turning = false;
}
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_A)
	{
        if((action == GLFW_PRESS || action == GLFW_REPEAT) && horizontalOffset >= -3)
        {
            moveLeft = true;
        }
        else moveLeft = false;
        if(action == GLFW_RELEASE) moveLeft = false;
	}
    else if (key == GLFW_KEY_D)
	{
        if((action == GLFW_PRESS || action == GLFW_REPEAT) && horizontalOffset <= 3)
        {
            moveRight = true;
        }
        else moveRight = false;
        if(action == GLFW_RELEASE) moveRight = false;
	}

    else if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
    else if(key == GLFW_KEY_R && action == GLFW_PRESS)
    	{
        	Reload();
    	}
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        display(meshes);
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(gWidth, gHeight, "Simple Example", NULL, NULL);

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

    char rendererInfo[512] = {0};
    strcpy(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();

    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, gWidth, gHeight); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

