#include "TextTexture.hlsli"

//テキストの構造体
struct TextParam
{
    float4 color; //テキストの色
};
//アウトラインの構造体
struct EdgeParam
{
    uint isEdgeDisplay; //アウトライン表示フラグ
    float width; //アウトラインの幅
    float2 slideRate; //アウトラインのスライド量
    float4 color; //アウトラインの色
};

ConstantBuffer<TextParam> gTextParam : register(b0);
ConstantBuffer<EdgeParam> gEdgeParam : register(b1);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float4 centerColor = gTexture.Sample(gSampler, input.texcoord);
    
    //すでにテキスト本体が書かれていたら返す
    if (centerColor.a > 0.0f)
    {
        output.color = gTextParam.color;
        return output;
    }
    //アウトラインの表示設定がオフだったらcenterColorをそのまま返す
    if (gEdgeParam.isEdgeDisplay == 0)
    {
        output.color = centerColor;
        return output;
    }
    
    //アウトラインの処理
    {
        //対象のピクセルから周りのピクセルのインデックスを手に入れる
        int range = (int) gEdgeParam.width * 2 + 1;
        //テクスチャのサイズを手に入れる
        uint textureWidth, textureHeight;
        gTexture.GetDimensions(textureWidth, textureHeight);
        //値を代入
        for (int y0 = 0; y0 < range; ++y0)
        {
            for (int x0 = 0; x0 < range; ++x0)
            {
                // サンプリングするピクセルの位置を割り出す
                float2 offset;
                offset.x = float((float) x0 - gEdgeParam.width * rcp(2.0f) - gEdgeParam.slideRate.x);
                offset.y = float((float) y0 - gEdgeParam.width * rcp(2.0f) - gEdgeParam.slideRate.y);

                // 中心からの距離を計算
                float dist = length(offset + gEdgeParam.slideRate);
                // 円から外れたピクセルは無視
                if (dist > gEdgeParam.width)
                    continue;

                // サンプリング位置をUVで算出
                float2 uvStepSize = float2(rcp((float) textureWidth), rcp((float) textureHeight));
                float2 sampleUV = input.texcoord + offset * uvStepSize;

                // アルファ値でアウトライン候補判断
                float alpha = gTexture.Sample(gSampler, sampleUV).a;
                //アルファ値が0より大きい→文字に当たってる
                if (alpha > 0.0f)
                {
                    //色をアウトライン用の色に変えてreturn (もし別の処理を加えたい場合は外でreturn)
                    output.color = gEdgeParam.color;
                    return output;
                }
            }
        }
    }
    
    //色は特に変えずにreturn 
    output.color = centerColor;
    return output;
}