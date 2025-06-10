struct Well
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInverseTransposeMatrix;
};
struct Vertex
{
    float4 position;
    float2 texcoord;
    float3 normal;
};
struct VertexInfluence
{
    float4 weight;
    int4 index;
};
struct SkinningInformation
{
    uint numVertices;
};
// SkinningObject3d.VS.hlslで作ったものと同じPalette
StructuredBuffer<Well> gMatrixPalette : register(t0);
// VertexBufferViewのstream0として利用していた入力頂点
StructuredBuffer<Vertex> gInputVertices : register(t1);
// VertexBufferViewのstream1として利用していた入力インフルエンス
StructuredBuffer<VertexInfluence> gInfluences : register(t2);
// Skinning計算後の頂点データ。SkinnedVertex
RWStructuredBuffer<Vertex> gOutputVertices : register(u0);
// Skinningに関するちょっとした情報
ConstantBuffer<SkinningInformation> gSkinningInformation : register(b0);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint vertexIndex = DTid.x;
    //稼働する必要のないスレッドでは計算処理を省く
    if (vertexIndex < gSkinningInformation.numVertices)
    {
        // inputから頂点情報を受け取る
        Vertex input = gInputVertices[vertexIndex];
        VertexInfluence influence = gInfluences[vertexIndex];
        
        //skinning後の頂点を計算
        Vertex skinned;
        //UV座標はいじらない
        skinned.texcoord = input.texcoord;
        //位置の変換
        skinned.position = mul(input.position, gMatrixPalette[influence.index.x].skeletonSpaceMatrix) * influence.weight.x;
        skinned.position += mul(input.position, gMatrixPalette[influence.index.y].skeletonSpaceMatrix) * influence.weight.y;
        skinned.position += mul(input.position, gMatrixPalette[influence.index.z].skeletonSpaceMatrix) * influence.weight.z;
        skinned.position += mul(input.position, gMatrixPalette[influence.index.w].skeletonSpaceMatrix) * influence.weight.w;
        skinned.position.w = 1.0f;
        //法線の変換
        skinned.normal = mul(input.normal, (float3x3) gMatrixPalette[influence.index.x].skeletonSpaceInverseTransposeMatrix) * influence.weight.x;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.y].skeletonSpaceInverseTransposeMatrix) * influence.weight.y;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.z].skeletonSpaceInverseTransposeMatrix) * influence.weight.z;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.w].skeletonSpaceInverseTransposeMatrix) * influence.weight.w;
        skinned.normal = normalize(skinned.normal); //正規化して戻す
    
        // Skinning後の頂点データを書き込む
        gOutputVertices[vertexIndex] = skinned;
    }
}