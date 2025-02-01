#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "framework/hyperMaths.h"
#include "graphics.cpp"

Direction cameraDirection = NONE;
Scene scene;

// Error callback for GLFW
void errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Window resize callback
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraDirection = FORWARD;
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraDirection = BACKWARD;
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraDirection = LEFT;
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraDirection = RIGHT;
    else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraDirection = UP;
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraDirection = DOWN;
    else
        cameraDirection = NONE;
}

int main() {
    //GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwSetErrorCallback(errorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(800, 600, "Non Euclidean Space", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set viewport and callbacks
    glViewport(0, 0, windowWidth, windowHeight);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Initialize OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // Build scene
    scene.Build();

    // Animation timing
    float lastFrame = 0.0f;
    const float dt = 0.1f;

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate frame timing
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        processInput(window);

        // Animation
        static float tend = 0;
        float tstart = tend;
        tend = glfwGetTime();

        for (float t = tstart; t < tend; t += dt) {
            float Dt = fmin(dt, tend - t);
            scene.Animate(t, t + Dt);
            scene.camera.move(Dt, cameraDirection);
        }

        // Render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        scene.Render();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glfwTerminate();
    return 0;
}
