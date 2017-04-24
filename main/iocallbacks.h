namespace iocallbacks {
  bool keys[1024];
  bool firstMouse = false;
  float lastX = 0.0f;
  float lastY = 0.0f;

  void mouseCallback(GLFWwindow* window, double xpos, double ypos);
  void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
  void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  void doMovement();

  void init(GLFWwindow* window) {
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    for(auto& key : keys){
      key = false;
    }
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
    float deltaTime = frameCounter.deltaTime();
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

}
