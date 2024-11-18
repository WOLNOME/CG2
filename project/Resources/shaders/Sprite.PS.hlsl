#include "object3d.hlsli"

struct Material
{
    float32_t4 color;
    float32_t4x4 uvTransform;
    int32_t isTexture;
};
struct PixelShaderOutput
{
    float32_t4 color : SV_Target0;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    
    // テクスチャカラーの設定
    float32_t4 textureColor = (gMaterial.isTexture != 0) ? gTexture.Sample(gSampler, transformedUV.xy) : float4(1.0f, 1.0f, 1.0f, 1.0f);

   
    output.color = gMaterial.color * textureColor;
   
    return output;
}