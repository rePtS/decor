#pragma once

#include <vector>
#include <DirectXMath.h>
#include "ConstantBuffer.h"

#include <DeusEx.h>

////Operator new/delete for SSE aligned data
//class XMMAligned
//{
//public:
//    void* operator new(const size_t s)
//    {
//        return _aligned_malloc(s, std::alignment_of<__m128>::value);
//    }
//
//    void operator delete(void* const p)
//    {
//        _aligned_free(p);
//    }
//};

#define SLICE_NUMBER 10
#define MAX_SLICE_DATA_SIZE 16
#define SLICE_MAX_INDEX SLICE_NUMBER - 1
#define MAX_LIGHTS_DATA_SIZE 1024

class GlobalShaderConstants //: public XMMAligned
{
    const float HALF_DEGREE_TO_RADIANS = static_cast<float>(PI) / 360.0f;

public:
    explicit GlobalShaderConstants(ID3D11Device& Device, ID3D11DeviceContext& m_DeviceContext);
    GlobalShaderConstants(const GlobalShaderConstants&) = delete;
    GlobalShaderConstants& operator=(const GlobalShaderConstants&) = delete;

    //Operator new/delete for SSE aligned data
    void* operator new(const size_t s) { return _aligned_malloc(s, std::alignment_of<__m128>::value); }
    void operator delete(void* const p) { _aligned_free(p); }

    void Init();
    void Bind();

    void CheckProjectionChange(const FSceneNode& SceneNode);
    void CheckViewChange(const FSceneNode& SceneNode);
    void CheckLevelChange(const FSceneNode& SceneNode);

protected:
    struct PerFrame
    {
        float fRes[2];
        float padding[2];
        DirectX::XMMATRIX ProjectionMatrix;
        DirectX::XMMATRIX ViewMatrix;
        DirectX::XMVECTOR LightDir;
        
        uint32_t IndexesOfFirstLightsInSlices[MAX_SLICE_DATA_SIZE];
        uint32_t LightIndexesFromAllSlices[MAX_LIGHTS_DATA_SIZE];
        DirectX::XMVECTOR Lights[MAX_LIGHTS_DATA_SIZE];
    };
    ConstantBuffer<PerFrame> m_CBufPerFrame;

    // Vars for projection change check
    float m_fFov = 0.0f;
    int m_iViewPortX = 0;
    int m_iViewPortY = 0;
    
    // Cosine value (powered by 2) of the view cone's angle
    float m_SquaredViewConeCos = 0.0f;

    // Actual view matrix
    FCoords m_Coords;

    // Index of the current level (used to determine if a level is loaded/unloaded)
    int m_CurrentLevelIndex;

    // Light sources on the current level
    AAugmentation* m_AugLight;
    std::vector<AActor*> m_Lamps;
    std::vector<AActor*> m_TriggerLights;
    std::vector<AActor*> m_PointLights;
    std::vector<AActor*> m_SpotLights;

    struct LightData
    {
        enum class LightType : uint32_t
        {
            DIRECT = 1,
            POINT = 2,
            SPOT = 3
        };

        DirectX::XMVECTOR Color;
        DirectX::XMVECTOR Location;
        DirectX::XMVECTOR Direction;
        size_t RealIndex;
    };

    std::vector<LightData> m_LightsData;
    std::vector<size_t> m_LightSlices[SLICE_NUMBER];
};

