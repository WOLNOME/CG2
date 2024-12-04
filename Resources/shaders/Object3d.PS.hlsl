#include "object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t lightingKind;
    float32_t4x4 uvTransform;
    int32_t isTexture;
};
struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};
struct PixelShaderOutput
{
    float32_t4 color : SV_Target0;
};


ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor;
    if (gMaterial.isTexture != 0)
    {
        //テクスチャあり
        textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    }
    else
    {
        //テクスチャなし
        textureColor.x = 1.0f;
        textureColor.y = 1.0f;
        textureColor.z = 1.0f;
        textureColor.w = 1.0f;
        
    }
   
    
    if (gMaterial.lightingKind == 0)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    }
    else if (gMaterial.lightingKind == 1)
    {
        float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    }
    else if (gMaterial.lightingKind == 2)
    {
        output.color = gMaterial.color * textureColor;
    }
    
    return output;
}