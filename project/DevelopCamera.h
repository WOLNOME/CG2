#pragma once
#include "BaseCamera.h"

class DevelopCamera : public BaseCamera
{
public:
    DevelopCamera() : BaseCamera() {}
    virtual ~DevelopCamera() = default;
    //初期化
    void Initialize() override;
    //更新
    void Update();

};
