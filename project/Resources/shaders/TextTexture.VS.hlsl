#include "TextTexture.hlsli"

static const uint kNumVertex = 6;
static const float4 kPositions[kNumVertex] =
{
    float4(-1.0f, 1.0f, 0.0f, 1.0f), //左上
    float4(1.0f, 1.0f, 0.0f, 1.0f), //右上
    float4(-1.0f, -1.0f, 0.0f, 1.0f), //左下
    
    float4(-1.0f, -1.0f, 0.0f, 1.0f), //左下
    float4(1.0f, 1.0f, 0.0f, 1.0f), //右上
    float4(1.0f, -1.0f, 0.0f, 1.0f) //右下
};
static const float2 kTexCoords[kNumVertex] =
{
    float2(0.0f, 0.0f), //左上
    float2(1.0f, 0.0f), //右上
    float2(0.0f, 1.0f), //左下
    
    float2(0.0f, 1.0f), //左下
    float2(1.0f, 0.0f), //右上
    float2(1.0f, 1.0f)  //右下
};
VertexShaderOutput main(uint vertexId : SV_VertexID)
{
    VertexShaderOutput output;
    output.position = kPositions[vertexId];
    output.texcoord = kTexCoords[vertexId];
    return output;
}