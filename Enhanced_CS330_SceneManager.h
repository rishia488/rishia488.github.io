///////////////////////////////////////////////////////////////////////////////
// SceneManager.h
// ============
// Manages textures, materials, lighting, and rendering for the 3D scene.
//
// AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
// Enhanced by Arishia Jackson for CS 499 Milestone Two
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"
#include "GLFW/glfw3.h"

#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>

class SceneManager
{
public:
    SceneManager(ShaderManager* pShaderManager);
    ~SceneManager();

    struct TEXTURE_INFO
    {
        std::string tag;
        uint32_t ID;
    };

    struct OBJECT_MATERIAL
    {
        float ambientStrength;
        glm::vec3 ambientColor;
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        float shininess;
        std::string tag;
    };

    void PrepareScene();
    void RenderScene();
    void ProcessLightingControls(GLFWwindow* window);

private:
    ShaderManager* m_pShaderManager;
    ShapeMeshes* m_basicMeshes;
    int m_loadedTextures;
    TEXTURE_INFO m_textureIDs[16];
    std::vector<OBJECT_MATERIAL> m_objectMaterials;

    bool m_primaryLightEnabled;
    float m_lightIntensity;
    bool m_lightOnKeyWasPressed;
    bool m_lightOffKeyWasPressed;
    bool m_increaseKeyWasPressed;
    bool m_decreaseKeyWasPressed;

    bool CreateGLTexture(const char* filename, const std::string& tag);
    void BindGLTextures();
    void DestroyGLTextures();
    int FindTextureID(const std::string& tag) const;
    int FindTextureSlot(const std::string& tag) const;
    bool FindMaterial(const std::string& tag, OBJECT_MATERIAL& material) const;

    void SetTransformations(
        const glm::vec3& scaleXYZ,
        float XrotationDegrees,
        float YrotationDegrees,
        float ZrotationDegrees,
        const glm::vec3& positionXYZ);

    void SetShaderColor(float redColorValue, float greenColorValue, float blueColorValue, float alphaValue);
    void SetShaderTexture(const std::string& textureTag);
    void SetTextureUVScale(float u, float v);
    void SetShaderMaterial(const std::string& materialTag);

    void DefineObjectMaterials();
    void SetupSceneLights();
    void UpdateLighting();

    void RenderGroundPlane();
    void RenderAnimatedCubeArray();
    void RenderBottle();
};
