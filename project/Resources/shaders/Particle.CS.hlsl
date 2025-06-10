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
};
//粒の情報
struct Grain
{
    Transform transform;
    Transform basicTransform;
    float4 startColor;
    float4 endColor;
    float4 currentColor;
    float4 velocity;
    float4 startRotate;
    float4 endRotate;
    float startSize;
    float endSize;
    float lifeTime;
    float currentTime;
    
    int emitterIndex; //エミッターのID
};
//追加情報
struct ParticleInformation
{
    uint numGrains; //全ての粒の数
};

//エミッターの配列
StructuredBuffer<Emitter> gEmitters : register(t0);
//入力用粒の配列
StructuredBuffer<Grain> gInputGrains : register(t1);
//出力用粒の配列
RWStructuredBuffer<Grain> gOutputgrains : register(u0);
//稼働制御用情報
ConstantBuffer<ParticleInformation> gParticleInformation : register(b0);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint grainIndex = DTid.x;
    //稼働する必要のないスレッドでは計算処理を省く
    if (grainIndex >= gParticleInformation.numGrains)
        return;
    
    //Inputから粒の情報を受け取る
    Grain grain = gInputGrains[grainIndex];
    //使用するエミッターを選択
    Emitter emitter = gEmitters[grain.emitterIndex];
    //デルタタイムを定義
    float deltaTime = 1.0f / 60.0f;
    
    //現在時間の更新
    grain.currentTime += deltaTime;
    //正規化時間
    float normalizedTime = saturate(grain.currentTime * rcp(grain.lifeTime));
    
    ///==================///
    /// エミッターとの処理
    ///==================///
    //重力処理
    if (emitter.isGravity == 1)
        grain.velocity.y += emitter.gravity * deltaTime;
    //バウンド処理
    if (emitter.isBound == 1)
    {
        //粒の最底辺位置の計算
        float leg = grain.basicTransform.translate.y - lerp(grain.startSize, grain.endSize, normalizedTime);
        //床の反発処理
        if (leg > emitter.floorHeight && leg + (deltaTime * grain.velocity.y) < emitter.floorHeight)
            grain.velocity.y *= (-1.0f) * emitter.repulsion;
    }
    ///==================///
    /// 粒情報の処理
    ///==================///
    //速度加算
    grain.basicTransform.translate = grain.basicTransform.translate + (deltaTime * grain.velocity);
    //色更新
    grain.currentColor = lerp(grain.startColor, grain.endColor, normalizedTime);
    //回転更新
    float4 currentRotate = lerp(grain.startRotate, grain.endRotate, normalizedTime);
    //サイズ更新
    float4 currentSize = lerp(grain.startSize, grain.endSize, normalizedTime);
    //各粒のトランスフォーム
    grain.transform.translate = grain.basicTransform.translate;
    grain.transform.rotate = grain.basicTransform.rotate + currentRotate;
    grain.transform.scale = grain.basicTransform.scale * currentSize;
    
    //更新後の粒データを書き込む
    gOutputgrains[grainIndex] = grain;
}