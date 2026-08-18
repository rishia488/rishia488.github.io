///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp
// ============
// Manages the loading and rendering of 3D scenes with lighting and spotlight
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Modified for Milestone 5 by Arishia Jackson
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>

namespace
{
    const char* g_ModelName = "model";
    const char* g_ColorValueName = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName = "bUseTexture";
    const char* g_UseLightingName = "bUseLighting";
}

SceneManager::SceneManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_basicMeshes = new ShapeMeshes();
}

SceneManager::~SceneManager()
{
    m_pShaderManager = NULL;
    delete m_basicMeshes;
    m_basicMeshes = NULL;
}

bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
    int width = 0, height = 0, colorChannels = 0;
    GLuint textureID = 0;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(filename, &width, &height, &colorChannels, 0);

    if (image)
    {
        std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (colorChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (colorChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            std::cout << "Unsupported number of channels: " << colorChannels << std::endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_textureIDs[m_loadedTextures].ID = textureID;
        m_textureIDs[m_loadedTextures].tag = tag;
        m_loadedTextures++;

        return true;
    }

    std::cout << "Could not load image: " << filename << std::endl;
    return false;
}

void SceneManager::BindGLTextures()
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
    }
}

void SceneManager::DestroyGLTextures()
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        glDeleteTextures(1, &m_textureIDs[i].ID);
    }
}

int SceneManager::FindTextureID(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; ++i)
    {
        if (m_textureIDs[i].tag == tag)
            return m_textureIDs[i].ID;
    }
    return -1;
}

int SceneManager::FindTextureSlot(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; ++i)
    {
        if (m_textureIDs[i].tag == tag)
            return i;
    }
    return -1;
}

bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
    for (auto& mat : m_objectMaterials)
    {
        if (mat.tag == tag)
        {
            material = mat;
            return true;
        }
    }
    return false;
}

void SceneManager::SetTransformations(glm::vec3 scaleXYZ, float XrotationDegrees, float YrotationDegrees, float ZrotationDegrees, glm::vec3 positionXYZ)
{
    glm::mat4 scale = glm::scale(scaleXYZ);
    glm::mat4 rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 translation = glm::translate(positionXYZ);
    glm::mat4 modelView = translation * rotationX * rotationY * rotationZ * scale;

    if (m_pShaderManager)
        m_pShaderManager->setMat4Value(g_ModelName, modelView);
}

void SceneManager::SetShaderColor(float redColorValue, float greenColorValue, float blueColorValue, float alphaValue)
{
    glm::vec4 currentColor(redColorValue, greenColorValue, blueColorValue, alphaValue);

    if (m_pShaderManager)
    {
        m_pShaderManager->setIntValue(g_UseLightingName, true);
        m_pShaderManager->setIntValue(g_UseTextureName, false);
        m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
    }
}

void SceneManager::SetShaderTexture(std::string textureTag)
{
    if (m_pShaderManager)
    {
        m_pShaderManager->setIntValue(g_UseLightingName, true);
        m_pShaderManager->setIntValue(g_UseTextureName, true);
        int textureID = FindTextureSlot(textureTag);
        m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
    }
}

void SceneManager::SetTextureUVScale(float u, float v)
{
    if (m_pShaderManager)
        m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
}

void SceneManager::SetShaderMaterial(std::string materialTag)
{
    OBJECT_MATERIAL material;
    if (FindMaterial(materialTag, material))
    {
        m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
        m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
        m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
        m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
        m_pShaderManager->setFloatValue("material.shininess", material.shininess);
    }
}

void SceneManager::PrepareScene()
{
    m_basicMeshes->LoadPlaneMesh();
    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadCylinderMesh();

    CreateGLTexture("Textures/cap_texture.png", "cap");
    CreateGLTexture("Textures/label_texture.png", "label");

    OBJECT_MATERIAL plastic;
    plastic.tag = "plastic";
    plastic.ambientColor = glm::vec3(0.3f, 0.0f, 0.0f);
    plastic.ambientStrength = 0.5f;
    plastic.diffuseColor = glm::vec3(0.8f, 0.1f, 0.1f);
    plastic.specularColor = glm::vec3(1.0f);
    plastic.shininess = 64.0f;
    m_objectMaterials.push_back(plastic);

    OBJECT_MATERIAL labelmat;
    labelmat.tag = "labelmat";
    labelmat.ambientColor = glm::vec3(1.0f);
    labelmat.ambientStrength = 0.6f;
    labelmat.diffuseColor = glm::vec3(1.0f);
    labelmat.specularColor = glm::vec3(1.0f);
    labelmat.shininess = 64.0f;
    m_objectMaterials.push_back(labelmat);

    m_pShaderManager->setVec3Value("viewPos", glm::vec3(0.0f, 2.0f, 6.0f));

    m_pShaderManager->setVec3Value("spotlight.position", glm::vec3(2.0f, 1.5f, 1.5f));
    m_pShaderManager->setVec3Value("spotlight.direction", glm::vec3(0.0f, -1.0f, -1.0f));
    m_pShaderManager->setFloatValue("spotlight.cutOff", glm::cos(glm::radians(12.5f)));
    m_pShaderManager->setFloatValue("spotlight.outerCutOff", glm::cos(glm::radians(17.5f)));
    m_pShaderManager->setVec3Value("spotlight.ambient", glm::vec3(0.1f));
    m_pShaderManager->setVec3Value("spotlight.diffuse", glm::vec3(1.0f));
    m_pShaderManager->setVec3Value("spotlight.specular", glm::vec3(1.0f));
}

void SceneManager::RenderScene()
{
    glm::vec3 scaleXYZ;
    glm::vec3 positionXYZ;

    scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
    positionXYZ = glm::vec3(0.0f, -2.0f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderColor(0.5f, 0.5f, 0.5f, 1.0f);
    SetShaderMaterial("plastic");
    m_basicMeshes->DrawPlaneMesh();

    float time = static_cast<float>(glfwGetTime());
    float rotationAngle = time * 30.0f;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            for (int z = -1; z <= 1; ++z)
            {
                scaleXYZ = glm::vec3(0.3f);
                positionXYZ = glm::vec3(x * 0.6f - 2.0f, y * 0.6f + 2.0f, z * 0.6f);
                SetTransformations(scaleXYZ, rotationAngle, rotationAngle, 0.0f, positionXYZ);
                float red = (x + 1) / 2.0f;
                float green = (y + 1) / 2.0f;
                float blue = (z + 1) / 2.0f;
                SetShaderColor(red, green, blue, 1.0f);
                SetShaderMaterial("plastic");
                m_basicMeshes->DrawBoxMesh();
            }
        }
    }

    // Bottle Body - PINK color only (no texture)
    scaleXYZ = glm::vec3(0.5f, 1.0f, 0.5f);
    positionXYZ = glm::vec3(2.0f, 0.5f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 45.0f, 0.0f, positionXYZ);
    SetShaderColor(1.0f, 0.0f, 1.0f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();

    // Bottle Cap
    scaleXYZ = glm::vec3(0.55f, 0.2f, 0.55f);
    positionXYZ = glm::vec3(2.0f, 1.25f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderMaterial("plastic");
    SetShaderTexture("cap");
    SetTextureUVScale(2.0f, 2.0f);
    m_basicMeshes->DrawCylinderMesh();
}
