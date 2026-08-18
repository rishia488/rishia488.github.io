///////////////////////////////////////////////////////////////////////////////
// ViewManager.h
// ============
// Manages the camera, projection, and user navigation for the 3D scene.
//
// AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
// Enhanced by Arishia Jackson for CS 499 Milestone Two
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "camera.h"
#include "GLFW/glfw3.h"

class ViewManager
{
public:
    ViewManager(ShaderManager* pShaderManager);
    ~ViewManager();

    static void Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos);
    static void Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset);

    GLFWwindow* CreateDisplayWindow(const char* windowTitle);
    void PrepareSceneView();
    void ResetCamera();

private:
    ShaderManager* m_pShaderManager;
    GLFWwindow* m_pWindow;

    void ProcessKeyboardEvents();
};
