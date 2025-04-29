#include "Skybox.hlsli"
#define MAX_DL_NUM 1
#define MAX_PL_NUM 16
#define MAX_SL_NUM 16

struct Material
{
    float4 color;
    float4x4 uvTransform;
    float isTexture;
    float shininess;
};
struct CameraWorldPosition
{
    float3 worldPosition;
};
struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
    uint isActive;
};
struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    uint isActive;
};
struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
    uint isActive;
};
struct SceneLight
{
    DirectionalLight directionalLights[MAX_DL_NUM];
    PointLight pointLights[MAX_PL_NUM];
    SpotLight spotLights[MAX_SL_NUM];
    uint numDirectionalLights;
    uint numPointLights;
    uint numSpotLights;
};
struct LightFlag
{
    uint isActiveLights;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<CameraWorldPosition> gCameraWorldPosition : register(b1);
ConstantBuffer<SceneLight> gSceneLight : register(b2);
ConstantBuffer<LightFlag> gLightFlag : register(b3);

struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

//通常テクスチャ
TextureCube<float4> gTexture : register(t0);

//通常サンプラー
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    //テクスチャカラーの設定
    float4 textureColor = gTexture.Sample(gSampler, normalize(input.texcoord));
    
    //RGBの計算
    output.color.rgb = textureColor.rgb * gMaterial.color.rgb;
    
    //α値の計算
    output.color.a = textureColor.a * gMaterial.color.a;

    return output;
}