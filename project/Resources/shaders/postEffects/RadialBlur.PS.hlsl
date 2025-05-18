#include "CopyImage.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    const float2 kCenter = float2(0.5f, 0.5f);  //中心点
    const int kNumSamples = 10; //サンプリング数
    const float kBlurWidth = 0.01f; //ぼかしの幅
    //中心から現在のUVに対しての方向を計算
    float2 direction = input.texcoord - kCenter;
    float3 outputColor = float3(0.0f, 0.0f, 0.0f);
    for (int sampleIndex = 0; sampleIndex < kNumSamples; ++sampleIndex)
    {
        //現在のUVから先ほど計算下方向にサンプリング点を進めながらサンプリングしていく
        float2 texcoord = input.texcoord + direction * kBlurWidth * float(sampleIndex);
        outputColor.rgb += gTexture.Sample(gSampler, texcoord).rgb;
    }
    //平均化する
    outputColor.rgb *= rcp(float(kNumSamples));
    
    output.color.rgb = outputColor;
    
    //アルファ値を1(不透明)にすることで、スワップチェーンのクリアカラーと競合しないようにする
    output.color.a = 1.0f;
    return output;
}