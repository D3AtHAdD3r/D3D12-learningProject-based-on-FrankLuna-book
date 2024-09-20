#pragma once
#include<array>
#include"BoxApp.h"
#include <DirectXColors.h>


UINT geo_G_Num_of_Vertices = 14;
UINT geo_G_Num_of_Indices = 36;
std::array<Vertex, 14> vertices_G =
{
	Vertex({DirectX::XMFLOAT3(0, 0.8f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::White)}),
	Vertex({DirectX::XMFLOAT3(0, 0.8f, 0.5f), DirectX::XMFLOAT4(DirectX::Colors::Black)}),

	Vertex({DirectX::XMFLOAT3(-0.75f, 0.8f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Red)}),
	Vertex({DirectX::XMFLOAT3(-0.75f, 0.8f, 0.5f), DirectX::XMFLOAT4(DirectX::Colors::Green)}),

	Vertex({DirectX::XMFLOAT3(-1.0f, 0.0f, 0), DirectX::XMFLOAT4(DirectX::Colors::Blue)}),
	Vertex({DirectX::XMFLOAT3(-1.0f, 0.0f, 0.5f), DirectX::XMFLOAT4(DirectX::Colors::Yellow)}),

	Vertex({DirectX::XMFLOAT3(-0.7f, -0.8f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Cyan)}),
	Vertex({DirectX::XMFLOAT3(-0.7f, -0.8f, 0.5f), DirectX::XMFLOAT4(DirectX::Colors::Magenta)}),


	Vertex({DirectX::XMFLOAT3(0, -0.8f, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::White)}),
	Vertex({DirectX::XMFLOAT3(0, -0.8f, 0.5f), DirectX::XMFLOAT4(DirectX::Colors::Black)}),

	Vertex({DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT4(DirectX::Colors::Red)}),
	Vertex({DirectX::XMFLOAT3(0, 0, 0.5f), DirectX::XMFLOAT4(DirectX::Colors::Green)}), //11

	Vertex({DirectX::XMFLOAT3(-0.5f, 0, 0), DirectX::XMFLOAT4(DirectX::Colors::Blue)}),  //12
	Vertex({DirectX::XMFLOAT3(-0.5f, 0, 0.5f), DirectX::XMFLOAT4(DirectX::Colors::Cyan)}),  //13
};

std::array<std::uint16_t, 36> indices_G =
{
	0, 2, 1,
	2, 3, 1,

	2, 4, 3,
	4, 5, 3,

	4, 6, 7,
	7, 5, 4,

	6, 8, 7,
	8, 9, 7,

	8, 11, 9,
	8, 10, 11,

	10, 12, 11,
	12, 13, 11
};

//-----------------------------------------//
UINT geo_A_Num_of_Vertices = 10;
UINT geo_A_Num_of_Indices = 18;


std::array<Vertex, 10> vertices_A =
{

	Vertex({DirectX::XMFLOAT3(0,1.0f,0), DirectX::XMFLOAT4(DirectX::Colors::White)}),
	Vertex({DirectX::XMFLOAT3(0,1.0f,0.5f), DirectX::XMFLOAT4(DirectX::Colors::Black)}),

	Vertex({DirectX::XMFLOAT3(-1.0f, -1.0f, 0), DirectX::XMFLOAT4(DirectX::Colors::Red)}),
	Vertex({DirectX::XMFLOAT3(-1.0f, -1.0f, 0.5f), DirectX::XMFLOAT4(DirectX::Colors::Green)}),

	Vertex({DirectX::XMFLOAT3(1.0f, -1.0f, 0), DirectX::XMFLOAT4(DirectX::Colors::Blue)}),
	Vertex({DirectX::XMFLOAT3(1.0f, -1.0f, 0.5f), DirectX::XMFLOAT4(DirectX::Colors::Yellow)}),

	Vertex({DirectX::XMFLOAT3(0.5f, 0, 0), DirectX::XMFLOAT4(DirectX::Colors::Cyan)}),
	Vertex({DirectX::XMFLOAT3(0.5f, 0, 0.5f), DirectX::XMFLOAT4(DirectX::Colors::Magenta)}),


	Vertex({DirectX::XMFLOAT3(-0.5f, 0, 0), DirectX::XMFLOAT4(DirectX::Colors::White)}),
	Vertex({DirectX::XMFLOAT3(-0.5f, 0, 0.5f), DirectX::XMFLOAT4(DirectX::Colors::Black)}),

};

std::array<std::uint16_t, 18> indices_A =
{
	0, 2, 1,
	2, 3, 1,

	4, 1, 5,
	4, 0, 1,

	6,8, 7,
	8,9, 7

};

