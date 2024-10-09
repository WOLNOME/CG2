#include "Particle.hlsli"

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
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    output.color = gMaterial.color * textureColor;
    //テクスチャのα値が一定値以下のディスカード処理(パーティクルとかに使えそう)
    if (output.color.a == 0.0)
    {
        discard; //←ピクセルの棄却(存在抹消)
    }
    return output;
}