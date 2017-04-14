// glew must be before glfw
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "icg_helper.h"
#include <glm/gtc/matrix_transform.hpp>

#include "grid/grid.h"
#include "framebuffer.h"
#include "framebufferRGBA.h"
#include "screenquad/screenquad.h"
#include "perlin/perlin.h"
#include "camera/camera.h"
#include "normalmap/normalmap.h"
#include "water/water.h"

using namespace glm;

Grid grid;
Perlin perlin;
Camera camera;
FrameBufferRGBA framebuffer;
FrameBufferRGBA normalBuffer;
FrameBuffer reflectionBuffer;
ScreenQuad screenquad;
NormalMap normalMap;
Water water;

bool keys[1024];
bool firstMouse = false;
int window_width = 1280;
int window_height = 960;
float lastX = 0.0f;
float lastY = 0.0f;

const float OFFSET_QTY = 0.04f;

mat4 projection_matrix;
mat4 view_matrix;
mat4 quad_model_matrix;

const GLfloat SEC_DURATION = 1.0;
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame
GLfloat lastSec = 0.0;
GLuint frameCount = 0;


void Init() {
    // Initialize camera
    camera = Camera{vec3(0.0, 1.0, 0.0)};

    // sets background color
    glClearColor(0.0, 0.0, 0.0, 1.0 /*solid*/);
    perlin.Init();
    int framebuffer_texture_id = framebuffer.Init(1024, 1024, true);
    int normalBuffer_texture_id = normalBuffer.Init(1024, 1024, true);
    int reflectionBuffer_texture_id = reflectionBuffer.Init(window_width, window_height, false);
    normalMap.Init(framebuffer_texture_id);
    grid.Init(framebuffer_texture_id, normalBuffer_texture_id);
    screenquad.Init(window_width, window_height, normalBuffer_texture_id);
    water.Init(reflectionBuffer_texture_id, framebuffer_texture_id, normalBuffer_texture_id);

    // enable depth test.
    glEnable(GL_DEPTH_TEST);

    // enable culling
    glCullFace(GL_BACK);

    view_matrix = camera.GetViewMatrix();

    // scaling matrix to scale the cube down to a reasonable size.
    quad_model_matrix = IDENTITY_MATRIX;

    //Generate Perlin
    framebuffer.Bind();
        perlin.Draw();
    framebuffer.Unbind();

    normalBuffer.Bind();
        normalMap.Draw();
    normalBuffer.Unbind();


    //Initialise boolean keys array
    for(int i=0; i < 1024; i++){
        keys[i] = false;
    }
}

void Display() {
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if(currentFrame - lastSec > SEC_DURATION){
        std::cout << "Frames per second: " << frameCount << std::endl;
        lastSec = currentFrame;
        frameCount = 0;
    }

    view_matrix = camera.GetViewMatrix();
    mat4 mirrored_view_matrix = camera.GetMirroredViewMatrix(0.0f);
    // reflection computation

    reflectionBuffer.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        grid.Draw(glm::scale(IDENTITY_MATRIX, vec3(1.0, -1.0, 1.0)), view_matrix, projection_matrix, true);
    reflectionBuffer.Unbind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //TODO: uniformiser la facon dont on passe les matrices au GPU
    grid.Draw(glm::scale(IDENTITY_MATRIX, vec3(1.0, 1.0, 1.0)), view_matrix, projection_matrix, false);
    water.Draw(IDENTITY_MATRIX, view_matrix, projection_matrix);
    //grid.Draw(glm::scale(IDENTITY_MATRIX, vec3(1.0, -1.0, 1.0)), view_matrix, projection_matrix, true);
    //screenquad.Draw();

    frameCount++;
}

// transforms glfw screen coordinates into normalized OpenGL coordinates.
vec2 TransformScreenCoords(GLFWwindow* window, int x, int y) {
    // the framebuffer and the window doesn't necessarily have the same size
    // i.e. hidpi screens. so we need to get the correct one
    int width;
    int height;
    glfwGetWindowSize(window, &width, &height);
    return vec2(2.0f * (float)x / width - 1.0f,
                1.0f - 2.0f * (float)y / height);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

    if(firstMouse) // this bool variable is initially set to true
    {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(yoffset);
  projection_matrix = perspective(glm::radians(camera.Fov), (GLfloat)window_width / (GLfloat)window_height, 0.1f, 100.0f);
}

// Gets called when the windows/framebuffer is resized.
void SetupProjection(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;

    cout << "Window has been resized to "
         << window_width << "x" << window_height << "." << endl;

    glViewport(0, 0, window_width, window_height);

    projection_matrix = glm::perspective(glm::radians(camera.Fov), (GLfloat)window_width / window_height, 0.1f, 1000.0f);
    screenquad.UpdateSize(window_width, window_height);
}

void ErrorCallback(int error, const char* description) {
    fputs(description, stderr);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

   if(action == GLFW_PRESS){
        keys[key] = true;

        switch(key){
            case GLFW_KEY_F:
                grid.toggleWireframeMode();
                water.toggleWireframeMode();
                break;
            case GLFW_KEY_H:
                grid.updateZoomFactor(+0.1);
                break;
            case GLFW_KEY_G:
                grid.updateZoomFactor(-0.1);
                break;
            case GLFW_KEY_RIGHT:
                grid.updateOffset(vec2(-OFFSET_QTY, 0.0));
                break;
            case GLFW_KEY_LEFT:
                grid.updateOffset(vec2(OFFSET_QTY, 0.0));
                break;
            case GLFW_KEY_UP:
                grid.updateOffset(vec2(0.0, -OFFSET_QTY));
                break;
            case GLFW_KEY_DOWN:
                grid.updateOffset(vec2(0.0, OFFSET_QTY));
                break;
        }
    } else if(action == GLFW_RELEASE){
      keys[key] = false;
    }
    //camera.debug();
}

void doMovement()
{
  // Camera controls
  if(keys[GLFW_KEY_W])
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if(keys[GLFW_KEY_S])
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if(keys[GLFW_KEY_A])
    camera.ProcessKeyboard(LEFT, deltaTime);
  if(keys[GLFW_KEY_D])
    camera.ProcessKeyboard(RIGHT, deltaTime);
  if(keys[GLFW_KEY_SPACE])
    camera.ProcessKeyboard(UPWARD, deltaTime);
  if(keys[GLFW_KEY_LEFT_SHIFT])
    camera.ProcessKeyboard(DOWNWARD, deltaTime);
  if(keys[GLFW_KEY_J])
      camera.ProcessKeyboard(ROTATE_LEFT, deltaTime);
  if(keys[GLFW_KEY_I])
      camera.ProcessKeyboard(ROTATE_UP, deltaTime);
  if(keys[GLFW_KEY_K])
      camera.ProcessKeyboard(ROTATE_DOWN, deltaTime);
  if(keys[GLFW_KEY_L])
      camera.ProcessKeyboard(ROTATE_RIGHT, deltaTime);
}

int main(int argc, char *argv[]) {
    for(auto& key : keys) {
        key = false;
    }
    // GLFW Initialization
    if(!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(ErrorCallback);


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                          "Procedural terrain", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // makes the OpenGL context of window current on the calling thread
    glfwMakeContextCurrent(window);

    // Cursor is captured and hidden
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Enable transparent materials
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // set the callback for escape key
    glfwSetKeyCallback(window, KeyCallback);

    // set the framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, SetupProjection);

    // set the mouse press and position callback
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // GLEW Initialization (must have a context)
    // https://www.opengl.org/wiki/OpenGL_Loading_Library
    glewExperimental = GL_TRUE; // fixes glew error (see above link)
    if(glewInit() != GLEW_NO_ERROR) {
        fprintf( stderr, "Failed to initialize GLEW\n");
        return EXIT_FAILURE;
    }

    cout << "OpenGL" << glGetString(GL_VERSION) << endl;

    // initialize our OpenGL program
    Init();

    // update the window size with the framebuffer size (on hidpi screens the
    // framebuffer is bigger)
    glfwGetFramebufferSize(window, &window_width, &window_height);
    SetupProjection(window, window_width, window_height);

    // render loop
    while(!glfwWindowShouldClose(window)){
        Display();
        glfwSwapBuffers(window);
        glfwPollEvents();
        doMovement();
    }

    grid.Cleanup();
    perlin.Cleanup();
    water.Cleanup();
    reflectionBuffer.Cleanup();
    framebuffer.Cleanup();
    normalMap.Cleanup();

    // close OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
