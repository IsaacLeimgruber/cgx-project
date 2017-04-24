class FrameCounter {
    const GLfloat SEC_DURATION = 1.0;
    GLfloat deltaTime_ = 0.0f;	// Time between current frame and last frame
    GLfloat lastFrame_ = 0.0f;  	// Time of last frame
    GLfloat lastSec_ = 0.0;
    GLuint frameCount_ = 0;

 public:
    void countNewFrame() {
      GLfloat currentFrame = glfwGetTime();
      deltaTime_ = currentFrame - lastFrame_;
      lastFrame_ = currentFrame;
      if(currentFrame - lastSec_ > SEC_DURATION){
        std::cout << "Frames per second: " << frameCount_ << std::endl;
        lastSec_ = currentFrame;
        frameCount_ = 0;
      }
      frameCount_++;
    }
    float deltaTime() {
      return deltaTime_;
    }
};
