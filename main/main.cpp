// glew must be before glfw
#include <iostream>
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

using namespace glm;

Grid grid;
Perlin perlin;
Camera camera;
FrameBuffer framebuffer;
FrameBuffer normalBuffer;
ScreenQuad screenquad;
NormalMap normalMap;


namespace gridParams {
  constexpr int iMax = 20;
  constexpr int jMax = 20;
  int iBottomLeftUnit = 0;
  int jBottomLeftUnit = 0;
}

namespace textureParams {
  constexpr float xScale = 1.0f / gridParams::jMax;
  constexpr float yScale = 1.0f / gridParams::iMax;
  constexpr float xOriginalWidth = 2.0f;
  constexpr float yOriginalHeight = 2.0f;
  constexpr float xWidth = xOriginalWidth * xScale;
  constexpr float yHeight = yOriginalHeight * yScale;
  const vec3 toTextureBottomLeft = vec3(-1 + xWidth / 2.0f, -1 + yHeight / 2.0, 0);
}
namespace noiseParams {
  const vec2 offsetGainPerGridColumn = {1, 0};
  const vec2 offsetGainPerGridLine = {0, 1};
  vec2 bottomLeftUnitOffset = {0, 0};
  float noiseBufferHeight = 1024;
  float noiseBufferWidth = 1024;
}

void redrawLine(int iLig) {
  using namespace gridParams;
  using namespace textureParams;
  using namespace noiseParams;

  int iUnit = (iLig + iMax - iBottomLeftUnit) % iMax;
  vec2 unitNoiseOffset = bottomLeftUnitOffset + offsetGainPerGridLine * ((float) iUnit);

  framebuffer.Bind();
  for (int jCol = 0; jCol < jMax; ++jCol) {
    vec3 iTranslation = {0, iLig * yHeight, 0};
    vec3 jTranslation = {((jBottomLeftUnit + jCol) % jMax) * xWidth, 0, 0};

    perlin.Draw(unitNoiseOffset + offsetGainPerGridColumn * ((float) jCol),
                {xScale, yScale},
                toTextureBottomLeft + iTranslation + jTranslation);
  }
  framebuffer.Unbind();
  normalBuffer.Bind();
  normalMap.Draw();
  normalBuffer.Unbind();
}
void redrawColumn(int jCol) {
  using namespace gridParams;
  using namespace textureParams;
  using namespace noiseParams;

  int jUnit = (jCol + jMax - jBottomLeftUnit) % jMax;
  vec2 unitNoiseOffset = bottomLeftUnitOffset + offsetGainPerGridColumn * ((float) jUnit);

  framebuffer.Bind();
  for (int iLig = 0; iLig < iMax; ++iLig) {
    vec3 iTranslation = {0, ((iBottomLeftUnit + iLig) % iMax) * yHeight, 0};
    vec3 jTranslation = {jCol * xWidth, 0, 0};

    perlin.Draw(unitNoiseOffset + offsetGainPerGridLine * ((float) iLig),
                {xScale, yScale},
                toTextureBottomLeft + iTranslation + jTranslation);
  }
  framebuffer.Unbind();
  normalBuffer.Bind();
  normalMap.Draw();
  normalBuffer.Unbind();
}
void updatePerlinOffsetLeft() {
  using namespace gridParams;
  using namespace textureParams;
  using namespace noiseParams;
  jBottomLeftUnit = (jBottomLeftUnit + jMax - 1) % jMax;
  int jObsoleteCol = jBottomLeftUnit;
  bottomLeftUnitOffset -= offsetGainPerGridColumn;
  grid.updateOffset(vec2(bottomLeftUnitOffset.x/jMax, bottomLeftUnitOffset.y/iMax));
  redrawColumn(jObsoleteCol);
}
void updatePerlinOffsetRight() {
  using namespace gridParams;
  using namespace textureParams;
  using namespace noiseParams;
  int jObsoleteCol = jBottomLeftUnit;
  jBottomLeftUnit = (jBottomLeftUnit + 1) % jMax;
  bottomLeftUnitOffset += offsetGainPerGridColumn;
  grid.updateOffset(vec2(bottomLeftUnitOffset.x/jMax, bottomLeftUnitOffset.y/iMax));
  redrawColumn(jObsoleteCol);
}
void updatePerlinOffsetBottom() {
  using namespace gridParams;
  using namespace textureParams;
  using namespace noiseParams;
  iBottomLeftUnit = (iBottomLeftUnit + iMax - 1) % iMax;
  int iObsoleteLig = iBottomLeftUnit;
  bottomLeftUnitOffset -= offsetGainPerGridLine;
  grid.updateOffset(vec2(bottomLeftUnitOffset.x/jMax, bottomLeftUnitOffset.y/iMax));
  redrawLine(iObsoleteLig);
}
void updatePerlinOffsetTop() {
  using namespace gridParams;
  using namespace textureParams;
  using namespace noiseParams;
  int iObsoleteLig = iBottomLeftUnit;
  iBottomLeftUnit = (iBottomLeftUnit + 1) % iMax;
  bottomLeftUnitOffset += offsetGainPerGridLine;
  grid.updateOffset(vec2(bottomLeftUnitOffset.x/jMax, bottomLeftUnitOffset.y/iMax));
  redrawLine(iObsoleteLig);
}


bool keys[1024];
bool firstMouse = false;
int window_width = 1280;
int window_height = 960;
float lastX = 0.0f;
float lastY = 0.0f;


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
    int framebuffer_texture_id = framebuffer.Init(noiseParams::noiseBufferWidth, noiseParams::noiseBufferHeight, true);
    int normalBuffer_texture_id = normalBuffer.Init(1024, 1024, true);
    normalMap.Init(framebuffer_texture_id);
    grid.Init(framebuffer_texture_id, normalBuffer_texture_id);
    screenquad.Init(window_width, window_height, framebuffer_texture_id);

    // enable depth test.
    glEnable(GL_DEPTH_TEST);

    view_matrix = camera.GetViewMatrix();

    // scaling matrix to scale the cube down to a reasonable size.
    quad_model_matrix = translate(mat4(1.0f), vec3(0.0f, -0.25f, 0.0f));

    //Generate Perlin
    for (int jCol = 0; jCol < gridParams::jMax; ++jCol) {
      redrawColumn(jCol);
    }

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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    view_matrix = camera.GetViewMatrix();

    //glm::rotate(IDENTITY_MATRIX,(float)( (3.14/180) * glfwGetTime() * 30), glm::vec3(0.0, 1.0, 0.0))

    grid.Draw(quad_model_matrix, view_matrix, projection_matrix);
    //screenquad.Draw();

    frameCount++;
}

// transforms glfw screen coordinates into normalized OpenGL coordinates.
vec2 TransformScreenCoords(GLFWwindow* window, int x, int y) {
    // the framebuffer and the window doesn't necessarily have the same size
    // i.e. hidpi screens. so we need to get the correct one
    int width = 1024;
    int height = 1024;
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
                break;
            case GLFW_KEY_H:
                grid.updateZoomFactor(+0.1);
                break;
            case GLFW_KEY_G:
                grid.updateZoomFactor(-0.1);
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
  if(keys[GLFW_KEY_RIGHT])
      updatePerlinOffsetRight();
  if(keys[GLFW_KEY_LEFT])
      updatePerlinOffsetLeft();
  if(keys[GLFW_KEY_UP])
      updatePerlinOffsetTop();
  if(keys[GLFW_KEY_DOWN])
      updatePerlinOffsetBottom();
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

    // render loop
    while(!glfwWindowShouldClose(window)){
        SetupProjection(window, window_width, window_height);
        Display();
        glfwSwapBuffers(window);
        glfwPollEvents();
        doMovement();
    }

    grid.Cleanup();
    perlin.Cleanup();
    framebuffer.Cleanup();
    normalMap.Cleanup();

    // close OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
