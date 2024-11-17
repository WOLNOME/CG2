#include "Particle.hlsli"

struct ParticleForGPU
{
    float32_t4x4 World;
    float32_t4 color;
};
struct ViewProjectionTransformationMatrix
{
    float32_t4x4 View;
    float32_t4x4 Projection;
    float32_t4 cameraPos;
};
StructuredBuffer<ParticleForGPU> gParticle : register(t0);
ConstantBuffer<ViewProjectionTransformationMatrix> gViewProjectionTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, mul(mul(gParticle[instanceId].World, gViewProjectionTransformationMatrix.View), gViewProjectionTransformationMatrix.Projection));
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3) gParticle[instanceId].World));
    output.color = gParticle[instanceId].color;
    return output;
}