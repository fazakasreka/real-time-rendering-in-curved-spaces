#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include "scene.cpp"

Scene scene;
int windowWidth = 1000;
int windowHeight = 600;

Direction cameraDirection = NONE;
double mouseDeltaX = 0.0;
double mouseDeltaY = 0.0;
bool leftMousePressed = false;

// Mouse position
double mouseX = 0.0;
double mouseY = 0.0;

// Input handling
EM_BOOL mouseClickCallback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN) {
        if (e->button == 0) { // Left button
            leftMousePressed = true;
        }
    } else if (eventType == EMSCRIPTEN_EVENT_MOUSEUP) {
        if (e->button == 0) { // Left button
            leftMousePressed = false;
        }
    }
    return EM_TRUE;
}

EM_BOOL mouseMoveCallback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE) {
        if (leftMousePressed) {
            mouseDeltaX = e->movementX/(float)windowWidth;
            mouseDeltaY = e->movementY/(float)windowHeight;
        }
    }
}

EM_BOOL keyCallback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
    if (eventType == EMSCRIPTEN_EVENT_KEYDOWN) {
       if(e->keyCode == 32) { // Space key teleport to origin
        scene.camera.setPosition(vec4(0.0, 0.5, 0.5, 1.0));
       }
       if(e->keyCode == 87) { // W key
        cameraDirection = FORWARD;
       }
       if(e->keyCode == 83) { // S key
        cameraDirection = BACKWARD;
       }
       if(e->keyCode == 65) { // A key
        cameraDirection = LEFT;
       }
       if(e->keyCode == 68) { // D key
        cameraDirection = RIGHT;
       }
       if(e->keyCode == 81) { // Q key
        cameraDirection = UP;
       }
       if(e->keyCode == 69) { // E key
        cameraDirection = DOWN;
       }
       if(e->keyCode == 49) { // 1 key
        Curvature::setHyperbolic();
       }
       if(e->keyCode == 50) { // 2 key
        Curvature::setEuclidean(); 
       }
       if(e->keyCode == 51) { // 3 key
        Curvature::setSpherical();
       }
    } else if (eventType == EMSCRIPTEN_EVENT_KEYUP) {
        cameraDirection = NONE;
    }
    return EM_TRUE;
}

float lastFrame = 0.0f;
const float dt = 0.1f;

void main_loop() {
    // Calculate frame timing
    float currentFrame = emscripten_get_now() / 1000.0f;
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Animation update
    static float tend = 0;
    float tstart = tend;
    tend = currentFrame;

    for (float t = tstart; t < tend; t += dt) {
        float Dt = std::fmin(dt, tend - t);
        scene.Animate(t, t + Dt);
        scene.camera.pan(mouseDeltaX, mouseDeltaY);
        scene.camera.move(Dt, cameraDirection);
    }

    // Render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    scene.Render();
}

int main() {
    // Emscripten specific initialization
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.majorVersion = 2; // Request WebGL 2.0
    attr.minorVersion = 0;
    attr.alpha = false;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attr);
    if (context <= 0) {
        std::cerr << "Failed to create WebGL 2.0 context!" << std::endl;
        return -1;
    }
    emscripten_webgl_make_context_current(context);

    // Set the canvas size
    emscripten_set_canvas_element_size("#canvas", windowWidth, windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    // Set up input handling
    emscripten_set_mousedown_callback("#canvas", nullptr, true, mouseClickCallback);
    emscripten_set_mouseup_callback("#canvas", nullptr, true, mouseClickCallback);
    emscripten_set_mousemove_callback("#canvas", nullptr, true, mouseMoveCallback);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, keyCallback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, keyCallback);

    // Initialize WebGL state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Build scene
    scene.Build();
    scene.camera.updateAspectRatio(windowWidth, windowHeight);

    // Set the main loop
    emscripten_set_main_loop(main_loop, 0, true);

    return 0;
}