#include <cstdio>		// for C++ i/o
#include <iostream>
#include <string>
#include <cstddef>
using namespace std;	// to avoid having to use std::

#include <GLEW/glew.h>	// include GLEW
#include <GLFW/glfw3.h>	// include GLFW (which includes the OpenGL header)
#include <glm/glm.hpp>	// include GLM (ideally should only use the GLM headers that are actually used)
#include <glm/gtx/transform.hpp>
using namespace glm;	// to avoid having to use glm::

//for part 2 on the rotation of the planets, make the multiplication of the y the multiplication of the z.
#include "shader.h"

// struct for vertex attributes
struct Vertex
{
	GLfloat position[3];
	GLfloat color[3];
};

// global variables
Vertex line_vertices[2] = {
    1.0f, -.75f, 0.0f,  //right
    1.0f, 1.0f, 1.0f,   //color
    -1.0f, -.75f, 0.0f, //left
    1.0f, 1.0f, 1.0f,   //color
};

Vertex tri_vertices[] = {
    -0.25f, -.75f, 0.0f, //left
    0.0f, 0.7f, 0.7f,   //color
    0.0f, 0.0f, 0.0f, //top
    0.0f, 0.2f, 0.3f,   //color
    0.25f, -.75f, 0.0f, //right
    0.2f, 0.0f, 0.6f,   //color
};

//carriage vertices
Vertex c_vertices[] = {
    0.0f, 0.0f, 0.0f, //center
    0.0f, 0.7f, 0.7f,   //color
    -0.25f, 0.0f, 0.0f, //left
    0.0f, 0.7f, 0.7f,   //color
    -0.15f, 0.15f, 0.0f, //top left
    0.0f, 0.2f, 0.3f,   //color
    0.15f, 0.15f, 0.0f, //top right
    0.2f, 0.0f, 0.6f,   //color
    0.25f, 0.0f, 0.0f, //right
    0.2f, 0.0f, 0.6f,   //color
    0.15f, -0.15f, 0.0f, //bottom right
    0.2f, 0.0f, 0.6f,   //color
    -0.15f, -0.15f, 0.0f, //bottom left
    0.2f, 0.0f, 0.6f,   //color
    -0.25f, 0.0f, 0.0f, //left
    0.0f, 0.7f, 0.7f,   //color
};

GLuint line_indices[2] ={
  0, 1
};

GLuint tri_indices[3] ={
    0, 1, 2
};

GLuint c_indices[6] ={
    0, 1, 2,
    3, 4, 5
};

GLuint g_IBO = 0;				// index buffer object identifier
GLuint g_VBO = 0;				// vertex buffer object identifier
GLuint g_VAO = 0;				// vertex array object identifier
GLuint g_shaderProgramID = 0;	// shader program identifier
GLuint g_MVP_Index = 0;			// location in shader
glm::mat4 g_modelMatrix[17];		// object model matrices
glm::mat4 g_viewMatrix;			// view matrix can get rid of these becasue 2D and can delete from equations
glm::mat4 g_projectionMatrix;	// projection matrix

float g_orbitSpeed = 0.3f;
float g_rotationSpeed[2] = { 0.1f, 0.3f };

bool isD = false;
bool isP = false;

static void init(GLFWwindow* window)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// set clear background colour

	glEnable(GL_DEPTH_TEST);	// enable depth buffer test

	// create and compile our GLSL program from the shader files
	g_shaderProgramID = loadShaders("MVP_VS.vert", "ColorFS.frag");

	// find the location of shader variables
	GLuint positionIndex = glGetAttribLocation(g_shaderProgramID, "aPosition");
	GLuint colorIndex = glGetAttribLocation(g_shaderProgramID, "aColor");
	g_MVP_Index = glGetUniformLocation(g_shaderProgramID, "uModelViewProjectionMatrix");

	// initialise model matrix to the identity matrix
	g_modelMatrix[0] = g_modelMatrix[1] = g_modelMatrix[2] = g_modelMatrix[3] = glm::mat4(1.0f);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspectRatio = static_cast<float>(width) / height;

	// initialise projection matrix
	g_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	
	// generate identifier for VBO and copy data to GPU
	glGenBuffers(1, &g_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof( c_vertices),  c_vertices, GL_STATIC_DRAW);
    
	glGenBuffers(1, &g_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	//

	// generate identifiers for VAO
	glGenVertexArrays(1, &g_VAO);

	// create VAO and specify VBO data
	glBindVertexArray(g_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_IBO);
	// interleaved attributes
	glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

	glEnableVertexAttribArray(positionIndex);	// enable vertex attributes
	glEnableVertexAttribArray(colorIndex);
}

// function used to update the scene
static void update_scene()
{
    // static variables for rotation angles
    static float orbitAngle = 1.0f;
    static float rotationAngle[10] = {
        0.0f, 0.0f, 0.1f,
        0.2f, 0.3f, 0.4f,
        0.5f, 0.6f, 0.7f,
        0.8f
    };
    float scaleFactor = 0.1f;
    
	// update rotation angles
	orbitAngle += g_orbitSpeed * scaleFactor;
    for (int i = 0; i < 10; i++){
            rotationAngle[i] += g_rotationSpeed[i] * scaleFactor;
    }

	// update model matrix
    
    //frame
    if(isD==true && isP == false){
        g_modelMatrix[16] = glm::rotate(rotationAngle[1], glm::vec3(0.0f, 0.0f, 1.0f));
    }
    else if (isD == false && isP == false){
        g_modelMatrix[16] = glm::rotate(rotationAngle[1], glm::vec3(0.0f, 0.0f, -1.0f));
    }
    
    //carriage 1
    g_modelMatrix[4] = g_modelMatrix[16]
        * translate(glm::vec3(0.125f, 0.4f, 0.0f))
        * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
    
    //rest of carriages
    for (int i = 5; i < 15; i++){g_modelMatrix[i] = g_modelMatrix[i-1]
        * translate(glm::vec3((cos(i * M_PI/5))/2, (sin(i * M_PI/5))/2, 0.0f));
    }
}

// function used to render the scene
static void render_scene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear colour buffer and depth buffer

	glUseProgram(g_shaderProgramID);	// use the shaders associated with the shader program

	glBindVertexArray(g_VAO);		// make VAO active

// OG center box
	glm::mat4 MVP = g_modelMatrix[0] ;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof( c_indices),  c_indices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(c_vertices), c_vertices, GL_STATIC_DRAW);
    
//line
    MVP = g_modelMatrix[2];
    glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(line_indices), line_indices, GL_STATIC_DRAW);
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
    
//triangle
    MVP = g_modelMatrix[3];
    glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tri_vertices), tri_vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tri_indices), tri_indices, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    
//carriage
    MVP = g_modelMatrix[4];
    glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(c_vertices), c_vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(c_indices), c_indices, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLE_FAN, 8, GL_UNSIGNED_INT, 0);
    
    for (int i = 5; i < 15; i++){
        MVP = g_modelMatrix[i];
        glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
        glDrawElements(GL_TRIANGLE_FAN, 8, GL_UNSIGNED_INT, 0);
    }
    
//circle fan
    GLuint numOfVertices = 12; //10sides
    GLfloat doublePi = 2.0f * M_PI;
    
    GLfloat c_vert_x[numOfVertices];
    GLfloat c_vert_y[numOfVertices];
    GLfloat c_vert_z[numOfVertices];
    
    GLuint circle_indices[12] = {0};
    
    //centerpoints
    c_vert_x[0] = 0.0f;
    c_vert_y[0] = 0.0f;
    c_vert_z[0] = 0.0f;
    
    //set vertices
    for(int i = 1; i < numOfVertices; i++){
        c_vert_x[i] = (cos(i * doublePi / 10))/2.25;
        c_vert_y[i] = (sin(i * doublePi / 10))/2.25;
        c_vert_z[i] = 0.0f;
        
        circle_indices[i] = i;
    }
    
    GLfloat allVerts[numOfVertices * 6]; //36 coords, 36more for color
    GLfloat colorVerts[3] = {1.0f, 0.0f, 0.0f};
    
    //make combined array with all vertices.
    for(int k = 0; k < numOfVertices; k++){
        allVerts[k*6] = c_vert_x[k];
        allVerts[(k*6) + 1] = c_vert_y[k];
        allVerts[(k*6) + 2] = c_vert_z[k];
        allVerts[(k*6) + 3] = colorVerts[0];
        allVerts[(k*6) + 4] = colorVerts[1];
        allVerts[(k*6) + 5] = colorVerts[2];
    }
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    MVP = g_modelMatrix[16];
    glUniformMatrix4fv(g_MVP_Index, 1, GL_FALSE, &MVP[0][0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(allVerts), allVerts, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(circle_indices), circle_indices, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLE_FAN, 12, GL_UNSIGNED_INT, 0);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );//switch back to fill
    
	glFlush();	// flush the pipeline
}

// key press or release callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// quit if the ESCAPE key was press
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
    if (key == GLFW_KEY_D && action == GLFW_PRESS){
        isD = !isD;
    }
    //toggle wireframe and fill when press w
    if(key == GLFW_KEY_P && action == GLFW_PRESS) {
        isP = !isP;
    }
}

// mouse movement callback function
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
}

// mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
}

// error callback function
static void error_callback(int error, const char* description)
{
	cerr << description << endl;	// output error description
}

int main(void)
{
	GLFWwindow* window = NULL;	// pointer to a GLFW window handle

	glfwSetErrorCallback(error_callback);	// set error callback function

	// initialise GLFW
	if (!glfwInit())
	{
		// if failed to initialise GLFW
		exit(EXIT_FAILURE);
	}

	// minimum OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window and its OpenGL context
	window = glfwCreateWindow(800, 800, "Assessment1", NULL, NULL);

	// if failed to create window
	if (window == NULL)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);	// set window context as the current context
	glfwSwapInterval(1);			// swap buffer interval

	// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		cerr << "GLEW initialisation failed" << endl;
		exit(EXIT_FAILURE);
	}

	// set key callback function
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);



	// initialise rendering states
	init(window);

	// variables for simple time management
	float lastUpdateTime = glfwGetTime();
	float currentTime = lastUpdateTime;

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();

		// only update if more than 0.02 seconds since last update
		if (currentTime - lastUpdateTime > 0.02)
		{
			update_scene();		// update the scene
			render_scene();		// render the scene
		// draw tweak bar(s)

			glfwSwapBuffers(window);	// swap buffers
			glfwPollEvents();			// poll for events

			lastUpdateTime = currentTime;	// update last update time
		}
	}

	// clean up
	glDeleteProgram(g_shaderProgramID);
	glDeleteBuffers(1, &g_IBO);
	glDeleteBuffers(1, &g_VBO);
	glDeleteVertexArrays(1, &g_VAO);

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

