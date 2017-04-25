// glew must be before glfw
#include <iostream>
#include <array>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "icg_helper.h"
#include <glm/gtc/matrix_transform.hpp>

#include "grid/grid.h"
#include "framebuffer.h"
#include "screenquad/screenquad.h"
#include "perlin/perlin.h"
#include "camera/camera.h"
#include "normalmap/normalmap.h"
#include "frame_counter.h"

using namespace std;
using namespace glm;

Grid grid;
Perlin perlin;
Camera camera;
FrameBuffer framebuffer;
FrameBuffer normalBuffer;
ScreenQuad screenquad;
NormalMap normalMap;
FrameCounter frameCounter;

int window_width = 1280;
int window_height = 960;

mat4 projection_matrix;
mat4 view_matrix;
mat4 quad_model_matrix;

#include "infinite_grid.h"
#include "iocallbacks.h"

GLFWwindow* setupGLFWwindow();
void initGlew();
void ErrorCallback(int error, const char* description);
void SetupProjection(GLFWwindow* window, int width, int height);
vec2 transformScreenCoords(GLFWwindow* window, int x, int y);

void display() {
    frameCounter.countNewFrame();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    view_matrix = camera.GetViewMatrix();
    grid.Draw(quad_model_matrix, view_matrix, projection_matrix);
    //screenquad.Draw();
}

int main(int argc, char *argv[]) {
    using namespace noiseParams;

    GLFWwindow* window = setupGLFWwindow();
    initGlew();

    glClearColor(0.0, 0.0, 0.0, 1.0 /*solid*/);
    glEnable(GL_DEPTH_TEST);

    int framebuffer_id = framebuffer.Init(noiseBufferWidth, noiseBufferHeight, true);
    int normalBuffer_id = normalBuffer.Init(noiseBufferWidth, noiseBufferHeight, true);
    camera = Camera{vec3(0.0, 1.0, 0.0)};
    perlin.Init();
    normalMap.Init(framebuffer_id);
    grid.Init(framebuffer_id, normalBuffer_id);
    //screenquad.Init(window_width, window_height, normalBuffer_id);

    view_matrix = camera.GetViewMatrix();
    quad_model_matrix = translate(mat4(1.0f), vec3(0.0f, -0.25f, 0.0f));

    drawPerlin(framebuffer);
    normalBuffer.Bind();
    normalMap.Draw();
    normalBuffer.Unbind();

    // update the window size with the framebuffer size (on hidpi screens the
    // framebuffer is bigger)
    glfwGetFramebufferSize(window, &window_width, &window_height);

    // render loop
    while(!glfwWindowShouldClose(window)){
        SetupProjection(window, window_width, window_height);
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
        iocallbacks::doMovement();
    }

    // close OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}

GLFWwindow* setupGLFWwindow() {
  // GLFW Initialization
  if(!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    exit(EXIT_FAILURE);
  }
  glfwSetErrorCallback(ErrorCallback);
  // setup openGl version
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // create window
  GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                        "Procedural terrain", NULL, NULL);
  if(!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  // makes the OpenGL context of window current on the calling thread
  glfwMakeContextCurrent(window);
  // Cursor is captured and hidden
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // set the framebuffer resize callback
  glfwSetFramebufferSizeCallback(window, SetupProjection);
  iocallbacks::init(window);
  return window;
}

void initGlew() {
  // GLEW Initialization (must have a context)
  // https://www.opengl.org/wiki/OpenGL_Loading_Library
  glewExperimental = GL_TRUE; // fixes glew error (see above link)
  if(glewInit() != GLEW_NO_ERROR) {
    fprintf( stderr, "Failed to initialize GLEW\n");
    exit(EXIT_FAILURE);
  }
  cout << "OpenGL" << glGetString(GL_VERSION) << endl;
}

// transforms glfw screen coordinates into normalized OpenGL coordinates.
vec2 transformScreenCoords(GLFWwindow* window, int x, int y) {
    // the framebuffer and the window doesn't necessarily have the same size
    // i.e. hidpi screens. so we need to get the correct one
    int width = 1024;
    int height = 1024;
    glfwGetWindowSize(window, &width, &height);
    return vec2(2.0f * (float)x / width - 1.0f,
                1.0f - 2.0f * (float)y / height);
}

// Gets called when the windows/framebuffer is resized.
void SetupProjection(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;
    //cout << "Window has been resized to "
    //     << window_width << "x" << window_height << "." << endl;
    glViewport(0, 0, window_width, window_height);
    projection_matrix = glm::perspective(glm::radians(camera.Fov), (GLfloat)window_width / window_height, 0.1f, 1000.0f);
    screenquad.UpdateSize(window_width, window_height);
}

void ErrorCallback(int error, const char* description) {
    fputs(description, stderr);
}
