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
#include "scene_controler.h"
#include "camera/camera.h"
#include "camera/fractionalview.h"
#include "light/light.h"
#include "material/material.h"
#include "skyDome/skyDome.h"
#include "blurQuad/blurquad.h"
#include "large_scene.h"
#include "bezier/BezierCurve.h"
#include "model/model.h"

using namespace glm;

float grid_size = 1.0f;
LargeScene scene(2 * grid_size);
SceneControler sceneControler(scene, grid_size, grid_size);
SkyDome skyDome;
Camera camera;
ColorAndDepthFBO screenQuadBuffer, reflectionBuffer, screenQuadBufferPostProcessing;
DoubleColorAndDepthFBO bloomHDRBuffer;
DepthFBO shadowBuffer;
ScreenQuad screenquad;
BlurQuad blurQuad;
Light light;
Material material;

Perlin perlin;

int postProcessingTextureId;
float perlinTextureSize = 512;

bool keys[1024];
bool firstMouse = false;
bool wireframeDebugEnabled = false;
bool enableBlurPostProcess = true;

// Window size in screen coordinates
int window_width_sc;
int window_height_sc;
// OpenGL window dimensions in pixels, ignored if fullscreen
int window_width = 1600;
int window_height = 1200;
const bool FULLSCREEN = true;
// native render dimensions in pixels
int screenWidth = 1440;
int screenHeight = 1080;
float lastX = 0.0f;
float lastY = 0.0f;

const float DISPLACEMENT_TIME = 10.f;
const float OFFSET_QTY = 0.04f;
float start_time1;
float start_time2;

mat4 projection_matrix, view_matrix, mirrored_view_matrix, quad_model_matrix;
mat4 depth_projection_matrix, depth_bias_matrix, depth_view_matrix, depth_model_matrix, depth_mvp;
mat4 MVP, mMVP, MV, mMV, NORMALM, mNORMALM;
//mat4 shipM, mShipMVP, mShipMV, mShipNORMALM;

bool untoggleAllBezier = false;
bool toggleBezier1 = true;
bool toggleBezier2 = true;

const float bezierTime1 = 10.f;
bool startedCurve1 = false;

vec3 bezier1Start = vec3(10., 5., 0.);
vec3 lastBezierPos = bezier1Start;
BezierCurve bezier1({
                       bezier1Start,
                       vec3(7.,5.f, 7.),
                       vec3(0.,5.f, 10.)
                   });


const float bezierTime2 = 10.f;
bool startedCurve2 = false;
vec3 bezier2Start = vec3(1., 10., 1.);
BezierCurve bezier2({
                        bezier2Start,
                        vec3(2.,9.f, 2.),
                        vec3(3.,8.f, 3.),
                        vec3(4.,2.f, 4.),
                        vec3(10.,2.f, 6.5),
                        vec3(15.,1.f, 15.),
                        vec3(19.,0.5f, 25.)
                   });


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
LargeScene::TileSet visibleTiles;
FractionalView fractionalView;

//Model mightyShip("yacht.3ds");
//GLuint mightyShipShaderProgram;

void Init() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    if(untoggleAllBezier){
            toggleBezier1 = false;
            toggleBezier2 = false;
        }

    camera   = Camera{vec3(0.0, 3.0, 0.0), vec3(0.0f, 1.0f, 0.0f), grid_size * Camera::SPEED};
    light    = Light{vec3(0.0, 2.0, -4.0)};
    material = Material{};

    // buffers must be initialized in that order
    int screenQuadBuffer_texture_id = screenQuadBuffer.Init(screenWidth, screenHeight, GL_RGB16F, GL_RGB, GL_FLOAT, false, false);
    bloomHDRBuffer.Init(screenWidth, screenHeight, GL_RGB16F, GL_RGB, GL_FLOAT, true, false);
    scene.initHeightMap(perlinTextureSize, perlinTextureSize);
    reflectionBuffer.Init(screenWidth, screenHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT, true, true);
    screenQuadBufferPostProcessing.Init(screenWidth, screenHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT, true, true);

    int shadowBuffer_texture_id     = shadowBuffer.Init(4096, 4096, GL_DEPTH_COMPONENT32, GL_UNSIGNED_INT);

    //screenquad.Init(bloomHDRBuffer.getColorTexture(0), screenQuadBuffer_texture_id);
    screenquad.Init(bloomHDRBuffer.getColorTexture(0), screenQuadBuffer_texture_id);
    blurQuad.Init(screenWidth, screenHeight, reflectionBuffer.getColorTexture());
    scene.init(shadowBuffer_texture_id, reflectionBuffer.getColorTexture(), &light);
    skyDome.Init();
    skyDome.useLight(&light);

    //mightyShipShaderProgram = icg_helper::LoadShaders("yacht_vshader.glsl", "yacht_fshader.glsl");
    //mightyShip.Init(mightyShipShaderProgram, shadowBuffer_texture_id);
    //mightyShip.useLight(&light);

    float skyDomeRadius = skyDome.getRadius();
    float sceneHalfMaxSize = scene.maximumExtent() / 2.0;

    view_matrix             = camera.GetViewMatrix();
    depth_projection_matrix = glm::ortho(-sceneHalfMaxSize, sceneHalfMaxSize, -sceneHalfMaxSize, sceneHalfMaxSize, skyDomeRadius - sceneHalfMaxSize, skyDomeRadius + sceneHalfMaxSize);
    depth_view_matrix       = lookAt(light.getPos(), vec3(0.0,0.0,0.0), vec3(0, 0, 0));
    quad_model_matrix       = glm::scale(IDENTITY_MATRIX, grid_size * vec3(1.0f, 1.0f / grid_size, 1.0f));
    depth_mvp               = depth_projection_matrix * depth_view_matrix * depth_model_matrix;
    depth_bias_matrix       = biasMatrix * depth_mvp;
    depth_model_matrix      = quad_model_matrix;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);

    for(auto& key : keys){
        key = false;
    }
}

void computeReflections(LargeScene::TileSet const& visibleTiles);
void computeBloom();
void drawMightyShip(glm::mat4 const& , glm::mat4 const&, glm::mat4 const& , glm::mat4 const& );

void Display() {
    glClear(GL_DEPTH_BUFFER_BIT);

    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if(currentFrame - lastSec > SEC_DURATION){
        std::cout << "Frames per second: " << frameCount << std::endl;
        lastSec = currentFrame;
        frameCount = 0;
    }

    scene.writeVisibleTilesOnly(visibleTiles, camera.getPos(), camera.getFront());

    //Compute matrices
    view_matrix = camera.GetViewMatrix();
    mirrored_view_matrix = camera.GetMirroredViewMatrix(0.0f);



    MV = view_matrix * quad_model_matrix;
    MVP = projection_matrix * MV;
    NORMALM = inverse(transpose(MV));

    //shadow matrices
    depth_view_matrix = lookAt(light.getPos(), vec3(0.0,0.0,0.0), vec3(0, 1, 0));
    depth_model_matrix = IDENTITY_MATRIX;
    depth_mvp = depth_projection_matrix * depth_view_matrix * depth_model_matrix;
    depth_bias_matrix = biasMatrix * depth_mvp;

    shadowBuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    scene.drawMountains(MVP, MV, IDENTITY_MATRIX, depth_mvp, fractionalView, false, true);
    glEnable(GL_CULL_FACE);
    shadowBuffer.Unbind();

    computeReflections(visibleTiles);

    bloomHDRBuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    skyDome.Draw(quad_model_matrix, view_matrix, projection_matrix, camera.getPos());
    scene.drawMountainTiles(visibleTiles, MVP, MV, NORMALM, depth_bias_matrix, fractionalView, false);
    scene.drawWaterTiles(visibleTiles, MVP, MV, NORMALM, depth_bias_matrix, fractionalView);
    scene.drawGrassTiles(visibleTiles, projection_matrix * view_matrix,
                         vec2(camera.getPos().x, camera.getPos().z));
    scene.drawModels(MVP, MV, depth_bias_matrix);
    //glm::mat4 shipMV = view_matrix * shipM;
    //glm::mat4 shipMVP = projection_matrix * shipMV;
    //glm::mat4 shipNORMALM = inverse(transpose(shipMV));

    //glDisable(GL_CULL_FACE);
    //mightyShip.Draw(shipMVP, shipMV, shipNORMALM, depth_bias_matrix);
    //glEnable(GL_CULL_FACE);

    // END OF MODEL LOADING INTEGRATION
    bloomHDRBuffer.Unbind();

    computeBloom();

    glViewport(0, 0, window_width, window_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    screenquad.Draw();
    frameCount++;
}


void computeReflections(LargeScene::TileSet const& visibleTiles) {

    //mirror matrices
    mMV = mirrored_view_matrix * quad_model_matrix;
    mMVP = projection_matrix * mMV;
    mNORMALM = inverse(transpose(mMV));

    //mShipMV = mirrored_view_matrix * shipM;
    //mShipMVP = projection_matrix * mShipMV;
    //mShipNORMALM = inverse(transpose(mShipMV));

    reflectionBuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    skyDome.Draw(quad_model_matrix, mirrored_view_matrix, projection_matrix, camera.getPos());
    scene.drawMountainTiles(visibleTiles, mMVP, mMV, mNORMALM, depth_bias_matrix, fractionalView, true);
    scene.drawModels(mMVP, mMV, depth_bias_matrix, true);
    //glDisable(GL_CULL_FACE);
    //mightyShip.Draw(mShipMVP, mShipMV, mShipNORMALM, depth_bias_matrix, true);
    //glEnable(GL_CULL_FACE);
    reflectionBuffer.Unbind();

    //Code below performs blur on reflection
    if(enableBlurPostProcess){
        blurQuad.setRenderingPassNumber(0);
        blurQuad.updateTextureId(reflectionBuffer.getColorTexture());

        screenQuadBufferPostProcessing.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        blurQuad.Draw(1.0);
        screenQuadBufferPostProcessing.Unbind();

        blurQuad.setRenderingPassNumber(1);
        blurQuad.updateTextureId(screenQuadBufferPostProcessing.getColorTexture());

        reflectionBuffer.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        blurQuad.Draw(1.0);
        reflectionBuffer.Unbind();
    }
}

void computeBloom() {

    blurQuad.setRenderingPassNumber(0);
    blurQuad.updateTextureId(bloomHDRBuffer.getColorTexture(1));

    screenQuadBufferPostProcessing.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    blurQuad.Draw(0.5);
    screenQuadBufferPostProcessing.Unbind();

    blurQuad.setRenderingPassNumber(1);
    blurQuad.updateTextureId(screenQuadBufferPostProcessing.getColorTexture());

    screenQuadBuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    blurQuad.Draw(0.5);
    screenQuadBuffer.Unbind();

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
    projection_matrix = perspective(glm::radians(camera.Fov), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 5000.0f);
}

// Gets called when the windows/framebuffer is resized.
void SetupProjection(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;

    cout << "Window has been resized to "
         << window_width << "x" << window_height << "." << endl;

    glViewport(0, 0, window_width, window_height);

    projection_matrix = glm::perspective(glm::radians(camera.Fov), (GLfloat)screenWidth / screenHeight, 0.1f, 5000.0f);

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
        case GLFW_KEY_B:
            enableBlurPostProcess = !enableBlurPostProcess;
            break;
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
        case GLFW_KEY_U:
            screenquad.updateExposure(-0.2);
            break;
        case GLFW_KEY_I:
            screenquad.updateExposure(0.2);
            break;
        case GLFW_KEY_O:
            screenquad.updateGamma(-0.1);
            break;
        case GLFW_KEY_P:
            screenquad.updateGamma(0.1);
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

    float bezierTimeC1 = (glfwGetTime() - start_time1)/bezierTime1;
    float bezierTimeC2 = (glfwGetTime() - start_time2)/bezierTime2;
    if(bezierTimeC1 <= 1.f && toggleBezier1 && startedCurve1){
        vec3 bezierPos = bezier1.getPoint(bezierTimeC1);
        vec3 deltaPos = bezierPos - lastBezierPos;
        lastBezierPos = bezierPos;

        camera.move(deltaPos);
        vec3 radial_view = normalize(cross(normalize(deltaPos), vec3(0.f, 1.f, 0.f)) - vec3(0.f, 1.f, 0.f));
        camera.setFront(radial_view);

    }

    if(0 <= bezierTimeC2 && bezierTimeC2 <= 1.f  && toggleBezier2 && startedCurve2){
        vec3 bezierPos = bezier2.getPoint(bezierTimeC2);
        vec3 deltaPos = bezierPos - lastBezierPos;
        lastBezierPos = bezierPos;

        camera.move(deltaPos);
        vec3 radial_view = cross(deltaPos, vec3(0.f, 1.f, 0.f));
        camera.setFront(normalize(radial_view - vec3(0.f, 1.f, 0.f)));
        camera.setFront(deltaPos);
    }

    if(!startedCurve1 && toggleBezier1){
        start_time1 = glfwGetTime();
        lastBezierPos = bezier1Start;
        camera.setPos(bezier1Start);
        lastBezierPos = bezier1Start;
        startedCurve1 = true;
    }

    if(!startedCurve2 && (glfwGetTime() - start_time1) > bezierTime1 && toggleBezier2){
        start_time2 = glfwGetTime();
        camera.setPos(bezier2Start);
        lastBezierPos = bezier2Start;
        startedCurve2 = true;
    }


    vec2 actualPos = sceneControler.position();
    vec3 newPos = camera.getPos();
    float displacementX = newPos.x - actualPos.x;
    float displacementY = newPos.z - actualPos.y;
    sceneControler.move({displacementX, displacementY});
    vec2 updatedPos = sceneControler.position();
    scene.setCenter(updatedPos/grid_size);

    vec3 cameraPos = vec3(updatedPos.x, newPos.y, updatedPos.y);

    camera.setPos(cameraPos);
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
    glfwSwapInterval(1);

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
