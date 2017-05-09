// glew must be before glfw
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "icg_helper.h"
#include <glm/gtc/matrix_transform.hpp>
#include "gridmesh.h"
#include "terrain/terrain.h"
#include "framebuffer.h"
#include "screenquad/screenquad.h"
#include "large_scene.h"
#include "camera/camera.h"
#include "camera/fractionalview.h"
#include "light/light.h"
#include "material/material.h"
#include "skydome/skyDome.h"
#include "perlin/perlin_texture.h"

using namespace glm;

LargeScene scene;
SkyDome skyDome;
Perlin perlin;
Camera camera;
ColorFBO cloudBuffer;
PerlinTexture perlinTexture;
ColorAndDepthFBO screenQuadBuffer, reflectionBuffer;
DepthFBO shadowBuffer;
ScreenQuad screenquad;
CloudPlane cloudPlane;
Light light;
Material material;

bool keys[1024];
bool useContinuousPerlinMoves = true;
bool firstMouse = false;
bool wireframeDebugEnabled = false;
// Window size in screen coordinates
int window_width_sc;
int window_height_sc;
// OpenGL window dimensions in pixels, ignored if fullscreen
int window_width = 1600;
int window_height = 1200;
const bool FULLSCREEN = false;
// native render dimensions in pixels
int screenWidth = 1440;
int screenHeight = 1080;
float lastX = 0.0f;
float lastY = 0.0f;

const float OFFSET_QTY = 0.04f;

mat4 projection_matrix, view_matrix, mirrored_view_matrix, quad_model_matrix;
mat4 depth_projection_matrix, depth_bias_matrix, depth_view_matrix, depth_model_matrix, depth_mvp;
mat4 MVP, mMVP, MV, mMV, NORMALM, mNORMALM;

mat4 biasMatrix = mat4(
            0.5, 0.0, 0.0, 0.0,
            0.0, 0.5, 0.0, 0.0,
            0.0, 0.0, 0.5, 0.0,
            0.5, 0.5, 0.5, 1.0
            );

const GLfloat SEC_DURATION = 1.0;
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame
GLfloat lastSec = 0.0;
GLuint frameCount = 0;

FractionalView fractionalView;

void Init() {
    glClearColor(0.0f, 0.8f, 1.0f, 1.0f);

    camera   = Camera{vec3(0.0, 2.5, 0.0)};
    light    = Light{vec3(0.0, 2.0, -4.0)};
    material = Material{};

    // buffers must be initialized in that order
    int screenQuadBuffer_texture_id = screenQuadBuffer.Init(screenWidth, screenHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT, false, false);
    scene.initPerlin();
    int cloudBuffer_texture_id = cloudBuffer.Init(1024, 1024, GL_RGB32F, GL_RGB, GL_FLOAT, true);
    int reflectionBuffer_texture_id = reflectionBuffer.Init(screenWidth, screenHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT, true, true);
    int shadowBuffer_texture_id     = shadowBuffer.Init(2048, 2048, GL_DEPTH_COMPONENT16, GL_FLOAT);

    perlin.Init();

    screenquad.Init(screenQuadBuffer_texture_id, 0);
    cloudPlane.Init(cloudBuffer_texture_id, 0);
    scene.init(shadowBuffer_texture_id, reflectionBuffer_texture_id, &light);
    skyDome.Init();
    skyDome.useLight(&light);

    cloudBuffer.Bind();
        perlin.Draw(vec2(0.0, 0.0));
    cloudBuffer.Unbind();

    float skyDomeRadius = skyDome.getRadius();
    float sceneHalfMaxSize = scene.maximumExtent() / 2.0;

    view_matrix             = camera.GetViewMatrix();
    depth_projection_matrix = glm::ortho(-sceneHalfMaxSize, sceneHalfMaxSize, -sceneHalfMaxSize, sceneHalfMaxSize, skyDomeRadius - sceneHalfMaxSize, skyDomeRadius + sceneHalfMaxSize);
    depth_view_matrix       = lookAt(light.getPos(), vec3(0.0,0.0,0.0), vec3(0, 0, 0));
    depth_model_matrix      = IDENTITY_MATRIX;
    depth_mvp               = depth_projection_matrix * depth_view_matrix * depth_model_matrix;
    depth_bias_matrix       = biasMatrix * depth_mvp;
    quad_model_matrix       = IDENTITY_MATRIX;


    for(auto& key : keys){
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

    //Compute matrices

    view_matrix = camera.GetViewMatrix();
    mirrored_view_matrix = camera.GetMirroredViewMatrix(0.0f);

    MV = view_matrix * quad_model_matrix;
    MVP = projection_matrix * MV;
    NORMALM = inverse(transpose(MV));

    //mirror matrices
    mMV = mirrored_view_matrix * quad_model_matrix;
    mMVP = projection_matrix * mMV;
    mNORMALM = inverse(transpose(mMV));

    //shadow matrices
    depth_view_matrix = lookAt(light.getPos(), vec3(0.0,0.0,0.0), vec3(0, 1, 0));
    depth_model_matrix = IDENTITY_MATRIX;
    depth_mvp = depth_projection_matrix * depth_view_matrix * depth_model_matrix;
    depth_bias_matrix = biasMatrix * depth_mvp;

    shadowBuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    scene.draw(MVP, MV, IDENTITY_MATRIX, depth_mvp, fractionalView, false, true);
    shadowBuffer.Unbind();

    // reflection computation
    reflectionBuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    skyDome.Draw(mirrored_view_matrix, projection_matrix, camera.getPos());
    cloudPlane.Draw(mMVP);
    scene.draw(mMVP, mMV, mNORMALM, depth_bias_matrix, fractionalView, true, false);
    reflectionBuffer.Unbind();

    screenQuadBuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    skyDome.Draw(view_matrix, projection_matrix, camera.getPos());
    cloudPlane.Draw(MVP);
    scene.draw(MVP, MV, NORMALM, depth_bias_matrix, fractionalView, false, false);
    screenQuadBuffer.Unbind();

    glViewport(0, 0, window_width, window_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    screenquad.Draw();
    frameCount++;
}

// transforms glfw screen coordinates into normalized OpenGL coordinates.
vec2 TransformScreenCoords(GLFWwindow* window, int x, int y) {

    return vec2(2.0f * (float)x / window_width_sc - 1.0f,
                1.0f - 2.0f * (float)y / window_height_sc);
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
    projection_matrix = perspective(glm::radians(camera.Fov), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);
}

// Gets called when the windows/framebuffer is resized.
void SetupProjection(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;

    cout << "Window has been resized to "
         << window_width << "x" << window_height << "." << endl;

    glViewport(0, 0, window_width, window_height);

    projection_matrix = glm::perspective(glm::radians(camera.Fov), (GLfloat)screenWidth / screenHeight, 0.1f, 100.0f);

    glfwGetWindowSize(window, &window_width_sc, &window_height_sc);
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
            scene.toggleWireFrame();
            break;
        case GLFW_KEY_H:
            fractionalView.zoom += 0.1f;
            break;
        case GLFW_KEY_G:
            fractionalView.zoom -= 0.1f;
            break;
        case GLFW_KEY_N:
            scene.toggleDebugMode();
            break;
        case GLFW_KEY_Q:
            useContinuousPerlinMoves = !useContinuousPerlinMoves;
            break;
        case GLFW_KEY_RIGHT:
            if (!useContinuousPerlinMoves)
                scene.moveCols(LargeScene::DOWN);
            break;
        case GLFW_KEY_LEFT:
            if (!useContinuousPerlinMoves)
                scene.moveCols(LargeScene::UP);
            break;
        case GLFW_KEY_UP:
            if (!useContinuousPerlinMoves)
                scene.moveRows(LargeScene::DOWN);
            break;
        case GLFW_KEY_DOWN:
            if (!useContinuousPerlinMoves)
                scene.moveRows(LargeScene::UP);
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

    if (useContinuousPerlinMoves) {
        if(keys[GLFW_KEY_RIGHT]) {
            scene.moveNoise(vec2(OFFSET_QTY, 0.0));
        }
        if(keys[GLFW_KEY_LEFT]) {
            scene.moveNoise(vec2(-OFFSET_QTY, 0.0));
        }
        if(keys[GLFW_KEY_UP]) {
            scene.moveNoise(vec2(0.0, OFFSET_QTY));
        }
        if(keys[GLFW_KEY_DOWN]) {
            scene.moveNoise(vec2(0.0, -OFFSET_QTY));
        }
    }
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
    glfwWindowHint(GLFW_REFRESH_RATE, 0);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //Only for MacOSX
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    GLFWmonitor* monitor = NULL;

    if(FULLSCREEN){
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        window_width = mode->width;
        window_height = mode->height;
    }

    window = glfwCreateWindow(window_width, window_height,"Procedural terrain", monitor, NULL);

    if(!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // makes the OpenGL context of window current on the calling thread
    glfwMakeContextCurrent(window);

    // Cursor is captured and hidden
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(0);

    // enable depth test.
    glEnable(GL_DEPTH_TEST);
    // enable culling
    glCullFace(GL_BACK);
    // Enable transparent materials
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    scene.cleanup();
    reflectionBuffer.Cleanup();
    shadowBuffer.Cleanup();

    // close OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
