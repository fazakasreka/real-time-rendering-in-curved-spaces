#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "scene.cpp"

Scene scene;
int windowWidth = 1200;
int windowHeight = 800;

Direction cameraDirection = NONE;
double mouseDeltaX = 0.0;
double mouseDeltaY = 0.0;
bool leftMousePressed = false;

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

    // Change curvature
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        Curvature::setHyperbolic();
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        Curvature::setEuclidean();
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        Curvature::setSpherical();

    //teleport to origin
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        scene.camera.setPosition(vec4(0.0, 0.5, 0.5, 1.0));
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftMousePressed = true;
        }
        else if (action == GLFW_RELEASE) {
            leftMousePressed = false;
        }
    }
}

// Mouse callback function
double mouseX = 0.0;
double mouseY = 0.0;
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (leftMousePressed) {
        float newMouseX = (xpos / windowWidth * 2.0) - 1.0;
        float newMouseY = 1.0 - (ypos / windowHeight * 2.0);
        mouseDeltaX = newMouseX - mouseX;
        mouseDeltaY = newMouseY - mouseY;
        mouseX = newMouseX;
        mouseY = newMouseY;
    }else{
        mouseDeltaX = 0.0;
        mouseDeltaY = 0.0;
    }
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

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Non Euclidean Space", nullptr, nullptr);
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
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // Initialize OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // Build scene
    scene.Build();
    scene.camera.updateAspectRatio(windowWidth, windowHeight);

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
            scene.camera.pan(mouseDeltaX, mouseDeltaY);
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
