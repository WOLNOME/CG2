#include "CopyImage.hlsli"

struct Data
{
    float3 hsvColor;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<Data> gData : register(b0);

float3 HSVToRGB(float3 hsv){
    float H = hsv.x;
    float S = hsv.y;
    float V = hsv.z;

    float C = V * S;
    float X = C * (1 - abs(fmod(H * 6, 2) - 1));
    float m = V - C;

    float3 rgb;

    if (0.0 <= H && H < 1.0 / 6.0)
        rgb = float3(C, X, 0);
    else if (1.0 / 6.0 <= H && H < 2.0 / 6.0)
        rgb = float3(X, C, 0);
    else if (2.0 / 6.0 <= H && H < 3.0 / 6.0)
        rgb = float3(0, C, X);
    else if (3.0 / 6.0 <= H && H < 4.0 / 6.0)
        rgb = float3(0, X, C);
    else if (4.0 / 6.0 <= H && H < 5.0 / 6.0)
        rgb = float3(X, 0, C);
    else
        rgb = float3(C, 0, X);

    return rgb + m;
}

float3 RGBToHSV(float3 rgb)
{
    float R = rgb.r;
    float G = rgb.g;
    float B = rgb.b;

    float maxVal = max(R, max(G, B));
    float minVal = min(R, min(G, B));
    float delta = maxVal - minVal;

    float H = 0;
    if (delta > 0)
    {
        if (maxVal == R)
            H = fmod((G - B) / delta, 6.0);
        else if (maxVal == G)
            H = ((B - R) / delta) + 2.0;
        else
            H = ((R - G) / delta) + 4.0;

        H /= 6.0;
        if (H < 0)
            H += 1.0;
    }

    float S = (maxVal == 0) ? 0 : delta / maxVal;
    float V = maxVal;

    return float3(H, S, V);
}

float WrapValue(float value, float minRange, float maxRange)
{
    float range = maxRange - minRange;
    float modValue = fmod(value - minRange, range);
    if (modValue < 0)
    {
        modValue += range;
    }
    return minRange + modValue;
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    float3 hsv = RGBToHSV(textureColor.rgb);
    
    //色パラメータを増減させる
    hsv.x += gData.hsvColor.x;
    hsv.y += gData.hsvColor.y;
    hsv.z += gData.hsvColor.z;
    //0~1の範囲で収まるようにする
    hsv.x = WrapValue(hsv.x, 0.0f, 1.0f);
    hsv.y = saturate(hsv.y);
    hsv.z = saturate(hsv.z);
    
    float3 rgb = HSVToRGB(hsv);
    
    output.color.rgb = rgb;
    //アルファ値を1(不透明)にすることで、スワップチェーンのクリアカラーと競合しないようにする
    output.color.a = 1.0f;
    return output;
}