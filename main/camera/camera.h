/*
 * Code taken and adapted from https://learnopengl.com/#!Getting-started/Camera
 */

#pragma once

// Std. Includes
#include <vector>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UPWARD,
    DOWNWARD,
    ROTATE_UP,
    ROTATE_DOWN,
    ROTATE_LEFT,
    ROTATE_RIGHT
};

// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Default camera values
    static constexpr GLfloat YAW        =  45.0f;
    static constexpr GLfloat PITCH      =  0.0f;
    static constexpr GLfloat SPEED      =  2.0f;
    static constexpr GLfloat SENSITIVTY =  0.05f;
    static constexpr GLfloat FOV        =  60.0f;
    static constexpr GLfloat MAX_FOV    =  90.0f;
    static constexpr GLfloat MIN_FOV    =  1.0f;

    // Camera Attributes
    vec3 Position;
    vec3 Front;
    vec3 Up;
    vec3 Right;
    vec3 WorldUp;
    // Eular Angles
    GLfloat Yaw;
    GLfloat Pitch;
    // Camera options
    GLfloat MovementSpeed;
    GLfloat MouseSensitivity;
    GLfloat Fov;

    // Constructor with vectors
    Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), GLfloat speed = SPEED, GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(vec3(-1.0f, 0.0f, 0.0f)), MovementSpeed(speed), MouseSensitivity(SENSITIVTY), Fov(FOV)
    {
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateCameraVectors();
    }
    // Constructor with scalar values
    Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Fov(FOV)
    {
        this->Position = vec3(posX, posY, posZ);
        this->WorldUp = vec3(upX, upY, upZ);
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateCameraVectors();
    }

    // Returns the view matrix calculated using EulEr Angles and the LookAt Matrix
    mat4 GetViewMatrix()
    {
        return lookAt(this->Position, this->Position + this->Front, this->WorldUp);
    }

    mat4 GetMirroredViewMatrix(float mirrorHeight)
    {
        vec3 mirrorPos = vec3(this->Position.x, -(this->Position.y) + 2.0 * mirrorHeight, this->Position.z);
        vec3 mirrorFront = vec3(this->Front.x, -this->Front.y, this->Front.z);

        return lookAt(mirrorPos, mirrorPos + mirrorFront, this->WorldUp);
    }

    const vec3& getPos() {
        return this->Position;
    }

    const vec3& getFront() {
        return this->Front;
    }

    void setPos(vec3 pos) {
        this->Position = pos;
    }

    void setFront(vec3 target){
        this->Front = normalize(target - this->Position);
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->MovementSpeed * deltaTime;
        if (direction == FORWARD)
            this->Position += this->Front * velocity;
        if (direction == BACKWARD)
            this->Position -= this->Front * velocity;
        if (direction == LEFT)
            this->Position -= this->Right * velocity;
        if (direction == RIGHT)
            this->Position += this->Right * velocity;
        if (direction == UPWARD)
            this->Position += this->WorldUp * velocity;
        if (direction == DOWNWARD)
            this->Position -= this->WorldUp * velocity;
        if (direction == ROTATE_DOWN)
            this->Pitch += .5;
        if (direction == ROTATE_UP)
            this->Pitch -= .5;
        if (direction == ROTATE_LEFT)
            this->Yaw -= .5;
        if (direction == ROTATE_RIGHT)
            this->Yaw += .5;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= this->MouseSensitivity;
        yoffset *= this->MouseSensitivity;

        this->Yaw   += xoffset;
        this->Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (this->Pitch > 89.0f)
                this->Pitch = 89.0f;
            if (this->Pitch < -89.0f)
                this->Pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Eular angles
        this->updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(GLfloat yoffset)
    {
        cout << Fov << endl;
        if (this->Fov >= MIN_FOV && this->Fov <= MAX_FOV)
            this->Fov -= yoffset;
        if (this->Fov <= MIN_FOV)
            this->Fov = MIN_FOV;
        if (this->Fov >= MAX_FOV)
            this->Fov = MAX_FOV;
    }

    void debug() {
        cout << "front: " << this->Front << endl;
        cout << "up: " << this->Up << endl;
        cout << "right: " << this->Right << endl;
        cout << "position: " << this->Position << endl;
        cout << "yaw:" << this->Yaw << endl;
        cout << "pitch:" << this->Pitch << endl;
    }


private:
    // Calculates the front vector from the Camera's (updated) Eular Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        vec3 front;
        front.x = cos(radians(this->Yaw)) * cos(radians(this->Pitch));
        front.y = sin(radians(this->Pitch));
        front.z = sin(radians(this->Yaw)) * cos(radians(this->Pitch));
        this->Front = normalize(front);
        // Also re-calculate the Right and Up vector
        this->Right = normalize(cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        this->Up    = normalize(cross(this->Right, this->Front));
    }
};

