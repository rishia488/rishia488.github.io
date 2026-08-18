///////////////////////////////////////////////////////////////////////////////
// MainCode.cpp
// ============
// Initializes OpenGL resources and runs the application loop.
//
// Enhanced by Arishia Jackson for CS 499 Milestone Two
///////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>

#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShaderManager.h"

namespace
{
    const char* const WINDOW_TITLE = "CS 499 Enhanced CS 330 Final Project";

    GLFWwindow* g_Window = nullptr;
    SceneManager* g_SceneManager = nullptr;
    ShaderManager* g_ShaderManager = nullptr;
    ViewManager* g_ViewManager = nullptr;
}

bool InitializeGLFW();
bool InitializeGLEW();
void CleanupApplication();

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    if (!InitializeGLFW())
    {
        return EXIT_FAILURE;
    }

    g_ShaderManager = new ShaderManager();
    if (g_ShaderManager == nullptr)
    {
        std::cerr << "ERROR: The shader manager could not be created." << std::endl;
        CleanupApplication();
        return EXIT_FAILURE;
    }

    g_ViewManager = new ViewManager(g_ShaderManager);
    if (g_ViewManager == nullptr)
    {
        std::cerr << "ERROR: The view manager could not be created." << std::endl;
        CleanupApplication();
        return EXIT_FAILURE;
    }

    g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);
    if (g_Window == nullptr)
    {
        std::cerr << "ERROR: Application startup stopped because the display window is unavailable." << std::endl;
        CleanupApplication();
        return EXIT_FAILURE;
    }

    if (!InitializeGLEW())
    {
        CleanupApplication();
        return EXIT_FAILURE;
    }

    g_ShaderManager->LoadShaders(
        "../../Utilities/shaders/vertexShader.glsl",
        "../../Utilities/shaders/fragmentShader.glsl");
    g_ShaderManager->use();

    g_SceneManager = new SceneManager(g_ShaderManager);
    if (g_SceneManager == nullptr)
    {
        std::cerr << "ERROR: The scene manager could not be created." << std::endl;
        CleanupApplication();
        return EXIT_FAILURE;
    }

    g_SceneManager->PrepareScene();

    std::cout << "\nCS 499 Enhanced Controls\n"
              << "W/A/S/D: Move camera\n"
              << "Q/E: Move down/up\n"
              << "Mouse: Look around\n"
              << "Mouse wheel: Change camera speed\n"
              << "P/O: Perspective/orthographic projection\n"
              << "R: Reset camera\n"
              << "1/2: Turn primary light on/off\n"
              << "3/4: Increase/decrease light intensity\n"
              << "Escape: Exit\n" << std::endl;

    while (!glfwWindowShouldClose(g_Window))
    {
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        g_ViewManager->PrepareSceneView();
        g_SceneManager->ProcessLightingControls(g_Window);
        g_SceneManager->RenderScene();

        glfwSwapBuffers(g_Window);
        glfwPollEvents();
    }

    CleanupApplication();
    return EXIT_SUCCESS;
}

bool InitializeGLFW()
{
    if (glfwInit() != GLFW_TRUE)
    {
        std::cerr << "ERROR: GLFW initialization failed." << std::endl;
        return false;
    }

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    return true;
}

bool InitializeGLEW()
{
    glewExperimental = GL_TRUE;
    const GLenum glewResult = glewInit();

    if (glewResult != GLEW_OK)
    {
        std::cerr << "ERROR: GLEW initialization failed: "
                  << glewGetErrorString(glewResult) << std::endl;
        return false;
    }

    std::cout << "INFO: OpenGL successfully initialized.\n"
              << "INFO: OpenGL version: " << glGetString(GL_VERSION) << "\n" << std::endl;
    return true;
}

void CleanupApplication()
{
    delete g_SceneManager;
    g_SceneManager = nullptr;

    delete g_ViewManager;
    g_ViewManager = nullptr;

    delete g_ShaderManager;
    g_ShaderManager = nullptr;

    if (g_Window != nullptr)
    {
        glfwDestroyWindow(g_Window);
        g_Window = nullptr;
    }

    glfwTerminate();
}
