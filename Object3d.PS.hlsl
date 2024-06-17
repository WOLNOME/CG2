#include "object3d.hlsli"

//struct
struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    int32_t enableShadowing;
};
struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};
struct Ray
{
    float32_t3 origin;
    float32_t3 diff;
};
struct Sphere
{
    float32_t3 center;
    float radius;
};
struct MatInverseVPV
{
    float32_t4x4 ivpv;
};
    
struct PixelShaderOutput
{
    float32_t4 color : SV_Target0;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Sphere> gSphere : register(b2);
ConstantBuffer<MatInverseVPV> gMatInverseVPV : register(b3);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    //セルフシャドウのみ
    if (gMaterial.enableLighting != 0 && gMaterial.enableShadowing == 0)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    }
    //シャドウマップのみ
    else if (gMaterial.enableLighting == 0 && gMaterial.enableShadowing != 0)
    {
        //レイを作成
        Ray ray;
        //diffは平行光源と逆の向き
        ray.diff = -1.0f * gDirectionalLight.direction;
        //originは各ピクセルのワールド座標を入れる
        float32_t3 worldCoordinate;
        //Coordinateにスクリーン座標とvpvの逆行列をtransform
        worldCoordinate.x = input.position.x * gMatInverseVPV.ivpv._m00 + input.position.y * gMatInverseVPV.ivpv._m10 + input.position.z * gMatInverseVPV.ivpv._m20 + 1.0f * gMatInverseVPV.ivpv._m30;
        worldCoordinate.y = input.position.x * gMatInverseVPV.ivpv._m01 + input.position.y * gMatInverseVPV.ivpv._m11 + input.position.z * gMatInverseVPV.ivpv._m21 + 1.0f * gMatInverseVPV.ivpv._m31;
        worldCoordinate.z = input.position.x * gMatInverseVPV.ivpv._m02 + input.position.y * gMatInverseVPV.ivpv._m12 + input.position.z * gMatInverseVPV.ivpv._m22 + 1.0f * gMatInverseVPV.ivpv._m32;
        float w = input.position.x * gMatInverseVPV.ivpv._m03 + input.position.y * gMatInverseVPV.ivpv._m13 + input.position.z * gMatInverseVPV.ivpv._m23 + 1.0f * gMatInverseVPV.ivpv._m33;
        if (w != 0.0f)
        {
            worldCoordinate.x /= w;
            worldCoordinate.y /= w;
            worldCoordinate.z /= w;
        }
        
        //オリジンに当てはめる
        ray.origin = worldCoordinate;
        
        //球体とレイの当たり判定を求める
        //レイと球の中心の距離をdとする
        float d;
        float32_t3 cp;
        float32_t3 otc;
        float32_t3 proj;
        otc = gSphere.center - ray.origin;
        float n = otc.x * ray.diff.x + otc.y * ray.diff.y + otc.z * ray.diff.z;
        float l = pow(sqrt(pow(ray.diff.x, 2) + pow(ray.diff.y, 2) + pow(ray.diff.z, 2)), 2);
        proj.x = n / l * ray.diff.x;
        proj.y = n / l * ray.diff.y;
        proj.z = n / l * ray.diff.z;
        cp.x = ray.origin.x + proj.x;
        cp.y = ray.origin.y + proj.y;
        cp.z = ray.origin.z + proj.z;
        
        d = distance(gSphere.center, cp);
        
        //dが半径より小さいと当たっている→影ができる
        if (d < gSphere.radius)
        {
            float shadowDensity;
            const float shadowRange = 15.0f;
            float pixelToCenter;
            pixelToCenter = distance(gSphere.center, worldCoordinate);
            if (pixelToCenter >= shadowRange)
            {
                shadowDensity = 1.0f;
            }
            else if (pixelToCenter <= 0.0f)
            {
                shadowDensity = 0.0f;
            }
            else
            {
                shadowDensity = pixelToCenter / shadowRange;
            }
            
            
                output.color = gMaterial.color * textureColor * shadowDensity;
        }
        else
        {
            output.color = gMaterial.color * textureColor;

        }

    }
    //両方付けないもしくは付ける
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    
    
    return output;
}