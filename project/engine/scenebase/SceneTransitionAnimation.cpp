#include "SceneTransitionAnimation.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include <cassert>

SceneTransitionAnimation::SceneTransitionAnimation() {
}

SceneTransitionAnimation::~SceneTransitionAnimation() {
}

void SceneTransitionAnimation::Initialize() {
	//変数の初期化
	state_ = TransitionState::NONE;
	type_ = TransitionType::NONE;
	frame_ = 0;
	timer_ = 0;
	isTransitioning_ = false;
	//スプライトの初期化
	sprite_ = std::make_unique<Sprite>();
	textureHandle_ = TextureManager::GetInstance()->LoadTexture("black.png");
	sprite_->Initialize(textureHandle_);
}

void SceneTransitionAnimation::Update() {
	//スプライトの更新
	sprite_->Update();
}

void SceneTransitionAnimation::Draw() {
	//共通部分の描画設定
	SpriteCommon::GetInstance()->SettingCommonDrawing();
	//スプライトの描画
	if (isTransitioning_) {
		sprite_->Draw();
	}
}

void SceneTransitionAnimation::StartTransition() {
	//必要な変数の確認
	if (type_ == TransitionType::NONE) {
		assert(0 && "遷移の種類が設定されていません");
	}
	if (frame_ == 0) {
		assert(0 && "フレームが設定されていません");
	}

	//フェードイン開始
	state_ = TransitionState::UPDATE_IN;
	//遷移中フラグを立てる
	isTransitioning_ = true;
}

void SceneTransitionAnimation::UpdateIn() {
	switch (type_) {
	case SceneTransitionAnimation::TransitionType::FADE:
		//フェードイン処理
		if (state_ == TransitionState::UPDATE_IN) {
			//フレームが0になったら
			if (timer_ == 0) {
				//フェードイン終了
				state_ = TransitionState::END_IN;
			}
			else {
				//フレームを減らす
				timer_--;
				//透明度を計算
				float alpha = 1.0f - MyMath::Lerp(0.0f, 1.0f, static_cast<float>(timer_) / static_cast<float>(frame_));
				//スプライトの透明度を設定
				sprite_->SetColor({ 1.0f,1.0f,1.0f,alpha });
			}
		}
		break;
	default:
		break;
	}

}

void SceneTransitionAnimation::EndIn() {
	switch (type_) {
	case SceneTransitionAnimation::TransitionType::FADE:
		//フェードイン終了
		if (state_ == TransitionState::END_IN) {
			//フェードアウト開始
			state_ = TransitionState::UPDATE_OUT;
			//フレームをリセット
			timer_ = frame_;
			//スプライトの透明度を設定
			sprite_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
		}
		break;
	default:
		break;
	}
}

void SceneTransitionAnimation::UpdateOut() {
	switch (type_) {
	case SceneTransitionAnimation::TransitionType::FADE:
		//フェードアウト処理
		if (state_ == TransitionState::UPDATE_OUT) {
			//フレームが0になったら
			if (timer_ == 0) {
				//フェードアウト終了
				state_ = TransitionState::END_OUT;
			}
			else {
				//フレームを減らす
				timer_--;
				//透明度を計算
				float alpha = 1.0f - MyMath::Lerp(1.0f, 0.0f, static_cast<float>(timer_) / static_cast<float>(frame_));
				//スプライトの透明度を設定
				sprite_->SetColor({ 1.0f,1.0f,1.0f,alpha });
			}
		}
		break;
	default:
		break;
	}
}

void SceneTransitionAnimation::EndOut() {
	switch (type_) {
	case SceneTransitionAnimation::TransitionType::FADE:
		//フェードアウト終了
		if (state_ == TransitionState::END_OUT) {
			//遷移終了
			state_ = TransitionState::END_ALL;
			//遷移中フラグを下げる
			isTransitioning_ = false;
			//スプライトの透明度を設定
			sprite_->SetColor({ 1.0f,1.0f,1.0f,0.0f });
		}
		break;
	default:
		break;
	}
}

void SceneTransitionAnimation::EndAll() {
	//遷移終了
	state_ = TransitionState::NONE;
	type_ = TransitionType::NONE;
	frame_ = 0;
	timer_ = 0;
	//遷移中フラグを下げる
	isTransitioning_ = false;
}
