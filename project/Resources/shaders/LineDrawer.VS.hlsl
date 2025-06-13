#include "LineDrawer.hlsli"

struct LineForGPU
{
    float4x4 World;
    float4 start;
    float4 end;
    float4 color;
};
struct CameraInfo
{
    float4x4 matWorld;
    float4x4 matView;
    float4x4 matProjection;
};
StructuredBuffer<LineForGPU> gLine : register(t0);
ConstantBuffer<CameraInfo> gCameraInfo : register(b0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float vertexIndex : VERTEXINDEX0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    //もし頂点番号(始点か終点かのデータ)が0(始点)だったら
    if (input.vertexIndex == 0.0f)
    {
        output.position = mul(gLine[instanceId].start, mul(mul(gLine[instanceId].World, gCameraInfo.matView), gCameraInfo.matProjection));
        
    }
    //頂点番号が1(終点)だったら
    else
    {
        output.position = mul(gLine[instanceId].end, mul(mul(gLine[instanceId].World, gCameraInfo.matView), gCameraInfo.matProjection));
    }
    output.color = gLine[instanceId].color;
    return output;
}