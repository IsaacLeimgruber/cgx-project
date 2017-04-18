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

using namespace glm;

constexpr int gridWidth  = 3;
constexpr int gridHeight = 3;

template <class T>
using Matrix = array<array<T, gridWidth>, gridHeight>;

Matrix<Grid> grid;
Perlin perlin;
Camera camera;
Matrix<FrameBuffer> framebuffer;
Matrix<FrameBuffer> normalBuffer;
ScreenQuad screenquad;
Matrix<NormalMap> normalMap;
vec2 currentOffset{1, 1}; //offset for middle grid
int iOffset = 0;
int jOffset = 0;
bool setupWindowBuffer = true;
int noiseTextureWidth = 256;
int noiseTextureHeight = 256;

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

vec3 translationForGrid(int i, int j) {
  static Matrix<vec3> gridTranslation{
    vec3(-1, 0,-1),
    vec3( 0, 0,-1),
    vec3( 1, 0,-1),
    vec3(-1, 0, 0),
    vec3( 0, 0, 0),
    vec3( 1, 0, 0),
    vec3(-1, 0, 1),
    vec3( 0, 0, 1),
    vec3( 1, 0, 1),
  };
  int ti = (i + iOffset + gridHeight) % gridHeight;
  int tj = (j + jOffset + gridWidth ) % gridWidth;
  return gridTranslation[ti][tj];
}
void redrawNoise(int i, int j) {
  vec3 t = translationForGrid(i, j);

  framebuffer[i][j].Bind();
  perlin.Draw(currentOffset.x + t.x, currentOffset.y - t.z);
  framebuffer[i][j].Unbind();

  normalBuffer[i][j].Bind();
  normalMap[i][j].Draw();
  normalBuffer[i][j].Unbind();

  setupWindowBuffer = true;
}
void Init() {
    // Initialize camera
    camera = Camera{vec3(0.0, 1.0, 0.0)};

    // sets background color
    glClearColor(0.0, 0.0, 0.0, 1.0 /*solid*/);
    perlin.Init();

    Matrix<int> framebuffer_id;
    Matrix<int> normalbuffer_id;
    for (int i = 0; i < gridHeight; ++i) {
      for (int j = 0; j < gridWidth; ++j) {
        framebuffer_id[i][j] = framebuffer[i][j].Init(noiseTextureWidth, noiseTextureHeight, true);
        normalbuffer_id[i][j] = normalBuffer[i][j].Init(noiseTextureWidth, noiseTextureHeight, true);
        normalMap[i][j].Init(framebuffer_id[i][j]);
        grid[i][j].Init(framebuffer_id[i][j], normalbuffer_id[i][j]);
      }
    }

    //screenquad.Init(window_width, window_height, normalbuffer_id[0]);

    // enable depth test.
    glEnable(GL_DEPTH_TEST);

    view_matrix = camera.GetViewMatrix();

    // scaling matrix to scale the cube down to a reasonable size.
    quad_model_matrix = translate(mat4(1.0f), vec3(0.0f, -0.25f, 0.0f));

    //Generate Perlin
    for (int i = 0; i < gridHeight; ++i) {
      for (int j = 0; j < gridWidth; ++j) {
        redrawNoise(i, j);
      }
    }

    //Initialise boolean keys array
    for(auto& key : keys) {
      key = false;
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

    for (int i = 0; i < gridHeight; ++i) {
      for (int j = 0; j < gridWidth; ++j) {
        glm::mat4 t = translate(quad_model_matrix, translationForGrid(i,j)*2.0f);
        grid[i][j].Draw(t, view_matrix, projection_matrix);
      }
    }

    frameCount++;
}

void moveRight() {
  jOffset = (jOffset + 1) % gridWidth;
  currentOffset.x -= 1;

  int j = (gridWidth - jOffset) % gridWidth;
  for (int i = 0; i < gridHeight; ++i) {
    redrawNoise(i, j);
  }
}
void moveLeft() {
  jOffset = (jOffset + gridWidth - 1) % gridWidth;
  currentOffset.x += 1;

  int j = (gridWidth - jOffset - 1) % gridWidth;
  for (int i = 0; i < gridHeight; ++i) {
    redrawNoise(i, j);
  }
}
void moveUp() {
  iOffset = (iOffset + gridHeight - 1) % gridHeight;
  currentOffset.y -= 1;

  int i = (gridHeight - iOffset - 1) % gridHeight;
  for (int j = 0; j < gridWidth; ++j) {
    redrawNoise(i, j);
  }
}
void moveDown() {
  iOffset = (iOffset + 1) % gridHeight;
  currentOffset.y += 1;

  int i = (gridHeight - iOffset) % gridHeight;
  for (int j = 0; j < gridWidth; ++j) {
    redrawNoise(i, j);
  }
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
                for (auto& lig : grid) {
                  for (auto& item : lig) {
                      item.toggleWireframeMode();
                  }
                }
                break;
            case GLFW_KEY_H:
                //grid[0].updateZoomFactor(+0.1);
                break;
            case GLFW_KEY_G:
                //grid[0].updateZoomFactor(-0.1);
                break;
            case GLFW_KEY_RIGHT:
                moveRight();
                break;
            case GLFW_KEY_LEFT:
                moveLeft();
                break;
            case GLFW_KEY_UP:
                moveUp();
                break;
            case GLFW_KEY_DOWN:
                moveDown();
                break;
        }
    } else if(action == GLFW_RELEASE){
      keys[key] = false;
    }
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

    Init();

    // updates the window size with the framebuffer size
    // (on hidpi screens the framebuffer is bigger)
    glfwGetFramebufferSize(window, &window_width, &window_height);

    while(!glfwWindowShouldClose(window)){

        if (setupWindowBuffer) {
          // note: this is needed each time another buffer is binded and unbinded, hence the boolean
          SetupProjection(window, window_width, window_height);
          setupWindowBuffer = false;
        }

        Display();
        glfwSwapBuffers(window);
        glfwPollEvents();
        doMovement();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
