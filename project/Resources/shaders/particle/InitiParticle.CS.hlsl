#include "ParticleCSCommon.hlsli"

//粒の配列
RWStructuredBuffer<Grain> gGrains : register(u0);
//フリーリストのインデックス
RWStructuredBuffer<int> gFreeListIndex : register(u1);
//フリーリスト
RWStructuredBuffer<uint> gFreeList : register(u2);

//JSON情報の配列
ConstantBuffer<JsonInfo> gJsonInfo : register(b0);

[numthreads(1024, 1, 1)]
//UAVはCPU側で初期化できないので、こちら側で初期化処理を行う。
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint grainIndex = DTid.x;
    //稼働する必要のないスレッドでは計算処理を省く
    if (grainIndex >= gJsonInfo.maxGrains)
        return;
    
    //粒の全要素を0で埋める
    gGrains[grainIndex] = (Grain) 0;
    //FreeListを連番で初期化
    gFreeList[grainIndex] = grainIndex;
    //Indexが末尾を指すようにする
    if (grainIndex == 0)
    {
        gFreeListIndex[0] = gJsonInfo.maxGrains - 1;
    }
    
}