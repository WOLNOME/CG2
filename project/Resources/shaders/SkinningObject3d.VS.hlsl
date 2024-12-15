#include "SkinningObject3d.hlsli"

struct WorldTransformationMatrix
{
    float4x4 World; //ワールド行列
    float4x4 WorldInverseTranspose; //ワールド逆転置行列
};
struct ViewProjectionTransformationMatrix
{
    float4x4 View;
    float4x4 Projection;
};
struct Well
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInverseTransposeMatrix;
};
ConstantBuffer<WorldTransformationMatrix> gWorldTransformationMatrix : register(b0);
ConstantBuffer<ViewProjectionTransformationMatrix> gViewProjectionTransformationMatrix : register(b1);
StructuredBuffer<Well> gMatrixPalette : register(t0);

struct Skinned
{
    float4 position;
    float3 normal;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 weight : WEIGHT0;
    int4 index : INDEX0;
};

Skinned Skinning(VertexShaderInput input)
{
    Skinned skinned;
    //位置の変換
    skinned.position = mul(input.position, gMatrixPalette[input.index.x].skeletonSpaceMatrix) * input.weight.x;
    skinned.position += mul(input.position, gMatrixPalette[input.index.y].skeletonSpaceMatrix) * input.weight.y;
    skinned.position += mul(input.position, gMatrixPalette[input.index.z].skeletonSpaceMatrix) * input.weight.z;
    skinned.position += mul(input.position, gMatrixPalette[input.index.w].skeletonSpaceMatrix) * input.weight.w;
    skinned.position.w = 1.0f;
    //法線の変換
    skinned.normal = mul(input.normal, (float3x3)gMatrixPalette[input.index.x].skeletonSpaceInverseTransposeMatrix) * input.weight.x;
    skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[input.index.y].skeletonSpaceInverseTransposeMatrix) * input.weight.y;
    skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[input.index.z].skeletonSpaceInverseTransposeMatrix) * input.weight.z;
    skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[input.index.w].skeletonSpaceInverseTransposeMatrix) * input.weight.w;
    skinned.normal = normalize(skinned.normal);//正規化して戻す
    
    return skinned;
}

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    Skinned skinned = Skinning(input);
    //Skinningを使って変換
    output.position = mul(skinned.position, mul(mul(gWorldTransformationMatrix.World, gViewProjectionTransformationMatrix.View), gViewProjectionTransformationMatrix.Projection));
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(skinned.normal, (float3x3) gWorldTransformationMatrix.WorldInverseTranspose));
    output.worldPosition = mul(skinned.position, gWorldTransformationMatrix.World).xyz;
    return output;
}
