#include "Object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t lightingKind;
    float32_t4x4 uvTransform;
    int32_t isTexture;
    float32_t shininess;
};
struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};
struct LightFlag
{
    int32_t isDirectionalLight;
};
struct CameraWorldPosition
{
    float32_t3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<LightFlag> gLightFlag : register(b2);
ConstantBuffer<CameraWorldPosition> gCameraWorldPosition : register(b3);


struct PixelShaderOutput
{
    float32_t4 color : SV_Target0;
};

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);

    // テクスチャカラーの設定
    float32_t4 textureColor = (gMaterial.isTexture != 0) ? gTexture.Sample(gSampler, transformedUV.xy) : float4(1.0f, 1.0f, 1.0f, 1.0f);

    // 平行光源の計算
    if (gLightFlag.isDirectionalLight == 1)
    {
        
        
        if (gMaterial.lightingKind == 0)
        {
            //反射の計算
            float32_t3 toEye = normalize(gCameraWorldPosition.worldPosition - input.worldPosition);
            float32_t3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
            float32_t3 halfVector = normalize(-gDirectionalLight.direction + toEye);
            float NdotH = dot(normalize(input.normal), halfVector);
            float specularPow = pow(saturate(NdotH), gMaterial.shininess);
            
            float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            //拡散反射
            float32_t3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            //鏡面反射
            float32_t3 specularColor = { 1.0f, 1.0f, 1.0f }; //この値はMaterialで変えられるようになるとよい。
            float32_t3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * specularColor;
            //拡散反射+鏡面反射
            output.color.rgb = diffuse + specular;
            //α値
            output.color.a = gMaterial.color.a * textureColor.a;
            
        }
        else if (gMaterial.lightingKind == 1)
        {
            float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
            output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
        }
        else if (gMaterial.lightingKind == 2)
        {
            output.color = gMaterial.color * textureColor;
        }
    }
    else
    {
        // 光源がない場合
        output.color = gMaterial.color * textureColor;
    }

    return output;
}
