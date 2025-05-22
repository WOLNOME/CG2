#include "Particle.hlsli"

struct ParticleForGPU
{
    float4x4 World;
    float4 color;
};
struct ViewProjectionTransformationMatrix
{
    float4x4 View;
    float4x4 Projection;
};
StructuredBuffer<ParticleForGPU> gParticle : register(t0);
ConstantBuffer<ViewProjectionTransformationMatrix> gViewProjectionTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, mul(mul(gParticle[instanceId].World, gViewProjectionTransformationMatrix.View), gViewProjectionTransformationMatrix.Projection));
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) gParticle[instanceId].World));
    output.color = gParticle[instanceId].color;
    return output;
}