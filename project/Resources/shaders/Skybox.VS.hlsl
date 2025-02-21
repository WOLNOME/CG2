#include "Skybox.hlsli"

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
ConstantBuffer<WorldTransformationMatrix> gWorldTransformationMatrix : register(b0);
ConstantBuffer<ViewProjectionTransformationMatrix> gViewProjectionTransformationMatrix : register(b1);

struct VertexShaderInput
{
    float4 position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, mul(mul(gWorldTransformationMatrix.World, gViewProjectionTransformationMatrix.View), gViewProjectionTransformationMatrix.Projection)).xyww;
    output.texcoord = input.position.xyz;
    return output;
}
