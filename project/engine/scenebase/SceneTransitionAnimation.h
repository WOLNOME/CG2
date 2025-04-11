#pragma once
#include "Sprite.h"
#include "MyMath.h"
#include <cstdint>
#include <memory>

class SceneTransitionAnimation {
public:
	//遷移の状態
	enum class TransitionState {
		NONE,
		UPDATE_IN,
		END_IN,
		UPDATE_OUT,
		END_OUT,
		END_ALL,
	};
	//遷移の種類
	enum class TransitionType {
		NONE,
		FADE,
	};
public:
	SceneTransitionAnimation();
	~SceneTransitionAnimation();
	void Initialize();
	void Update();
	void Draw();

	//遷移処理全般
	void StartTransition();
	void UpdateIn();
	void EndIn();
	void UpdateOut();
	void EndOut();
	void EndAll();

	//ゲッター
	TransitionState GetState() const { return state_; }
	bool IsTransitioning() const { return isTransitioning_; }
	//セッター
	void SetTransitionType(TransitionType type) { type_ = type; }
	void SetFrame(const uint32_t frame) { frame_ = frame; timer_ = frame; }

private:
	//遷移の状態
	TransitionState state_;
	//遷移の種類
	TransitionType type_;
	//フレーム
	uint32_t frame_;
	uint32_t timer_;
	//遷移中フラグ
	bool isTransitioning_;

	//スプライト
	uint32_t textureHandle_;
	std::unique_ptr<Sprite> sprite_;
};

