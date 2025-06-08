#include "CopyImage.hlsli"

struct Threshold
{
    float value;
};

Texture2D<float4> gTexture : register(t0);
Texture2D<float> gMaskTexture : register(t1);
SamplerState gSampler : register(s0);
ConstantBuffer<Threshold> gThreshold : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float mask = gMaskTexture.Sample(gSampler, input.texcoord);
    //maskの値が閾値以下の場合はdiscardして抜く
    if (mask <= gThreshold.value)
    {
        discard;
    }
    //Edgeっぽさを算出
    float edge = 1.0f - smoothstep(gThreshold.value, gThreshold.value + 0.03f, mask);
    output.color = gTexture.Sample(gSampler, input.texcoord);
    //Edgeっぽいほど指定した色を加算
    output.color.rgb += edge * float3(1.0f, 0.4f, 0.3f);
    //アルファ値を1(不透明)にすることで、スワップチェーンのクリアカラーと競合しないようにする
    output.color.a = 1.0f;
    return output;
}