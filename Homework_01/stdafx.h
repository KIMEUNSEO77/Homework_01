// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <math.h>

#include <cmath>

// 윈도우 클라이언트 영역 크기
#define FRAME_BUFFER_WIDTH 1280
#define FRAME_BUFFER_HEIGHT 960

// DegreeToRadian : 각도를 라디안으로 변환하는 매크로
#define DegreeToRadian(x) float((x)*3.141592654f/180.0f)

#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")

// DirectX Math 라이브러리
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

// 편리한 함수 사용을 위한 네임스페이스
using namespace DirectX;
using namespace DirectX::PackedVector;

// 방향 매크로
#define DIR_FORWARD		0x01 
#define DIR_BACKWARD	0x02
#define DIR_LEFT		0x04
#define DIR_RIGHT		0x08
#define DIR_UP			0x10
#define DIR_DOWN		0x20

namespace Matrix4x4
{
	// 단위 행렬 생성 함수
	inline XMFLOAT4X4 Identity()
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixIdentity());

		return(xmmtx4x4Result);
	}
}

