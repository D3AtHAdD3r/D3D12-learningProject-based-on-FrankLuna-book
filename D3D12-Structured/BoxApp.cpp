#include "BoxApp.h"
#include <DirectXColors.h>
#include<array>
#include<d3dcompiler.h>

BoxApp::BoxApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

BoxApp::~BoxApp()
{
}

bool BoxApp::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	BuildDescriptorHeaps(); //Creates descriptor heap for constant-buffer-view
	BuildConstantBuffers(); //creates Constan-Buffer-View
	BuildRootSignature();
	BuildShaderAndInputLayout();
	BuildboxGeometry();
	BuildPSO();

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}

void BoxApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	DirectX::XMStoreFloat4x4(&mProj, P);
}

void BoxApp::Update(const GameTimer& gt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// Build the view matrix.
	DirectX::XMVECTOR pos = DirectX::XMVectorSet(x, y, z, 1.0f);			//camera world pos
	DirectX::XMVECTOR target = DirectX::XMVectorZero();						//camera looking at origin
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
	DirectX::XMStoreFloat4x4(&mView, view);

	DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mWorld); //worldmatrix of box
	DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&mProj);
	DirectX::XMMATRIX worldViewProj = world * view * proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	ObjectConstants objConstants;
	DirectX::XMStoreFloat4x4(&objConstants.WorldViewProj, DirectX::XMMatrixTranspose(worldViewProj));
	mObjectCB->CopyData(0, objConstants);

}

void BoxApp::Draw(const GameTimer& gt)
{
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	D3D12_RESOURCE_BARRIER res_bar = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mCommandList->ResourceBarrier(1, &res_bar);

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE cur_backbuf_view = CurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE depth_view = DepthStencilView();
	mCommandList->OMSetRenderTargets(1, &cur_backbuf_view, true, &depth_view);

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	D3D12_VERTEX_BUFFER_VIEW vert_view[] = { mBoxGeo1->VertexBufferView(),  mBoxGeo2->VertexBufferView() };
	mCommandList->IASetVertexBuffers(0, 2, vert_view);


	D3D12_INDEX_BUFFER_VIEW index_view = mBoxGeo1->IndexBufferView();
	mCommandList->IASetIndexBuffer(&index_view);
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

	mCommandList->DrawIndexedInstanced(
		mBoxGeo1->DrawArgs["box"].IndexCount,
		1, 0, 0, 0);

	// Indicate a state transition on the resource usage.
	D3D12_RESOURCE_BARRIER res_bar2 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCommandList->ResourceBarrier(1, &res_bar2);

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0) {
		// Make each pixel correspond to a quarter of a degree.
		float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0) {
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void BoxApp::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap)));
}

void BoxApp::BuildConstantBuffers()
{
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);

	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();

	// Offset to the ith object constant buffer in the buffer.
	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	md3dDevice->CreateConstantBufferView(
		&cbvDesc,
		mCbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void BoxApp::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr) {
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));

}

void BoxApp::BuildShaderAndInputLayout()
{
	HRESULT hr = S_OK;

	mvsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	mInputLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
}

void BoxApp::BuildboxGeometry()
{
	std::array<Vertex1, 10> vertices1 =
	{
		Vertex1({DirectX::XMFLOAT3(-0.3f, -0.3f, 0.0f)}), //bl			0
		Vertex1({DirectX::XMFLOAT3(-0.3f, -0.3f, 0.5f)}), //bl-f		1
		Vertex1({DirectX::XMFLOAT3(-0.6f, 0.6f, 0.0f)}), //tl			2
		Vertex1({DirectX::XMFLOAT3(-0.6f, 0.6f, 0.5f)}), //tl-f			3
		Vertex1({DirectX::XMFLOAT3(0.0f, 0.3f, 0.0f)}), //c				4
		Vertex1({DirectX::XMFLOAT3(0.0f, 0.3f, 0.5f)}), //c-f			5
		Vertex1({DirectX::XMFLOAT3(0.3f, -0.3f, 0.0f)}), //br			6
		Vertex1({DirectX::XMFLOAT3(0.3f, -0.3f, 0.5f)}),//br-f			7
		Vertex1({DirectX::XMFLOAT3(0.6f, 0.6f, 0.0f)}),	//tr			8
		Vertex1({DirectX::XMFLOAT3(0.6f, 0.6f, 0.5f)}) //tr-f			9
	};

	std::array<Vertex2, 10> colors2 =
	{
		Vertex2(DirectX::XMFLOAT4(DirectX::Colors::White)),
		Vertex2(DirectX::XMFLOAT4(DirectX::Colors::Black)),
		Vertex2(DirectX::XMFLOAT4(DirectX::Colors::Red)),
		Vertex2(DirectX::XMFLOAT4(DirectX::Colors::Green)),
		Vertex2(DirectX::XMFLOAT4(DirectX::Colors::Blue)),
		Vertex2(DirectX::XMFLOAT4(DirectX::Colors::Cyan)),
		Vertex2(DirectX::XMFLOAT4(DirectX::Colors::Magenta)),
		Vertex2(DirectX::XMFLOAT4(DirectX::Colors::Salmon)),
		Vertex2(DirectX::XMFLOAT4(DirectX::Colors::Bisque)),
		Vertex2(DirectX::XMFLOAT4(DirectX::Colors::Purple))
	};

	std::array<std::uint16_t, 24> indices =
	{
		0, 1, 3,
		3, 2, 0,
		0, 1, 4,
		4, 1, 5,
		7, 4, 5,
		7, 4, 6,
		7, 9, 8,
		8, 6, 7
	};

	const UINT vbByteSize = (UINT)vertices1.size() * sizeof(Vertex1);
	const UINT vbColorsByteSize = colors2.size() * sizeof(Vertex2);
	const UINT ibBytesSize = (UINT)indices.size() * sizeof(std::uint16_t);

	mBoxGeo1 = std::make_unique<MeshGeometry>();
	mBoxGeo1->Name = "boxGeoV";

	mBoxGeo2 = std::make_unique<MeshGeometry>();
	mBoxGeo2->Name = "boxGeoC";

	//vertexbuffer #1 - blob
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo1->VertexBufferCPU));
	CopyMemory(mBoxGeo1->VertexBufferCPU->GetBufferPointer(), vertices1.data(), vbByteSize);

	//vertexbuffer #2 - blob
	ThrowIfFailed(D3DCreateBlob(vbColorsByteSize, &mBoxGeo2->VertexBufferCPU));
	CopyMemory(mBoxGeo2->VertexBufferCPU->GetBufferPointer(), colors2.data(), vbColorsByteSize);

	//Index buffer - blob
	ThrowIfFailed(D3DCreateBlob(ibBytesSize, &mBoxGeo1->IndexBufferCPU));
	CopyMemory(mBoxGeo1->IndexBufferCPU->GetBufferPointer(), indices.data(), ibBytesSize);

	//Create upload buffer and main buffers - vertex buffer, index buffer
	mBoxGeo1->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices1.data(), vbByteSize, mBoxGeo1->VertexBufferUploader);

	mBoxGeo1->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibBytesSize, mBoxGeo1->IndexBufferUploader);

	mBoxGeo2->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), colors2.data(), vbColorsByteSize, mBoxGeo2->VertexBufferUploader);


	mBoxGeo1->VertexByteStride = sizeof(Vertex1);
	mBoxGeo1->VertexBufferByteSize = vbByteSize;
	mBoxGeo1->IndexFormat = DXGI_FORMAT_R16_UINT;
	mBoxGeo1->IndexBufferByteSize = ibBytesSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mBoxGeo1->DrawArgs["box"] = submesh;

	mBoxGeo2->VertexByteStride = sizeof(Vertex2);
	mBoxGeo2->VertexBufferByteSize = vbColorsByteSize;
}

void BoxApp::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize()
	};


	D3D12_RASTERIZER_DESC raster_desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	raster_desc.CullMode = D3D12_CULL_MODE_NONE;
	raster_desc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoDesc.RasterizerState = raster_desc;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	D3D12_DEPTH_STENCIL_DESC dsv_desc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	//dsv_desc.DepthEnable = false;
	psoDesc.DepthStencilState = dsv_desc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;


	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}
