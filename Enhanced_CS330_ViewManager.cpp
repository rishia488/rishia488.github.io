///////////////////////////////////////////////////////////////////////////////
// ViewManager.cpp
// ============
// Manages the camera, projection, and user navigation for the 3D scene.
//
// AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
// Enhanced by Arishia Jackson for CS 499 Milestone Two
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace
{
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;
    const char* g_ViewName = "view";
    const char* g_ProjectionName = "projection";

    const glm::vec3 DEFAULT_CAMERA_POSITION(0.0f, 5.0f, 12.0f);
    const glm::vec3 DEFAULT_CAMERA_FRONT(0.0f, -0.5f, -2.0f);
    const glm::vec3 DEFAULT_CAMERA_UP(0.0f, 1.0f, 0.0f);
    const float DEFAULT_CAMERA_ZOOM = 80.0f;
    const float DEFAULT_CAMERA_SPEED = 2.5f;
    const float MIN_CAMERA_SPEED = 0.5f;
    const float MAX_CAMERA_SPEED = 10.0f;

    Camera* g_pCamera = nullptr;

    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    bool gOrthographicProjection = false;
    bool gResetKeyWasPressed = false;
    float gYaw = -90.0f;
    float gPitch = 0.0f;
    float gCameraSpeed = DEFAULT_CAMERA_SPEED;
}

ViewManager::ViewManager(ShaderManager* pShaderManager)
    : m_pShaderManager(pShaderManager), m_pWindow(nullptr)
{
    g_pCamera = new Camera();
    ResetCamera();
}

ViewManager::~ViewManager()
{
    m_pShaderManager = nullptr;
    m_pWindow = nullptr;

    delete g_pCamera;
    g_pCamera = nullptr;
}

GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle, nullptr, nullptr);

    if (window == nullptr)
    {
        std::cerr << "ERROR: The OpenGL window could not be created." << std::endl;
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);
    glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Callback);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_pWindow = window;
    return window;
}

void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
    (void)window;

    if (g_pCamera == nullptr)
    {
        return;
    }

    const float xPos = static_cast<float>(xMousePos);
    const float yPos = static_cast<float>(yMousePos);

    if (gFirstMouse)
    {
        gLastX = xPos;
        gLastY = yPos;
        gFirstMouse = false;
    }

    float xOffset = xPos - gLastX;
    float yOffset = gLastY - yPos;
    gLastX = xPos;
    gLastY = yPos;

    const float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    gYaw += xOffset;
    gPitch += yOffset;
    gPitch = std::clamp(gPitch, -89.0f, 89.0f);

    glm::vec3 direction;
    direction.x = std::cos(glm::radians(gYaw)) * std::cos(glm::radians(gPitch));
    direction.y = std::sin(glm::radians(gPitch));
    direction.z = std::sin(glm::radians(gYaw)) * std::cos(glm::radians(gPitch));
    g_pCamera->Front = glm::normalize(direction);
}

void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
    (void)window;
    (void)xOffset;

    gCameraSpeed += static_cast<float>(yOffset) * 0.5f;
    gCameraSpeed = std::clamp(gCameraSpeed, MIN_CAMERA_SPEED, MAX_CAMERA_SPEED);

    std::cout << "INFO: Camera speed set to " << gCameraSpeed << std::endl;
}

void ViewManager::ResetCamera()
{
    if (g_pCamera == nullptr)
    {
        return;
    }

    g_pCamera->Position = DEFAULT_CAMERA_POSITION;
    g_pCamera->Front = DEFAULT_CAMERA_FRONT;
    g_pCamera->Up = DEFAULT_CAMERA_UP;
    g_pCamera->Zoom = DEFAULT_CAMERA_ZOOM;

    gYaw = -90.0f;
    gPitch = 0.0f;
    gCameraSpeed = DEFAULT_CAMERA_SPEED;
    gOrthographicProjection = false;
    gFirstMouse = true;
}

void ViewManager::ProcessKeyboardEvents()
{
    if (m_pWindow == nullptr || g_pCamera == nullptr)
    {
        return;
    }

    if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_pWindow, true);
    }

    const float speed = gCameraSpeed * gDeltaTime;
    const glm::vec3 right = glm::normalize(glm::cross(g_pCamera->Front, g_pCamera->Up));

    if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
        g_pCamera->Position += speed * g_pCamera->Front;
    if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
        g_pCamera->Position -= speed * g_pCamera->Front;
    if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
        g_pCamera->Position -= speed * right;
    if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
        g_pCamera->Position += speed * right;
    if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
        g_pCamera->Position -= speed * g_pCamera->Up;
    if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
        g_pCamera->Position += speed * g_pCamera->Up;

    if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
        gOrthographicProjection = false;
    if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
        gOrthographicProjection = true;

    const bool resetPressed = glfwGetKey(m_pWindow, GLFW_KEY_R) == GLFW_PRESS;
    if (resetPressed && !gResetKeyWasPressed)
    {
        ResetCamera();
        std::cout << "INFO: Camera reset to its default position." << std::endl;
    }
    gResetKeyWasPressed = resetPressed;
}

void ViewManager::PrepareSceneView()
{
    if (m_pWindow == nullptr || g_pCamera == nullptr)
    {
        std::cerr << "ERROR: The view cannot be prepared because the window or camera is unavailable." << std::endl;
        return;
    }

    const float currentFrame = static_cast<float>(glfwGetTime());
    gDeltaTime = currentFrame - gLastFrame;
    gLastFrame = currentFrame;

    ProcessKeyboardEvents();

    const glm::mat4 view = g_pCamera->GetViewMatrix();
    glm::mat4 projection;

    if (gOrthographicProjection)
    {
        const float scale = 8.0f;
        const float aspectAdjustment = static_cast<float>(WINDOW_HEIGHT) / static_cast<float>(WINDOW_WIDTH);
        projection = glm::ortho(-scale, scale, -scale * aspectAdjustment, scale * aspectAdjustment, 0.1f, 100.0f);
    }
    else
    {
        projection = glm::perspective(
            glm::radians(g_pCamera->Zoom),
            static_cast<GLfloat>(WINDOW_WIDTH) / static_cast<GLfloat>(WINDOW_HEIGHT),
            0.1f,
            100.0f);
    }

    if (m_pShaderManager == nullptr)
    {
        std::cerr << "ERROR: The view cannot be sent to the shader because the shader manager is unavailable." << std::endl;
        return;
    }

    m_pShaderManager->setMat4Value(g_ViewName, view);
    m_pShaderManager->setMat4Value(g_ProjectionName, projection);
    m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
}
