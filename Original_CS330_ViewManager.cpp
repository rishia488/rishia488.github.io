///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    
#include <GLFW/glfw3.h>
#include <cmath>

// declaration of the global variables and defines
namespace
{
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	Camera* g_pCamera = nullptr;

	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	float gDeltaTime = 0.0f;
	float gLastFrame = 0.0f;

	bool bOrthographicProjection = false;
	float gYaw = -90.0f;
	float gPitch = 0.0f;
	float gCameraSpeed = 2.5f;
}

ViewManager::ViewManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
}

ViewManager::~ViewManager()
{
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle, NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_pWindow = window;
	return(window);
}

void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	float xPos = static_cast<float>(xMousePos);
	float yPos = static_cast<float>(yMousePos);
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
	float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;
	gYaw += xOffset;
	gPitch += yOffset;
	if (gPitch > 89.0f) gPitch = 89.0f;
	if (gPitch < -89.0f) gPitch = -89.0f;
	glm::vec3 direction;
	direction.x = cos(glm::radians(gYaw)) * cos(glm::radians(gPitch));
	direction.y = sin(glm::radians(gPitch));
	direction.z = sin(glm::radians(gYaw)) * cos(glm::radians(gPitch));
	g_pCamera->Front = glm::normalize(direction);
}

void ViewManager::ProcessKeyboardEvents()
{
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_pWindow, true);

	float speed = gCameraSpeed * gDeltaTime;
	glm::vec3 right = glm::normalize(glm::cross(g_pCamera->Front, g_pCamera->Up));
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
		bOrthographicProjection = false;
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
		bOrthographicProjection = true;
}

void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;
	ProcessKeyboardEvents();
	view = g_pCamera->GetViewMatrix();
	if (bOrthographicProjection)
	{
		float scale = 8.0f;
		projection = glm::ortho(-scale, scale, -scale * ((float)WINDOW_HEIGHT / (float)WINDOW_WIDTH), scale * ((float)WINDOW_HEIGHT / (float)WINDOW_WIDTH), 0.1f, 100.0f);
	}
	else
	{
		projection = glm::perspective(glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ViewName, view);
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}
