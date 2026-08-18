///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp
// ============
// Manages textures, materials, lighting, and rendering for the 3D scene.
//
// AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
// Enhanced by Arishia Jackson for CS 499 Milestone Two
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <algorithm>
#include <iostream>
#include <glm/gtx/transform.hpp>

namespace
{
    const char* g_ModelName = "model";
    const char* g_ColorValueName = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName = "bUseTexture";
    const char* g_UseLightingName = "bUseLighting";

    const float MIN_LIGHT_INTENSITY = 0.0f;
    const float MAX_LIGHT_INTENSITY = 2.0f;
    const float LIGHT_INTENSITY_STEP = 0.1f;
}

SceneManager::SceneManager(ShaderManager* pShaderManager)
    : m_pShaderManager(pShaderManager),
      m_basicMeshes(new ShapeMeshes()),
      m_loadedTextures(0),
      m_primaryLightEnabled(true),
      m_lightIntensity(1.0f),
      m_lightOnKeyWasPressed(false),
      m_lightOffKeyWasPressed(false),
      m_increaseKeyWasPressed(false),
      m_decreaseKeyWasPressed(false)
{
}

SceneManager::~SceneManager()
{
    DestroyGLTextures();
    delete m_basicMeshes;
    m_basicMeshes = nullptr;
    m_pShaderManager = nullptr;
}

bool SceneManager::CreateGLTexture(const char* filename, const std::string& tag)
{
    if (filename == nullptr || tag.empty())
    {
        std::cerr << "ERROR: A texture filename and tag are required." << std::endl;
        return false;
    }

    if (m_loadedTextures >= 16)
    {
        std::cerr << "ERROR: Texture capacity reached. Could not load: " << filename << std::endl;
        return false;
    }

    int width = 0;
    int height = 0;
    int colorChannels = 0;
    GLuint textureID = 0;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(filename, &width, &height, &colorChannels, 0);

    if (image == nullptr)
    {
        std::cerr << "ERROR: Texture could not be loaded: " << filename << std::endl;
        return false;
    }

    glGenTextures(1, &textureID);
    if (textureID == 0)
    {
        std::cerr << "ERROR: OpenGL could not create a texture for: " << filename << std::endl;
        stbi_image_free(image);
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (colorChannels == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    }
    else if (colorChannels == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }
    else
    {
        std::cerr << "ERROR: Unsupported channel count (" << colorChannels << ") for: " << filename << std::endl;
        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &textureID);
        return false;
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_textureIDs[m_loadedTextures].ID = textureID;
    m_textureIDs[m_loadedTextures].tag = tag;
    ++m_loadedTextures;

    std::cout << "INFO: Loaded texture " << filename
              << " (" << width << "x" << height << ", " << colorChannels << " channels)." << std::endl;
    return true;
}

void SceneManager::BindGLTextures()
{
    for (int i = 0; i < m_loadedTextures; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
    }
}

void SceneManager::DestroyGLTextures()
{
    for (int i = 0; i < m_loadedTextures; ++i)
    {
        if (m_textureIDs[i].ID != 0)
        {
            glDeleteTextures(1, &m_textureIDs[i].ID);
            m_textureIDs[i].ID = 0;
        }
    }
    m_loadedTextures = 0;
}

int SceneManager::FindTextureID(const std::string& tag) const
{
    for (int i = 0; i < m_loadedTextures; ++i)
    {
        if (m_textureIDs[i].tag == tag)
        {
            return static_cast<int>(m_textureIDs[i].ID);
        }
    }
    return -1;
}

int SceneManager::FindTextureSlot(const std::string& tag) const
{
    for (int i = 0; i < m_loadedTextures; ++i)
    {
        if (m_textureIDs[i].tag == tag)
        {
            return i;
        }
    }
    return -1;
}

bool SceneManager::FindMaterial(const std::string& tag, OBJECT_MATERIAL& material) const
{
    for (const auto& candidate : m_objectMaterials)
    {
        if (candidate.tag == tag)
        {
            material = candidate;
            return true;
        }
    }
    return false;
}

void SceneManager::SetTransformations(
    const glm::vec3& scaleXYZ,
    float XrotationDegrees,
    float YrotationDegrees,
    float ZrotationDegrees,
    const glm::vec3& positionXYZ)
{
    if (m_pShaderManager == nullptr)
    {
        return;
    }

    const glm::mat4 scale = glm::scale(scaleXYZ);
    const glm::mat4 rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
    const glm::mat4 translation = glm::translate(positionXYZ);
    const glm::mat4 modelView = translation * rotationX * rotationY * rotationZ * scale;

    m_pShaderManager->setMat4Value(g_ModelName, modelView);
}

void SceneManager::SetShaderColor(float redColorValue, float greenColorValue, float blueColorValue, float alphaValue)
{
    if (m_pShaderManager == nullptr)
    {
        return;
    }

    m_pShaderManager->setIntValue(g_UseLightingName, true);
    m_pShaderManager->setIntValue(g_UseTextureName, false);
    m_pShaderManager->setVec4Value(g_ColorValueName, glm::vec4(redColorValue, greenColorValue, blueColorValue, alphaValue));
}

void SceneManager::SetShaderTexture(const std::string& textureTag)
{
    if (m_pShaderManager == nullptr)
    {
        return;
    }

    const int textureSlot = FindTextureSlot(textureTag);
    if (textureSlot < 0)
    {
        std::cerr << "ERROR: Texture tag was not found: " << textureTag << std::endl;
        m_pShaderManager->setIntValue(g_UseTextureName, false);
        return;
    }

    m_pShaderManager->setIntValue(g_UseLightingName, true);
    m_pShaderManager->setIntValue(g_UseTextureName, true);
    m_pShaderManager->setSampler2DValue(g_TextureValueName, textureSlot);
}

void SceneManager::SetTextureUVScale(float u, float v)
{
    if (m_pShaderManager != nullptr)
    {
        m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
    }
}

void SceneManager::SetShaderMaterial(const std::string& materialTag)
{
    if (m_pShaderManager == nullptr)
    {
        return;
    }

    OBJECT_MATERIAL material;
    if (!FindMaterial(materialTag, material))
    {
        std::cerr << "ERROR: Material tag was not found: " << materialTag << std::endl;
        return;
    }

    m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
    m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
    m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
    m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
    m_pShaderManager->setFloatValue("material.shininess", material.shininess);
}

void SceneManager::DefineObjectMaterials()
{
    m_objectMaterials.clear();

    OBJECT_MATERIAL plastic;
    plastic.tag = "plastic";
    plastic.ambientColor = glm::vec3(0.3f, 0.0f, 0.0f);
    plastic.ambientStrength = 0.5f;
    plastic.diffuseColor = glm::vec3(0.8f, 0.1f, 0.1f);
    plastic.specularColor = glm::vec3(1.0f);
    plastic.shininess = 64.0f;
    m_objectMaterials.push_back(plastic);

    OBJECT_MATERIAL labelMaterial;
    labelMaterial.tag = "labelmat";
    labelMaterial.ambientColor = glm::vec3(1.0f);
    labelMaterial.ambientStrength = 0.6f;
    labelMaterial.diffuseColor = glm::vec3(1.0f);
    labelMaterial.specularColor = glm::vec3(1.0f);
    labelMaterial.shininess = 64.0f;
    m_objectMaterials.push_back(labelMaterial);
}

void SceneManager::SetupSceneLights()
{
    if (m_pShaderManager == nullptr)
    {
        std::cerr << "ERROR: Scene lighting could not be configured because the shader manager is unavailable." << std::endl;
        return;
    }

    m_pShaderManager->setVec3Value("viewPos", glm::vec3(0.0f, 2.0f, 6.0f));
    m_pShaderManager->setVec3Value("spotlight.position", glm::vec3(2.0f, 1.5f, 1.5f));
    m_pShaderManager->setVec3Value("spotlight.direction", glm::vec3(0.0f, -1.0f, -1.0f));
    m_pShaderManager->setFloatValue("spotlight.cutOff", glm::cos(glm::radians(12.5f)));
    m_pShaderManager->setFloatValue("spotlight.outerCutOff", glm::cos(glm::radians(17.5f)));
    UpdateLighting();
}

void SceneManager::UpdateLighting()
{
    if (m_pShaderManager == nullptr)
    {
        return;
    }

    const float activeIntensity = m_primaryLightEnabled ? m_lightIntensity : 0.0f;
    m_pShaderManager->setVec3Value("spotlight.ambient", glm::vec3(0.1f * activeIntensity));
    m_pShaderManager->setVec3Value("spotlight.diffuse", glm::vec3(activeIntensity));
    m_pShaderManager->setVec3Value("spotlight.specular", glm::vec3(activeIntensity));
}

void SceneManager::ProcessLightingControls(GLFWwindow* window)
{
    if (window == nullptr)
    {
        return;
    }

    const bool lightOnPressed = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
    if (lightOnPressed && !m_lightOnKeyWasPressed)
    {
        m_primaryLightEnabled = true;
        std::cout << "INFO: Primary light enabled." << std::endl;
    }
    m_lightOnKeyWasPressed = lightOnPressed;

    const bool lightOffPressed = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
    if (lightOffPressed && !m_lightOffKeyWasPressed)
    {
        m_primaryLightEnabled = false;
        std::cout << "INFO: Primary light disabled." << std::endl;
    }
    m_lightOffKeyWasPressed = lightOffPressed;

    const bool increasePressed = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
    if (increasePressed && !m_increaseKeyWasPressed)
    {
        m_lightIntensity = std::clamp(m_lightIntensity + LIGHT_INTENSITY_STEP, MIN_LIGHT_INTENSITY, MAX_LIGHT_INTENSITY);
        std::cout << "INFO: Light intensity set to " << m_lightIntensity << std::endl;
    }
    m_increaseKeyWasPressed = increasePressed;

    const bool decreasePressed = glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS;
    if (decreasePressed && !m_decreaseKeyWasPressed)
    {
        m_lightIntensity = std::clamp(m_lightIntensity - LIGHT_INTENSITY_STEP, MIN_LIGHT_INTENSITY, MAX_LIGHT_INTENSITY);
        std::cout << "INFO: Light intensity set to " << m_lightIntensity << std::endl;
    }
    m_decreaseKeyWasPressed = decreasePressed;

    UpdateLighting();
}

void SceneManager::PrepareScene()
{
    if (m_basicMeshes == nullptr || m_pShaderManager == nullptr)
    {
        std::cerr << "ERROR: The scene cannot be prepared because a required manager is unavailable." << std::endl;
        return;
    }

    m_basicMeshes->LoadPlaneMesh();
    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadCylinderMesh();

    const bool capLoaded = CreateGLTexture("Textures/cap_texture.png", "cap");
    const bool labelLoaded = CreateGLTexture("Textures/label_texture.png", "label");
    if (!capLoaded || !labelLoaded)
    {
        std::cerr << "WARNING: One or more scene textures failed to load." << std::endl;
    }

    BindGLTextures();
    DefineObjectMaterials();
    SetupSceneLights();
}

void SceneManager::RenderGroundPlane()
{
    SetTransformations(glm::vec3(20.0f, 1.0f, 10.0f), 0.0f, 0.0f, 0.0f, glm::vec3(0.0f, -2.0f, 0.0f));
    SetShaderColor(0.5f, 0.5f, 0.5f, 1.0f);
    SetShaderMaterial("plastic");
    m_basicMeshes->DrawPlaneMesh();
}

void SceneManager::RenderAnimatedCubeArray()
{
    const float rotationAngle = static_cast<float>(glfwGetTime()) * 30.0f;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            for (int z = -1; z <= 1; ++z)
            {
                const glm::vec3 position(x * 0.6f - 2.0f, y * 0.6f + 2.0f, z * 0.6f);
                SetTransformations(glm::vec3(0.3f), rotationAngle, rotationAngle, 0.0f, position);

                const float red = (x + 1) / 2.0f;
                const float green = (y + 1) / 2.0f;
                const float blue = (z + 1) / 2.0f;
                SetShaderColor(red, green, blue, 1.0f);
                SetShaderMaterial("plastic");
                m_basicMeshes->DrawBoxMesh();
            }
        }
    }
}

void SceneManager::RenderBottle()
{
    SetTransformations(glm::vec3(0.5f, 1.0f, 0.5f), 0.0f, 45.0f, 0.0f, glm::vec3(2.0f, 0.5f, 0.0f));
    SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
    SetShaderMaterial("plastic");
    m_basicMeshes->DrawCylinderMesh();

    SetTransformations(glm::vec3(0.55f, 0.2f, 0.55f), 0.0f, 0.0f, 0.0f, glm::vec3(2.0f, 1.25f, 0.0f));
    SetShaderMaterial("plastic");
    SetShaderTexture("cap");
    SetTextureUVScale(2.0f, 2.0f);
    m_basicMeshes->DrawCylinderMesh();
}

void SceneManager::RenderScene()
{
    if (m_basicMeshes == nullptr || m_pShaderManager == nullptr)
    {
        std::cerr << "ERROR: The scene cannot be rendered because a required manager is unavailable." << std::endl;
        return;
    }

    RenderGroundPlane();
    RenderAnimatedCubeArray();
    RenderBottle();
}
