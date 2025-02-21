#include "Particle.hlsli"

struct Material
{
    float4 color;
    float4x4 uvTransform;
    int isTexture;
    int lightingKind;
};
struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor;
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
        textureColor.a = 1.0f;
        
    }
   
    output.color = gMaterial.color * textureColor * input.color;
    
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}