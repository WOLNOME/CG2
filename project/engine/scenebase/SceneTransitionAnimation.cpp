#include "SceneTransitionAnimation.h"
#include "WinApp.h"
#include <cassert>

SceneTransitionAnimation::SceneTransitionAnimation()
	: state_(TransitionState::NONE), type_(TransitionType::NONE), frame_(0), timer_(0), isTransitioning_(false), alpha_(1.0f) {
}

SceneTransitionAnimation::~SceneTransitionAnimation() {
}

void SceneTransitionAnimation::Initialize() {
	//ブラシの色を決定
	ColorDecide();

	//変数の初期化
	alpha_ = 1.0f;
	state_ = TransitionState::NONE;
	type_ = TransitionType::NONE;
	frame_ = 0;
	timer_ = 0;
	isTransitioning_ = false;
}

void SceneTransitionAnimation::Update() {
	
}

void SceneTransitionAnimation::Draw() {
	//描画
	if (isTransitioning_) {
		DrawD2D();
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
				alpha_ = alpha;
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
			alpha_ = 1.0f;
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
				alpha_ = alpha;
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
			alpha_ = 0.0f;
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

void SceneTransitionAnimation::ColorDecide() {
	HRESULT hr;
	// 黒色ブラシの作成
	hr = d2drender->GetD2DDeviceContext()->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black),
		&blackBrush_
	);
	assert(SUCCEEDED(hr) && "カラーブラシの生成に失敗しました");
}

void SceneTransitionAnimation::DrawD2D() {
	// 黒幕を描画
	blackBrush_->SetOpacity(alpha_);
	d2drender->GetD2DDeviceContext()->FillRectangle(
		D2D1::RectF(0, 0, static_cast<float>(d2drender->GetD2DDeviceContext()->GetSize().width), static_cast<float>(d2drender->GetD2DDeviceContext()->GetSize().height)),
		blackBrush_.Get()
	);
}
