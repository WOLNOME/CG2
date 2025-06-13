#include "CopyImage.hlsli"
#include "../util/RandomUtility.hlsli"

struct Data
{
    float seed;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<Data> gData : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    //乱数生成
    float random = rand2dTo1d(mul(gData.seed, input.texcoord));
    //色にする
    output.color = float4(random, random, random, 1.0f);
    //アルファ値を1(不透明)にすることで、スワップチェーンのクリアカラーと競合しないようにする
    output.color.a = 1.0f;
    return output;
}