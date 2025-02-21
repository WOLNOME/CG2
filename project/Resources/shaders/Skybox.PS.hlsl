#include "Skybox.hlsli"

struct CameraWorldPosition
{
    float3 worldPosition;
};
ConstantBuffer<CameraWorldPosition> gCameraWorldPosition : register(b1);

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
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    //RGBの計算
    output.color.rgb = textureColor.rgb;
    
    //α値の計算
    output.color.a = textureColor.a;

    return output;
}