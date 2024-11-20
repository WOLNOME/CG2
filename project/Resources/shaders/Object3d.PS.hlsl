#include "Object3d.hlsli"

struct Material
{
    float32_t4 color;
    float32_t4x4 uvTransform;
    float32_t isTexture;
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
    int32_t isPointLight;
    int32_t isSpotLight;
};
struct CameraWorldPosition
{
    float32_t3 worldPosition;
};
struct PointLight
{
    float32_t4 color;
    float32_t3 position;
    float intensity;
    float radius;
    float decay;
};
struct SpotLight
{
    float32_t4 color;
    float32_t3 position;
    float intensity;
    float32_t3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
};


ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<LightFlag> gLightFlag : register(b2);
ConstantBuffer<CameraWorldPosition> gCameraWorldPosition : register(b3);
ConstantBuffer<PointLight> gPointLight : register(b4);
ConstantBuffer<SpotLight> gSpotLight : register(b5);

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
    float32_t3 diffuseDirectionalLight = { 0.0f, 0.0f, 0.0f };
    float32_t3 specularDirectionalLight = { 0.0f, 0.0f, 0.0f };
    
    if (gLightFlag.isDirectionalLight == 1)
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
        diffuseDirectionalLight = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        //鏡面反射
        float32_t3 specularColor = { 1.0f, 1.0f, 1.0f }; //この値はMaterialで変えられるようになるとよい。
        specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * specularColor;
    }
    
    //点光源の計算
    float32_t3 diffusePointLight = { 0.0f, 0.0f, 0.0f };
    float32_t3 specularPointLight = { 0.0f, 0.0f, 0.0f };
    
    if (gLightFlag.isPointLight == 1)
    {
        //光源から物体への方向
        float32_t3 pointLightDirection = normalize(input.worldPosition - gPointLight.position);
        //反射の計算
        float32_t3 toEye = normalize(gCameraWorldPosition.worldPosition - input.worldPosition);
        float32_t3 reflectLight = reflect(pointLightDirection, normalize(input.normal));
        float32_t3 halfVector = normalize(-pointLightDirection + toEye);
        float NdotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NdotH), gMaterial.shininess);
            
        float NdotL = dot(normalize(input.normal), -pointLightDirection);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        float32_t distance = length(gPointLight.position - input.worldPosition); //ポイントライトへの距離
        float32_t factor = pow(saturate(-distance / gPointLight.radius + 1.0f), gPointLight.decay);
        //拡散反射
        diffusePointLight = gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * cos * gPointLight.intensity * factor;
        //鏡面反射
        float32_t3 specularColor = { 1.0f, 1.0f, 1.0f }; //この値はMaterialで変えられるようになるとよい。
        specularPointLight = gPointLight.color.rgb * gPointLight.intensity * specularPow * specularColor * factor;
    }
    
    //スポットライトの計算
    float32_t3 diffuseSpotLight = { 0.0f, 0.0f, 0.0f };
    float32_t3 specularSpotLight = { 0.0f, 0.0f, 0.0f };
    
    if (gLightFlag.isSpotLight == 1)
    {
         //光源から物体表面への方向
        float32_t3 spotLightDirectionOnSurface = normalize(input.worldPosition - gSpotLight.position);
        //反射の計算
        float32_t3 toEye = normalize(gCameraWorldPosition.worldPosition - input.worldPosition);
        float32_t3 reflectLight = reflect(spotLightDirectionOnSurface, normalize(input.normal));
        float32_t3 halfVector = normalize(-spotLightDirectionOnSurface + toEye);
        float NdotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NdotH), gMaterial.shininess);
            
        float NdotL = dot(normalize(input.normal), -spotLightDirectionOnSurface);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        //光の減衰
        float32_t distance = length(gSpotLight.position - input.worldPosition); //スポットライトへの距離
        float32_t attenuationFactor = pow(saturate(-distance / gSpotLight.distance + 1.0f), gSpotLight.decay);
        //フォールオフ
        float32_t cosAngle = dot(spotLightDirectionOnSurface, gSpotLight.direction);
        float32_t falloffFactor = saturate((cosAngle - gSpotLight.cosAngle) / (gSpotLight.cosFalloffStart - gSpotLight.cosAngle));
        
        //拡散反射
        diffuseSpotLight = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * cos * gSpotLight.intensity * attenuationFactor * falloffFactor;
        //鏡面反射
        float32_t3 specularColor = { 1.0f, 1.0f, 1.0f }; //この値はMaterialで変えられるようになるとよい。
        specularSpotLight = gSpotLight.color.rgb * gSpotLight.intensity * specularPow * specularColor * attenuationFactor * falloffFactor;
    }
    
    
    //全ての拡散反射と鏡面反射の計算
    output.color.rgb = diffuseDirectionalLight + specularDirectionalLight + diffusePointLight + specularPointLight + diffuseSpotLight + specularSpotLight;
    if (gLightFlag.isDirectionalLight == 0 && gLightFlag.isPointLight == 0 && gLightFlag.isSpotLight == 0)
    {
        //光源がオフなら光の計算はしない
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb;
    }
    //α値の計算
    output.color.a = gMaterial.color.a * textureColor.a;

    return output;
}