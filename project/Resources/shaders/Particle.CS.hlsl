//トランスフォーム
struct Transform
{
    float4 scale;
    float4 rotate;
    float4 translate;
};
//エミッター情報
struct Emitter
{
    Transform transform;
    float gravity;
    float repulsion;
    float floorHeight;
    int clumpNum;
    uint isAffectedField;
    uint isGravity;
    uint isBound;
    
    uint numGrains;     //粒の数
};
//粒の情報
struct Grain
{
    Transform transform;
    Transform basicTransform;
    float4 startColor;
    float4 endColor;
    float4 velocity;
    float4 startRotate;
    float4 endRotate;
    float startSize;
    float endSize;
    float lifeTime;
    float currentTime;
    
    int emitterIndex;       //エミッターのID
};
//エミッターの配列
StructuredBuffer<Emitter> gEmitters : register(t0);
//入力用粒の配列
StructuredBuffer<Grain> gInputGrains : register(t1);
//出力用粒の配列
RWStructuredBuffer<Grain> gOutputgrains : register(u0);

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}