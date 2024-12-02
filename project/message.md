＜一日一機能！＞

エンジンでやりたいこと
・CG3の内容
・CG4の内容
・CG5の内容
・数学関数を使った当たり判定のシステム
・FBXファイルの読み込みで、アニメーションに対応させる
・パーティクルのブレンドモードの動的変化(パイプラインをあらかじめたくさん作る)
・シャドウマッピング
・アンリアルエンジンのようなメッシュの生成（円、UV球、立方体、円錐、円柱、平面）
・モデル描画時にライトの設定をしないといけないのを直す

<シャドウマップメモ>
model.cpp
object3d.cpp
directionalLight.cpp
shadowMapGenerator.cpp
↑コメントアウトファイル


<<リファレンス>>

 /////シャドウマップの計算/////
            
            // カメラの視点空間での深度を計算
            float viewSpaceDepth = length(input.worldPosition - gCameraWorldPosition.worldPosition);

            // デフォルトは最初のカスケード
            int cascadeIndex = 0;

            // カスケードスプリットに基づいて判定
            if (viewSpaceDepth <= gSceneLight.directionalLights[i].cascade[0].cascadeSplits)
                cascadeIndex = 0;
            else if (viewSpaceDepth <= gSceneLight.directionalLights[i].cascade[1].cascadeSplits)
                cascadeIndex = 1;
            else
                cascadeIndex = 2;
    
            // カスケードに対応するライトのビュープロジェクション行列を使用
            float4 lightSpacePosition = mul(gSceneLight.directionalLights[i].cascade[cascadeIndex].lightVPMatrix, float4(input.worldPosition, 1.0f));

            // ライト空間座標を正規化 (NDC空間 -> テクスチャ座標)
            float2 shadowUV = lightSpacePosition.xy / lightSpacePosition.w * 0.5f + 0.5f;

            // 深度値の比較用にライト空間の z 値を取得
            float shadowDepth = clamp(lightSpacePosition.z / lightSpacePosition.w, 0.0f, 1.0f);
    
            // シャドウマップの深度をサンプルして比較
            float shadowMapDepth = gDirLightShadowTexture[cascadeIndex].SampleCmpLevelZero(gShadowSampler, float3(shadowUV.x, shadowUV.y, i), shadowDepth).r;
            
            // バイアスを適用し、影の中かどうかを判定
            float bias = max(0.05f * (1.0f - dot(normalize(input.normal), gSceneLight.directionalLights[0].direction)), 0.005f);
            bool inShadow = (shadowDepth-bias > shadowMapDepth);

            float shadowFactor = inShadow ? 0.0f : 1.0f; // 影の中なら光を遮断
            
             //カスケードカラー
            float4 cascadeColor = { 1.0f, 1.0f, 1.0f, 1.0f };
            //if (cascadeIndex == 0)
            //{
            //    cascadeColor = float4(0.5f, 0.0f, 0.0f, 1.0f);

            //}
            //else if (cascadeIndex == 1)
            //{
            //    cascadeColor = float4(0.0f, 0.5f, 0.0f, 1.0f);
            //}
            //else
            //{
            //    cascadeColor = float4(0.0f, 0.0f, 0.5f, 1.0f);
            //}
            