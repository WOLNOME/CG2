#include "LineDrawer.hlsli"

struct LineForGPU
{
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4 start;
    float32_t4 end;
    float32_t4 color;
};
StructuredBuffer<LineForGPU> gLine : register(t0);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t vertexIndex : VERTEXINDEX0;
};

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    //もし頂点番号(始点か終点かのデータ)が0(始点)だったら
    if (input.vertexIndex == 0.0f)
    {
        output.position = mul(gLine[instanceId].start , gLine[instanceId].WVP);
        
    }
    //頂点番号が1(終点)だったら
    else
    {
        output.position = mul(gLine[instanceId].end, gLine[instanceId].WVP);
    }
    output.color = gLine[instanceId].color;
    return output;
}