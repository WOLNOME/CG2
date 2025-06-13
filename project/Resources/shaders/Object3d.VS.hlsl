#include "Object3d.hlsli"

struct WorldTransformationMatrix
{
    float4x4 World;//ワールド行列
    float4x4 WorldInverseTranspose;//ワールド逆転置行列
};
struct CameraInfo
{
    float4x4 matWorld;
    float4x4 matView;
    float4x4 matProjection;
};
ConstantBuffer<WorldTransformationMatrix> gWorldTransformationMatrix : register(b0);
ConstantBuffer<CameraInfo> gCameraInfo : register(b1);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, mul(mul(gWorldTransformationMatrix.World, gCameraInfo.matView), gCameraInfo.matProjection));
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) gWorldTransformationMatrix.WorldInverseTranspose));
    output.worldPosition = mul(input.position, gWorldTransformationMatrix.World).xyz;
    return output;
}
