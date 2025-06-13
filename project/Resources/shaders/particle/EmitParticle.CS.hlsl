#include "ParticleCommon.hlsli"
#include "../util/RandomUtility.hlsli"

//粒の配列
RWStructuredBuffer<Grain> gGrains : register(u0);
//フリーリストのインデックス
RWStructuredBuffer<int> gFreeListIndex : register(u1);
//フリーリスト
RWStructuredBuffer<uint> gFreeList : register(u2);

//エミッター情報
ConstantBuffer<EmitterInfo> gEmitterInfo : register(b0);
//JSON情報
ConstantBuffer<JsonInfo> gJsonInfo : register(b1);
//フレーム情報
ConstantBuffer<PerFrame> gPerFrame : register(b2);

//乱数ジェネレーター
class RandomGenerator
{
    float3 seed;

    float3 Generate3d()
    {
        seed = rand3dTo3d(seed);
        return seed;
    }
    float Generate1d()
    {
        float result = rand3dTo1d(seed);
        seed.x = result;
        return result;
    }
    float GenerateInRange(float min, float max)
    {
        return lerp(min, max, Generate1d());
    }
    float3 GenerateInRange(float3 min, float3 max)
    {
        return lerp(min, max, Generate3d());
    }
};

//粒生成関数
void GenerateGrain(int generateNum, RandomGenerator generator);

[numthreads(1, 1, 1)] //スレッド1個(1fに1度だけ通る)
void main(uint3 DTid : SV_DispatchThreadID)
{
    //エミッターが稼働していなければ処理を行わない
    if (gEmitterInfo.isPlay == 0)
        return;
    
    //生成に必要なローカル変数
    int max = gJsonInfo.maxGrains;
    int rate = gJsonInfo.emitRate;
    float ratePerFrame = rate * gPerFrame.deltaTime;
    int playingGrainNum = gJsonInfo.maxGrains - gFreeListIndex[0] - 1; //稼働中の粒の数
    int generateNum = 0; //このフレームで生成する数
    
    //乱数生成機の作成
    RandomGenerator generator;
    generator.seed = (DTid + gPerFrame.time) * gPerFrame.time;
    //エフェクトの生成スタイルによって分ける
    if (gEmitterInfo.effectStyle == 0)      //ループ処理
    {
        for (int i = 0; i < 60; i++)
        {
                //確率でこのフレームの生成数をインクリメント
            if (generator.GenerateInRange(0.0f, 100.0f) < ratePerFrame)
                generateNum++;
        }
    }
    else if (gEmitterInfo.effectStyle == 1)     //一度きり処理
    {
        //現在の粒の数が0なら生成
        if (playingGrainNum == 0)
        {
            //生成数は1つだけ
            generateNum = 1;
        }
        //本来CPU側ではisPlayをここでfalseにするが、できなくなってしまったので
        //Emitが終了次第OneShotのスタイル限定でisPlayをオフにする処理を追加する。
    }
    //粒の生成
    if (generateNum > 0 && playingGrainNum + generateNum < max)
    {
        GenerateGrain(generateNum, generator);
    }
}

void GenerateGrain(int generateNum, RandomGenerator generator)
{
    for (int i = 0; i < generateNum; i++)
    {
        int freeListIndex;
        
        //生成方法ごとに処理を分ける
        if (gEmitterInfo.generateMethod == 0)       //ランダム生成
        {
            //FreeListのIndexを1つ前に設定し、現在のIndexを取得する
            InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
            if (0 <= freeListIndex && freeListIndex < gJsonInfo.maxGrains)
            {
                uint grainIndex = gFreeList[freeListIndex];
                //値を入れていく
                gGrains[grainIndex].basicTransform.translate.x = generator.GenerateInRange(gEmitterInfo.transform.translate.x - gEmitterInfo.transform.scale.x, gEmitterInfo.transform.translate.x + gEmitterInfo.transform.scale.x);
                gGrains[grainIndex].basicTransform.translate.y = generator.GenerateInRange(gEmitterInfo.transform.translate.y - gEmitterInfo.transform.scale.y, gEmitterInfo.transform.translate.y + gEmitterInfo.transform.scale.y);
                gGrains[grainIndex].basicTransform.translate.z = generator.GenerateInRange(gEmitterInfo.transform.translate.z - gEmitterInfo.transform.scale.z, gEmitterInfo.transform.translate.z + gEmitterInfo.transform.scale.z);
                gGrains[grainIndex].basicTransform.rotate.x = generator.GenerateInRange(gJsonInfo.initRotateMin.x, gJsonInfo.initRotateMax.x);
                gGrains[grainIndex].basicTransform.rotate.y = generator.GenerateInRange(gJsonInfo.initRotateMin.y, gJsonInfo.initRotateMax.y);
                gGrains[grainIndex].basicTransform.rotate.z = generator.GenerateInRange(gJsonInfo.initRotateMin.z, gJsonInfo.initRotateMax.z);
                gGrains[grainIndex].basicTransform.scale.x = generator.GenerateInRange(gJsonInfo.initScaleMin.x, gJsonInfo.initScaleMax.x);
                gGrains[grainIndex].basicTransform.scale.y = generator.GenerateInRange(gJsonInfo.initScaleMin.y, gJsonInfo.initScaleMax.y);
                gGrains[grainIndex].basicTransform.scale.z = generator.GenerateInRange(gJsonInfo.initScaleMin.z, gJsonInfo.initScaleMax.z);
                gGrains[grainIndex].transform.translate = gGrains[grainIndex].basicTransform.translate;
                gGrains[grainIndex].transform.rotate = gGrains[grainIndex].basicTransform.rotate;
                gGrains[grainIndex].transform.scale = gGrains[grainIndex].basicTransform.scale;
                gGrains[grainIndex].startRotate.x = generator.GenerateInRange(gJsonInfo.startRotateMin.x, gJsonInfo.startRotateMax.x);
                gGrains[grainIndex].startRotate.y = generator.GenerateInRange(gJsonInfo.startRotateMin.y, gJsonInfo.startRotateMax.y);
                gGrains[grainIndex].startRotate.z = generator.GenerateInRange(gJsonInfo.startRotateMin.z, gJsonInfo.startRotateMax.z);
                gGrains[grainIndex].endRotate.x = generator.GenerateInRange(gJsonInfo.endRotateMin.x, gJsonInfo.endRotateMax.x);
                gGrains[grainIndex].endRotate.y = generator.GenerateInRange(gJsonInfo.endRotateMin.y, gJsonInfo.endRotateMax.y);
                gGrains[grainIndex].endRotate.z = generator.GenerateInRange(gJsonInfo.endRotateMin.z, gJsonInfo.endRotateMax.z);
                gGrains[grainIndex].startSize = generator.GenerateInRange(gJsonInfo.startSizeMin, gJsonInfo.startSizeMax);
                gGrains[grainIndex].endSize = generator.GenerateInRange(gJsonInfo.endSizeMin, gJsonInfo.endSizeMax);
                gGrains[grainIndex].startColor.r = generator.GenerateInRange(gJsonInfo.startColorMin.r, gJsonInfo.startColorMax.r);
                gGrains[grainIndex].startColor.g = generator.GenerateInRange(gJsonInfo.startColorMin.g, gJsonInfo.startColorMax.g);
                gGrains[grainIndex].startColor.b = generator.GenerateInRange(gJsonInfo.startColorMin.b, gJsonInfo.startColorMax.b);
                gGrains[grainIndex].startColor.a = generator.GenerateInRange(gJsonInfo.startColorMin.a, gJsonInfo.startColorMax.a);
                gGrains[grainIndex].endColor.r = generator.GenerateInRange(gJsonInfo.endColorMin.r, gJsonInfo.endColorMax.r);
                gGrains[grainIndex].endColor.g = generator.GenerateInRange(gJsonInfo.endColorMin.g, gJsonInfo.endColorMax.g);
                gGrains[grainIndex].endColor.b = generator.GenerateInRange(gJsonInfo.endColorMin.b, gJsonInfo.endColorMax.b);
                gGrains[grainIndex].endColor.a = generator.GenerateInRange(gJsonInfo.endColorMin.a, gJsonInfo.endColorMax.a);
                gGrains[grainIndex].velocity.x = generator.GenerateInRange(gJsonInfo.velocityMin.x, gJsonInfo.velocityMax.x);
                gGrains[grainIndex].velocity.y = generator.GenerateInRange(gJsonInfo.velocityMin.y, gJsonInfo.velocityMax.y);
                gGrains[grainIndex].velocity.z = generator.GenerateInRange(gJsonInfo.velocityMin.z, gJsonInfo.velocityMax.z);
                gGrains[grainIndex].lifeTime = generator.GenerateInRange(gJsonInfo.lifeTimeMin, gJsonInfo.lifeTimeMax);
                gGrains[grainIndex].currentTime = 0.0f;

            }
            else
            {
            //発生させられなかったので減らした分元に戻す。
                InterlockedAdd(gFreeListIndex[0], 1);
            //Emit中にParticleは消えないので、この後発生することはないためreturnして終わらせる
                return;
            }
        }
        else if (gEmitterInfo.generateMethod == 1)      //クランプ生成
        {
            //FreeListのIndexを1つ前に設定し、現在のIndexを取得する
            InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
            if (0 <= freeListIndex && freeListIndex < gJsonInfo.maxGrains)
            {
                uint grainIndex = gFreeList[freeListIndex];
            
                //共通の値は先に決めておく
                float4 basicTranslate = (float4) 0.0f;
                basicTranslate.x = generator.GenerateInRange(gEmitterInfo.transform.translate.x - gEmitterInfo.transform.scale.x, gEmitterInfo.transform.translate.x + gEmitterInfo.transform.scale.x);
                basicTranslate.y = generator.GenerateInRange(gEmitterInfo.transform.translate.y - gEmitterInfo.transform.scale.y, gEmitterInfo.transform.translate.y + gEmitterInfo.transform.scale.y);
                basicTranslate.z = generator.GenerateInRange(gEmitterInfo.transform.translate.z - gEmitterInfo.transform.scale.z, gEmitterInfo.transform.translate.z + gEmitterInfo.transform.scale.z);
                float4 velocity = (float4) 0.0f;
                velocity.x = generator.GenerateInRange(gJsonInfo.velocityMin.x, gJsonInfo.velocityMax.x);
                velocity.y = generator.GenerateInRange(gJsonInfo.velocityMin.y, gJsonInfo.velocityMax.y);
                velocity.z = generator.GenerateInRange(gJsonInfo.velocityMin.z, gJsonInfo.velocityMax.z);
                float lifeTime = 0.0f;
                lifeTime = generator.GenerateInRange(gJsonInfo.lifeTimeMin, gJsonInfo.lifeTimeMax);
                //クランプ数だけ回す
                for (int j = 0; j < gEmitterInfo.clumpNum; j++)
                {
                    //値を入れていく
                    gGrains[grainIndex].basicTransform.translate = basicTranslate;
                    gGrains[grainIndex].basicTransform.rotate.x = generator.GenerateInRange(gJsonInfo.initRotateMin.x, gJsonInfo.initRotateMax.x);
                    gGrains[grainIndex].basicTransform.rotate.y = generator.GenerateInRange(gJsonInfo.initRotateMin.y, gJsonInfo.initRotateMax.y);
                    gGrains[grainIndex].basicTransform.rotate.z = generator.GenerateInRange(gJsonInfo.initRotateMin.z, gJsonInfo.initRotateMax.z);
                    gGrains[grainIndex].basicTransform.scale.x = generator.GenerateInRange(gJsonInfo.initScaleMin.x, gJsonInfo.initScaleMax.x);
                    gGrains[grainIndex].basicTransform.scale.y = generator.GenerateInRange(gJsonInfo.initScaleMin.y, gJsonInfo.initScaleMax.y);
                    gGrains[grainIndex].basicTransform.scale.z = generator.GenerateInRange(gJsonInfo.initScaleMin.z, gJsonInfo.initScaleMax.z);
                    gGrains[grainIndex].transform.translate = basicTranslate;
                    gGrains[grainIndex].transform.rotate = gGrains[grainIndex].basicTransform.rotate;
                    gGrains[grainIndex].transform.scale = gGrains[grainIndex].basicTransform.scale;
                    gGrains[grainIndex].startRotate.x = generator.GenerateInRange(gJsonInfo.startRotateMin.x, gJsonInfo.startRotateMax.x);
                    gGrains[grainIndex].startRotate.y = generator.GenerateInRange(gJsonInfo.startRotateMin.y, gJsonInfo.startRotateMax.y);
                    gGrains[grainIndex].startRotate.z = generator.GenerateInRange(gJsonInfo.startRotateMin.z, gJsonInfo.startRotateMax.z);
                    gGrains[grainIndex].endRotate.x = generator.GenerateInRange(gJsonInfo.endRotateMin.x, gJsonInfo.endRotateMax.x);
                    gGrains[grainIndex].endRotate.y = generator.GenerateInRange(gJsonInfo.endRotateMin.y, gJsonInfo.endRotateMax.y);
                    gGrains[grainIndex].endRotate.z = generator.GenerateInRange(gJsonInfo.endRotateMin.z, gJsonInfo.endRotateMax.z);
                    gGrains[grainIndex].startSize = generator.GenerateInRange(gJsonInfo.startSizeMin, gJsonInfo.startSizeMax);
                    gGrains[grainIndex].endSize = generator.GenerateInRange(gJsonInfo.endSizeMin, gJsonInfo.endSizeMax);
                    gGrains[grainIndex].startColor.r = generator.GenerateInRange(gJsonInfo.startColorMin.r, gJsonInfo.startColorMax.r);
                    gGrains[grainIndex].startColor.g = generator.GenerateInRange(gJsonInfo.startColorMin.g, gJsonInfo.startColorMax.g);
                    gGrains[grainIndex].startColor.b = generator.GenerateInRange(gJsonInfo.startColorMin.b, gJsonInfo.startColorMax.b);
                    gGrains[grainIndex].startColor.a = generator.GenerateInRange(gJsonInfo.startColorMin.a, gJsonInfo.startColorMax.a);
                    gGrains[grainIndex].endColor.r = generator.GenerateInRange(gJsonInfo.endColorMin.r, gJsonInfo.endColorMax.r);
                    gGrains[grainIndex].endColor.g = generator.GenerateInRange(gJsonInfo.endColorMin.g, gJsonInfo.endColorMax.g);
                    gGrains[grainIndex].endColor.b = generator.GenerateInRange(gJsonInfo.endColorMin.b, gJsonInfo.endColorMax.b);
                    gGrains[grainIndex].endColor.a = generator.GenerateInRange(gJsonInfo.endColorMin.a, gJsonInfo.endColorMax.a);
                    gGrains[grainIndex].velocity = velocity;
                    gGrains[grainIndex].lifeTime = lifeTime;
                    gGrains[grainIndex].currentTime = 0.0f;
                    
                    //粒のインデックスを次の値へ
                    InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
                    if (0 <= freeListIndex && freeListIndex < gJsonInfo.maxGrains)
                    {
                        grainIndex = gFreeList[freeListIndex];
                    }
                    //空きがなかったらそのまま返す
                    else
                    {
                        //発生させられなかったので減らした分元に戻す。
                        InterlockedAdd(gFreeListIndex[0], 1);
                        //Emit中にParticleは消えないので、この後発生することはないためreturnして終わらせる
                        return;
                    }
                }
            }
            else
            {
                //発生させられなかったので減らした分元に戻す。
                InterlockedAdd(gFreeListIndex[0], 1);
                //Emit中にParticleは消えないので、この後発生することはないためreturnして終わらせる
                return;
            }
        }
            
        
    }

}