// 入力レイアウトに対応する構造体
struct VertexInput
{
    float4 position : POSITION0; // モデル空間での位置（w成分を含む）
};

// 出力データ
struct VertexOutput
{
    float4 position : SV_POSITION; // Homogeneous Clip Space座標
};

// 定数バッファ (カメラ行列、ライト行列などを設定)
struct WorldMatrix
{
    float4x4 World;
    float4x4 WorldInverseTranspose;
};
struct ViewProjectionTransformationMatrix
{
    float4x4 ViewProjection;
};

ConstantBuffer<WorldMatrix> gWorldMatrix : register(b0);
ConstantBuffer<ViewProjectionTransformationMatrix> gViewProjectionTransformationMatrix : register(b1);

// メインのVertex Shader
VertexOutput main(VertexInput input)
{
    VertexOutput output;

    // 頂点座標をワールド空間に変換
    float4 worldPosition = mul(input.position, gWorldMatrix.World);

    // ライトのView-Projection行列を使って座標変換
    float4 lightSpacePosition = mul(worldPosition, gViewProjectionTransformationMatrix.ViewProjection);

    // Homogeneous Clip Space 座標として出力
    output.position = lightSpacePosition;

    return output;
}
