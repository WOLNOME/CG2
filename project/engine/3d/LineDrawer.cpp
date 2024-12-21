#include "LineDrawer.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "SrvManager.h"
#include "BaseCamera.h"

LineDrawer::LineDrawer()
{
}

LineDrawer::~LineDrawer()
{
	//確保したSRVデスクリプタヒープの解放
	SrvManager::GetInstance()->Free(lineResource_.srvIndex);
	//ラインのリストクリア
	lines_.clear();
}

void LineDrawer::Initialize()
{
	//リソース作成
	lineResource_ = MakeLineResource();
	//インスタンシング用SRV設定
	SettingSRV();

}

void LineDrawer::Draw(const BaseCamera& camera)
{
	uint32_t instanceNum = 0;

	for (std::list<Line>::iterator lineIterator = lines_.begin(); lineIterator != lines_.end();) {
		if (instanceNum >= kNumMaxLine_) {
			break; // 最大インスタンス数に達したら終了
		}
		
		//レンダリングパイプライン
		Matrix4x4 worldMatrix = MyMath::MakeAffineMatrix(lineResource_.transform.scale, lineResource_.transform.rotate, lineResource_.transform.translate);
		lineResource_.instancingData[instanceNum].World = worldMatrix;
		lineResource_.instancingData[instanceNum].start = (*lineIterator).start;
		lineResource_.instancingData[instanceNum].end = (*lineIterator).end;
		lineResource_.instancingData[instanceNum].color = (*lineIterator).color;

		//次のインスタンスへ
		++instanceNum;
		//次のイテレータに進む
		++lineIterator;
	}
	//座標変換行列の場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(0, lineResource_.SrvHandleGPU);
	//カメラCBuffer場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, camera.GetViewProjectionConstBuffer()->GetGPUVirtualAddress());
	//頂点バッファービューを設定
	MainRender::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &lineResource_.vertexBufferView);
	//描画
	if (instanceNum > 0) {
		MainRender::GetInstance()->GetCommandList()->DrawInstanced(2, instanceNum, 0, 0);
	}

	//リストのクリア
	lines_.clear();
}

void LineDrawer::CreateLine(Vector3 start, Vector3 end, Vector4 color)
{
	// Line 構造体のインスタンスを作成し、引数のデータをセット
	Line newLine;
	newLine.start = { start.x,start.y,start.z,1.0f };
	newLine.end = { end.x,end.y,end.z,1.0f };
	newLine.color = color;

	// リストに新しいラインをプッシュ
	lines_.push_back(newLine);
}

void LineDrawer::ClearLine()
{
	lines_.clear();
}

LineDrawer::LineResource LineDrawer::MakeLineResource()
{
	LineResource lineResource;
	//各要素のリソース作成
	lineResource.vertexResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(VertexForVS) * 2);
	lineResource.instancingResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(LineForGPU) * kNumMaxLine_);
	//頂点バッファービューを作成
	lineResource.vertexBufferView.BufferLocation = lineResource.vertexResource->GetGPUVirtualAddress();
	lineResource.vertexBufferView.SizeInBytes = UINT(sizeof(VertexForVS) * 2);
	lineResource.vertexBufferView.StrideInBytes = sizeof(VertexForVS);
	//マッピング
	lineResource.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&lineResource.vertexData));
	lineResource.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&lineResource.instancingData));
	//データ書き込み
	lineResource.vertexData[0].position = { 0.0f,0.0f,0.0f };
	lineResource.vertexData[1].position = { 1.0f,0.0f,0.0f };
	lineResource.vertexData[0].vertexIndex = 0.0f;
	lineResource.vertexData[1].vertexIndex = 1.0f;
	//トランスフォーム
	lineResource.transform = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	//リターン
	return lineResource;
}

void LineDrawer::SettingSRV()
{
	//SRVマネージャーからデスクリプタヒープの空き番号を取得
	lineResource_.srvIndex = SrvManager::GetInstance()->Allocate();

	//srv設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = kNumMaxLine_;
	srvDesc.Buffer.StructureByteStride = sizeof(LineForGPU);
	lineResource_.SrvHandleCPU = SrvManager::GetInstance()->GetCPUDescriptorHandle(lineResource_.srvIndex);
	lineResource_.SrvHandleGPU = SrvManager::GetInstance()->GetGPUDescriptorHandle(lineResource_.srvIndex);
	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(lineResource_.instancingResource.Get(), &srvDesc, lineResource_.SrvHandleCPU);
}

