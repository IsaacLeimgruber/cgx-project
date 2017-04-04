// glew must be before glfw
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// contains helper functions such as shader compiler
#include "icg_helper.h"

#include <glm/gtc/matrix_transform.hpp>
#include "grid/grid.h"
#include "framebuffer.h"
#include "trackball.h"
#include "screenquad/screenquad.h"
#include "perlin/perlin.h"

Grid grid;
FrameBuffer framebuffer;
Perlin perlin;
ScreenQuad screenquad;

int window_width = 1280;
int window_height = 960;
float zoom_factor = 0.005f;
float previousMouseY = 0.f;
bool rightMouseButtonIsUp = true;

const float OFFSET_QTY = 0.04f;


using namespace glm;

mat4 projection_matrix;
mat4 view_matrix;
mat4 old_view_matrix;
mat4 trackball_matrix;
mat4 old_trackball_matrix;
mat4 quad_model_matrix;

Trackball trackball;

void Init() {
    // sets background color
    glClearColor(0.0, 0.0, 0.0, 1.0 /*solid*/);
    perlin.Init();
    int framebuffer_texture_id = framebuffer.Init(1024, 1024, true);
    grid.Init(framebuffer_texture_id);
    screenquad.Init(window_width, window_height, framebuffer_texture_id);

    // enable depth test.
    glEnable(GL_DEPTH_TEST);

    view_matrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, -4.0f));

    trackball_matrix = IDENTITY_MATRIX;

    // scaling matrix to scale the cube down to a reasonable size.
    quad_model_matrix = translate(mat4(1.0f), vec3(0.0f, -0.25f, 0.0f));

    //Generate Perlin
    framebuffer.Bind();
        perlin.Draw();
    framebuffer.Unbind();
}


void Display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    grid.Draw(trackball_matrix * quad_model_matrix, view_matrix, projection_matrix);

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

void MouseButton(GLFWwindow* window, int button, int action, int mod) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double x_i, y_i;
        glfwGetCursorPos(window, &x_i, &y_i);
        vec2 p = TransformScreenCoords(window, x_i, y_i);
        trackball.BeingDrag(p.x, p.y);
        old_trackball_matrix = trackball_matrix;
        // Store the current state of the model matrix.
    }
}

void MousePos(GLFWwindow* window, double x, double y) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        vec2 p = TransformScreenCoords(window, x, y);
        trackball_matrix = trackball.Drag(p.x,p.y) * old_trackball_matrix;
    }

    // zoom
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if(rightMouseButtonIsUp){
            rightMouseButtonIsUp = false;
            old_view_matrix = view_matrix;
            previousMouseY = y;
        }
        view_matrix = translate(IDENTITY_MATRIX, vec3{0.0, 0.0, (y-previousMouseY) * zoom_factor}) * old_view_matrix;
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE){
        rightMouseButtonIsUp = true;
    }
}

// Gets called when the windows/framebuffer is resized.
void SetupProjection(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;

    cout << "Window has been resized to "
         << window_width << "x" << window_height << "." << endl;

    glViewport(0, 0, window_width, window_height);

    projection_matrix = glm::perspective(45.0f, (GLfloat)window_width / window_height, 0.1f, 100.0f);
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
        switch(key){
            case GLFW_KEY_F:
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
    }
}


int main(int argc, char *argv[]) {
    // GLFW Initialization
    if(!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(ErrorCallback);

    // hint GLFW that we would like an OpenGL 3 context (at least)
    // http://www.glfw.org/faq.html#how-do-i-create-an-opengl-30-context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // attempt to open the window: fails if required version unavailable
    // note some Intel GPUs do not support OpenGL 3.2
    // note update the driver of your graphic card
    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                          "Trackball", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // makes the OpenGL context of window current on the calling thread
    glfwMakeContextCurrent(window);

    // set the callback for escape key
    glfwSetKeyCallback(window, KeyCallback);

    // set the framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, SetupProjection);

    // set the mouse press and position callback
    glfwSetMouseButtonCallback(window, MouseButton);
    glfwSetCursorPosCallback(window, MousePos);

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
    }

    grid.Cleanup();
    perlin.Cleanup();
    framebuffer.Cleanup();

    // close OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
