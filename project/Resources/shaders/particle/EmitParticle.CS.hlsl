#include "ParticleCSCommon.hlsli"
#include "RandomUtility.hlsli"

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
void GenerateGrain(int generateNum);

[numthreads(1, 1, 1)] //スレッド1個(1fに1度だけ通る)
void main(uint3 DTid : SV_DispatchThreadID)
{
    //エミッターが稼働していなければ処理を行わない
    if (gEmitter.isPlay == 0)
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
    switch (gEmitter.effectStyle)
    {
        case 0: //ループ処理
            for (int i = 0; i < 60; i++)
            {
                //確率でこのフレームの生成数をインクリメント
                if (generator.GenerateInRange(0.0f, 100.0f) < ratePerFrame)
                    generateNum++;
            }
            break;
        case 1: //一度きり処理
            //現在の粒の数が0なら生成
            if (playingGrainNum == 0)
            {
                //生成数は1つだけ
                generateNum = 1;
            }
            //本来CPU側ではisPlayをここでfalseにするが、できなくなってしまったので
            //Emitが終了次第OneShotのスタイル限定でisPlayをオフにする処理を追加する。
            break;
        default:
            break;
    }
    //粒の生成
    if (generateNum > 0 && playingGrainNum + generateNum < max)
    {
        GenerateGrain(generateNum);
    }
    
}

void GenerateGrain(int generateNum)
{
    
}