#include "Object3d.hlsli"

struct WorldTransformationMatrix
{
    float32_t4x4 World;
};
struct ViewProjectionTransformationMatrix
{
    float32_t4x4 View;
    float32_t4x4 Projection;
};
ConstantBuffer<WorldTransformationMatrix> gWorldTransformationMatrix : register(b0);
ConstantBuffer<ViewProjectionTransformationMatrix> gViewProjectionTransformationMatrix : register(b1);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, mul(mul(gWorldTransformationMatrix.World,gViewProjectionTransformationMatrix.View),gViewProjectionTransformationMatrix.Projection));
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3) gWorldTransformationMatrix.World));
    output.worldPosition = mul(input.position, gWorldTransformationMatrix.World).xyz;
    return output;
}
