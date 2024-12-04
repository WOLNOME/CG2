#include "Object3d.hlsli"
#define MAX_DL_NUM 1
#define MAX_DL_CASCADE_NUM 3
#define MAX_PL_NUM 16
#define MAX_SL_NUM 16

struct Material
{
    float4 color;
    float4x4 uvTransform;
    float isTexture;
    float shininess;
};
struct CameraWorldPosition
{
    float3 worldPosition;
};
struct CascadeData
{
    float4x4 lightVPMatrix;
    float cascadeSplits;
};
struct DirectionalLight
{
    CascadeData cascade[MAX_DL_CASCADE_NUM];
    float4 color;
    float3 direction;
    float intensity;
    uint numCascade;
    uint isActive;
};
struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    uint isActive;
};
struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
    uint isActive;
};
struct SceneLight
{
    DirectionalLight directionalLights[MAX_DL_NUM];
    PointLight pointLights[MAX_PL_NUM];
    SpotLight spotLights[MAX_SL_NUM];
    uint numDirectionalLights;
    uint numPointLights;
    uint numSpotLights;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<CameraWorldPosition> gCameraWorldPosition : register(b1);
ConstantBuffer<SceneLight> gSceneLight : register(b2);

struct PixelShaderOutput
{
    float4 color : SV_Target0;
};

//通常テクスチャ
Texture2D<float4> gTexture : register(t0);
//シャドウマップ(深度情報のみなのでfloat)
Texture2DArray<float> gDirLightShadowTexture[MAX_DL_CASCADE_NUM] : register(t1);

//通常サンプラー
SamplerState gSampler : register(s0);
//シャドウマップ用サンプラー
SamplerComparisonState gShadowSampler : register(s1);


PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);

    // テクスチャカラーの設定
    float4 textureColor = (gMaterial.isTexture != 0) ? gTexture.Sample(gSampler, transformedUV.xy) : float4(1.0f, 1.0f, 1.0f, 1.0f);
   
    //処理するライトの数
    int useLightCount = 0;
    
    // 平行光源の計算
    float3 diffuseDirectionalLight = { 0.0f, 0.0f, 0.0f };
    float3 specularDirectionalLight = { 0.0f, 0.0f, 0.0f };
    
    for (int i = 0; i < gSceneLight.numDirectionalLights; ++i)
    {
        if (gSceneLight.directionalLights[i].isActive == 1)
        {
            ///影の計算
            //ピクセルがどのカスケードに属しているかの計算
            int cascadeIndex = 0;
            float4 cascadeColor = { 1.0f, 1.0f, 1.0f, 1.0f };
            if (length(input.worldPosition - gCameraWorldPosition.worldPosition) <= gSceneLight.directionalLights[i].cascade[0].cascadeSplits)
            {
                cascadeIndex = 0;
                cascadeColor = float4(1.0f, 0.0f, 0.0f, 1.0f);

            }
            else if (length(input.worldPosition - gCameraWorldPosition.worldPosition) <= gSceneLight.directionalLights[i].cascade[1].cascadeSplits)
            {
                cascadeIndex = 1;
                cascadeColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
            }
            else
            {
                cascadeIndex = 2;
                cascadeColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
            }
            //ピクセルのワールド座標を光源のビュープロジェクション空間に変換
            float4 lightSpacePosition = mul(float4(input.worldPosition, 1.0f), gSceneLight.directionalLights[i].cascade[cascadeIndex].lightVPMatrix);
            //射影空間座標を正規化
            lightSpacePosition.xyz /= lightSpacePosition.w;
            //座標系を[0,1]に変換し、深度を抜き出す
            float2 pixelUV = lightSpacePosition.xy * 0.5f + 0.5f;
            float pixelDepth = lightSpacePosition.z;
            //ピクセルの深度と深度テクスチャのサンプリング結果を抜き出す
            float shadowDepth = gDirLightShadowTexture[cascadeIndex].SampleCmpLevelZero(gShadowSampler, float3(pixelUV, i), pixelDepth).r;
            //ピクセルの深度値とテクスチャの深度値を比較してシャドウの有無を判定
           float shadowFactor = shadowDepth < pixelDepth ? 0.0f : 1.0f;
           
            
            //反射の計算
            float3 toEye = normalize(gCameraWorldPosition.worldPosition - input.worldPosition);
            float3 reflectLight = reflect(gSceneLight.directionalLights[i].direction, normalize(input.normal));
            float3 halfVector = normalize(-gSceneLight.directionalLights[i].direction + toEye);
            float NdotH = dot(normalize(input.normal), halfVector);
            float specularPow = pow(saturate(NdotH), gMaterial.shininess);
            
            float NdotL = dot(normalize(input.normal), -gSceneLight.directionalLights[i].direction);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            //拡散反射
            diffuseDirectionalLight += gMaterial.color.rgb * textureColor.rgb * gSceneLight.directionalLights[i].color.rgb * cascadeColor.rgb * cos * gSceneLight.directionalLights[i].intensity * shadowFactor;
            // diffuseDirectionalLight += float3(shadowFactor, shadowFactor, shadowFactor);
            //鏡面反射
            float3 specularColor = { 1.0f, 1.0f, 1.0f }; //この値はMaterialで変えられるようになるとよい。
            specularDirectionalLight += gSceneLight.directionalLights[i].color.rgb * cascadeColor.rgb * gSceneLight.directionalLights[i].intensity * specularPow * specularColor * shadowFactor;
            //specularDirectionalLight += float3(shadowFactor, shadowFactor, shadowFactor);
            useLightCount++;
        }
    }
    
    
    //点光源の計算
    float3 diffusePointLight = { 0.0f, 0.0f, 0.0f };
    float3 specularPointLight = { 0.0f, 0.0f, 0.0f };
    
    for (int j = 0; j < gSceneLight.numPointLights; ++j)
    {
        if (gSceneLight.pointLights[j].isActive == 1)
        {
            useLightCount++;
            //光源から物体への方向
            float3 pointLightDirection = normalize(input.worldPosition - gSceneLight.pointLights[j].position);
            //反射の計算
            float3 toEye = normalize(gCameraWorldPosition.worldPosition - input.worldPosition);
            float3 reflectLight = reflect(pointLightDirection, normalize(input.normal));
            float3 halfVector = normalize(-pointLightDirection + toEye);
            float NdotH = dot(normalize(input.normal), halfVector);
            float specularPow = pow(saturate(NdotH), gMaterial.shininess);
            
            float NdotL = dot(normalize(input.normal), -pointLightDirection);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            float distance = length(gSceneLight.pointLights[j].position - input.worldPosition); //ポイントライトへの距離
            float factor = pow(saturate(-distance / gSceneLight.pointLights[j].radius + 1.0f), gSceneLight.pointLights[j].decay);
            //拡散反射
            diffusePointLight += gMaterial.color.rgb * textureColor.rgb * gSceneLight.pointLights[j].color.rgb * cos * gSceneLight.pointLights[j].intensity * factor;
            //鏡面反射
            float3 specularColor = { 1.0f, 1.0f, 1.0f }; //この値はMaterialで変えられるようになるとよい。
            specularPointLight += gSceneLight.pointLights[j].color.rgb * gSceneLight.pointLights[j].intensity * specularPow * specularColor * factor;
        }
    }
    
    //スポットライトの計算
    float3 diffuseSpotLight = { 0.0f, 0.0f, 0.0f };
    float3 specularSpotLight = { 0.0f, 0.0f, 0.0f };
    
    for (int k = 0; k < gSceneLight.numPointLights; ++k)
    {
        if (gSceneLight.spotLights[k].isActive == 1)
        {
            useLightCount++;
            //光源から物体表面への方向
            float3 spotLightDirectionOnSurface = normalize(input.worldPosition - gSceneLight.spotLights[k].position);
             //反射の計算
            float3 toEye = normalize(gCameraWorldPosition.worldPosition - input.worldPosition);
            float3 reflectLight = reflect(spotLightDirectionOnSurface, normalize(input.normal));
            float3 halfVector = normalize(-spotLightDirectionOnSurface + toEye);
            float NdotH = dot(normalize(input.normal), halfVector);
            float specularPow = pow(saturate(NdotH), gMaterial.shininess);
            
            float NdotL = dot(normalize(input.normal), -spotLightDirectionOnSurface);
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
            //光の減衰
            float distance = length(gSceneLight.spotLights[k].position - input.worldPosition); //スポットライトへの距離
            float attenuationFactor = pow(saturate(-distance / gSceneLight.spotLights[k].distance + 1.0f), gSceneLight.spotLights[k].decay);
            //フォールオフ
            float cosAngle = dot(spotLightDirectionOnSurface, gSceneLight.spotLights[k].direction);
            float falloffFactor = saturate((cosAngle - gSceneLight.spotLights[k].cosAngle) / (gSceneLight.spotLights[k].cosFalloffStart - gSceneLight.spotLights[k].cosAngle));
        
            //拡散反射
            diffuseSpotLight += gMaterial.color.rgb * textureColor.rgb * gSceneLight.spotLights[k].color.rgb * cos * gSceneLight.spotLights[k].intensity * attenuationFactor * falloffFactor;
            //鏡面反射
            float3 specularColor = { 1.0f, 1.0f, 1.0f }; //この値はMaterialで変えられるようになるとよい。
            specularSpotLight += gSceneLight.spotLights[k].color.rgb * gSceneLight.spotLights[k].intensity * specularPow * specularColor * attenuationFactor * falloffFactor;
        }
    }
    
    //全ての拡散反射と鏡面反射の計算
    output.color.rgb = diffuseDirectionalLight + specularDirectionalLight + diffusePointLight + specularPointLight + diffuseSpotLight + specularSpotLight;
   
    //全てのライトがオフならライトの計算はしない
    if (useLightCount == 0)
    {
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb;
    }
    
    //α値の計算
    output.color.a = gMaterial.color.a * textureColor.a;

    return output;
}