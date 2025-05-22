#include "CopyImage.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};


static const float pi = 3.14159265f;

static const float2 kIndex3x3[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
};

float gauss(float x, float y, float sigma)
{
    float exponent = -(x * x + y * y) * rcp(2.0f * sigma * sigma);
    float denominator = 2.0f * pi * sigma * sigma;
    return exp(exponent) * rcp(denominator);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color.rgb = float3(0.0f, 0.0f, 0.0f);
    output.color.a = 1.0f;
    
    //kernelを求める
    float weight = 0.0f;
    float kernel[3][3];
    for (int x0 = 0; x0 < 3; ++x0)
    {
        for (int y0 = 0; y0 < 3; ++y0)
        {
            kernel[x0][y0] = gauss((float) x0, (float) y0, 2.0f);
            weight += kernel[x0][y0];
        }
    }
    //求めたkernelを使い、BoxFilterと同じく畳み込みを行う。
    uint width, height; //uvStepSizeの算出
    gTexture.GetDimensions(width, height);
    float2 uvStepSize = float2(rcp((float) width), rcp((float) height));
    for (int x1 = 0; x1 < 3; ++x1)
    {
        for (int y1 = 0; y1 < 3; ++y1)
        {
            //現在のtexcoordを算出
            float2 texcoord = input.texcoord + kIndex3x3[x1][y1] * uvStepSize;
            //色に1/9をかけて足す
            float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
            output.color.rgb += fetchColor * kernel[x1][y1];
        }
    }
    //畳み込み後の値を正規化する。
    output.color.rgb *= rcp(weight);
   
    //アルファ値を1(不透明)にすることで、スワップチェーンのクリアカラーと競合しないようにする
    output.color.a = 1.0f;
    return output;
}