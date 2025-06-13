//トランスフォーム
struct Transform
{
    float4 scale;
    float4 rotate;
    float4 translate;
};
//エミッター情報
struct EmitterInfo
{
    Transform transform;
    int generateMethod;
    int effectStyle;
    float gravity;
    float repulsion;
    float floorHeight;
    int clumpNum;
    uint isAffectedField;
    uint isGravity;
    uint isBound;
    uint isBillboard;
    uint isPlay;
};
//JSON情報
struct JsonInfo
{
    float4 velocityMax;
    float4 velocityMin;
    float4 initRotateMax;
    float4 initRotateMin;
    float4 initScaleMax;
    float4 initScaleMin;
    float4 startColorMax;
    float4 startColorMin;
    float4 endColorMax;
    float4 endColorMin;
    float4 startRotateMax;
    float4 startRotateMin;
    float4 endRotateMax;
    float4 endRotateMin;
    float startSizeMax;
    float startSizeMin;
    float endSizeMax;
    float endSizeMin;
    float lifeTimeMax;
    float lifeTimeMin;
    int emitRate;
    int maxGrains;
};
//粒の情報
struct Grain
{
    Transform transform;
    Transform basicTransform;
    float4 velocity;
    float4 startColor;
    float4 endColor;
    float4 startRotate;
    float4 endRotate;
    float startSize;
    float endSize;
    float lifeTime;
    float currentTime;
};
//フレーム情報
struct PerFrame
{
    float time;
    float deltaTime;
};