#include "ParticleCSCommon.hlsli"

//粒の配列
RWStructuredBuffer<Grain> gGrains : register(u0);
//フリーリストのインデックス
RWStructuredBuffer<int> gFreeListIndex : register(u1);
//フリーリスト
RWStructuredBuffer<uint> gFreeList : register(u2);

//エミッターの配列
ConstantBuffer<Emitter> gEmitter : register(b0);
//JSON情報の配列
ConstantBuffer<JsonInfo> gJsonInfo : register(b1);
//稼働制御用情報
ConstantBuffer<ParticleInformation> gParticleInformation : register(b2);
//フレーム情報
ConstantBuffer<PerFrame> gPerFrame : register(b3);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint grainIndex = DTid.x;
    //稼働する必要のないスレッドでは計算処理を省く
    if (grainIndex >= gParticleInformation.numGrains)
        return;
    
    //Outputから粒の情報を受け取る
    Grain grain = gGrains[grainIndex];
    //使用するエミッターを選択
    Emitter emitter = gEmitter;
    
    //現在時間の更新
    grain.currentTime += gPerFrame.deltaTime;
    ///==================///
    /// 粒の削除処理
    ///==================///
    
    
    
    //正規化時間
    float normalizedTime = saturate(grain.currentTime * rcp(grain.lifeTime));
    
    
    ///==================///
    /// エミッターとの処理
    ///==================///
    //重力処理
    if (emitter.isGravity == 1)
        grain.velocity.y += emitter.gravity * gPerFrame.deltaTime;
    //バウンド処理
    if (emitter.isBound == 1)
    {
        //粒の最底辺位置の計算
        float leg = grain.basicTransform.translate.y - lerp(grain.startSize, grain.endSize, normalizedTime);
        //床の反発処理
        if (leg > emitter.floorHeight && leg + (gPerFrame.deltaTime * grain.velocity.y) < emitter.floorHeight)
            grain.velocity.y *= (-1.0f) * emitter.repulsion;
    }
    ///==================///
    /// 粒情報の処理
    ///==================///
    //速度加算
    grain.basicTransform.translate = grain.basicTransform.translate + (gPerFrame.deltaTime * grain.velocity);
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
    gGrains[grainIndex] = grain;
}