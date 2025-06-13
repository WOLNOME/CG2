#include "ParticleCSCommon.hlsli"

//粒の配列
RWStructuredBuffer<Grain> gGrains : register(u0);
//フリーリストのインデックス
RWStructuredBuffer<int> gFreeListIndex : register(u1);
//フリーリスト
RWStructuredBuffer<uint> gFreeList : register(u2);

//エミッターの配列
ConstantBuffer<EmitterInfo> gEmitterInfo : register(b0);
//JSON情報の配列
ConstantBuffer<JsonInfo> gJsonInfo : register(b1);
//フレーム情報
ConstantBuffer<PerFrame> gPerFrame : register(b2);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint grainIndex = DTid.x;
    //稼働する必要のないスレッドでは計算処理を省く
    if (grainIndex >= gJsonInfo.maxGrains)
        return;
    // 寿命が0の粒子は死亡済みと見なして即スキップ
    if (gGrains[grainIndex].lifeTime == 0)
        return;
    
    //Outputから粒の情報を受け取る
    Grain grain = gGrains[grainIndex];
    //使用するエミッターを選択
    EmitterInfo emitterInfo = gEmitterInfo;
    
    //現在時間の更新
    grain.currentTime += gPerFrame.deltaTime;
    ///==================///
    /// 粒の削除処理
    ///==================///
    //寿命を迎えたら
    if (grain.currentTime > grain.lifeTime)
    {
        //全データに0を入れる
        gGrains[grainIndex] = (Grain) 0;
        int freeListIndex;
        InterlockedAdd(gFreeListIndex[0], 1, freeListIndex);
        //最新のFreeListIndexの場所に死亡済みGrainのIndexを設定する。
        if ((freeListIndex + 1) < gJsonInfo.maxGrains)
        {
            gFreeList[freeListIndex + 1] = grainIndex;
            return;
        }
        else
        {
            //ここに来たら設定がおかしい。安全策をうっておく
            InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
        }
        
    }
    ///==================///
    /// エミッターとの処理
    ///==================///
    //正規化時間
    float normalizedTime = saturate(grain.currentTime * rcp(grain.lifeTime));
    //重力処理
    if (emitterInfo.isGravity == 1)
        grain.velocity.y += emitterInfo.gravity * gPerFrame.deltaTime;
    //バウンド処理
    if (emitterInfo.isBound == 1)
    {
        //粒の最底辺位置の計算
        float leg = grain.basicTransform.translate.y - lerp(grain.startSize, grain.endSize, normalizedTime);
        //床の反発処理
        if (leg > emitterInfo.floorHeight && leg + (gPerFrame.deltaTime * grain.velocity.y) < emitterInfo.floorHeight)
            grain.velocity.y *= (-1.0f) * emitterInfo.repulsion;
    }
    ///==================///
    /// 粒情報の処理
    ///==================///
    //速度加算
    grain.basicTransform.translate = grain.basicTransform.translate + (gPerFrame.deltaTime * grain.velocity);
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