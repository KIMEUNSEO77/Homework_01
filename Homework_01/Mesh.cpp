// Mesh.cpp: CMesh 클래스의 구현


#include "stdafx.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"

#include <random>

// 랜덤 색상 생성하는 유틸리티 네임스페이스
namespace ColorUtils
{
	COLORREF GetRandomColor()
	{
		// C++11 random 라이브러리 사용 (static으로 두어 매번 생성하지 않도록 함)
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<int> dis(0, 255);

		// R, G, B 각각 0~255 사이의 랜덤 값 반환
		return RGB(dis(gen), dis(gen), dis(gen));
	}
}

CPolygon::CPolygon(int nVertices)
{
	m_nVertices = nVertices;
	m_pVertices = new CVertex[nVertices];
}

CPolygon::~CPolygon()
{
	if (m_pVertices) delete[] m_pVertices;
}
void CPolygon::SetVertex(int nIndex, CVertex vertex)
{
	if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices)
	{
		m_pVertices[nIndex] = vertex;
	}
}

CMesh::CMesh(int nPolygons)
{
	m_nPolygons = nPolygons;
	m_ppPolygons = new CPolygon * [nPolygons];
}

CMesh::~CMesh()
{
	if (m_ppPolygons)
	{
		for (int i = 0; i < m_nPolygons; i++) if (m_ppPolygons[i])
			delete m_ppPolygons[i];
		delete[] m_ppPolygons;
	}
}

void CMesh::SetPolygon(int nIndex, CPolygon* pPolygon)
{
	// 메쉬의 다각형을 설정 
	if ((0 <= nIndex) && (nIndex < m_nPolygons)) 
		m_ppPolygons[nIndex] = pPolygon;
}

void Draw2DLine(HDC hDCFrameBuffer, XMFLOAT3& f3PreviousProject, XMFLOAT3& f3CurrentProject)
{
	// 투영 좌표계 두 점을 화면 좌표계로 변환하고 변환된 두 점(픽셀)을 선분으로 그림
	XMFLOAT3 f3Previous = CGraphicsPipeline::ScreenTransform(f3PreviousProject);
	XMFLOAT3 f3Current = CGraphicsPipeline::ScreenTransform(f3CurrentProject);

	::MoveToEx(hDCFrameBuffer, (long)f3Previous.x, (long)f3Previous.y, NULL);
	::LineTo(hDCFrameBuffer, (long)f3Current.x, (long)f3Current.y);
}

// 면으로 그리는 함수
float EdgeFunction(const XMFLOAT3& a, const XMFLOAT3& b, float x, float y)
{
	return (x - a.x) * (b.y - a.y) - (y - a.y) * (b.x - a.x);
}

bool IsPointInTriangle(float x, float y, const XMFLOAT3& v0, const XMFLOAT3& v1, const XMFLOAT3& v2)
{
	float e0 = EdgeFunction(v0, v1, x, y);
	float e1 = EdgeFunction(v1, v2, x, y);
	float e2 = EdgeFunction(v2, v0, x, y);

	return ((e0 >= 0.0f) && (e1 >= 0.0f) && (e2 >= 0.0f)) ||
		((e0 <= 0.0f) && (e1 <= 0.0f) && (e2 <= 0.0f));
}

void FillTriangle2D(HDC hDCFrameBuffer,
	const XMFLOAT3& f3Projected0,
	const XMFLOAT3& f3Projected1,
	const XMFLOAT3& f3Projected2,
	COLORREF color)
{
	// 투영 좌표계 -> 화면 좌표계
	XMFLOAT3 v0 = CGraphicsPipeline::ScreenTransform(f3Projected0);
	XMFLOAT3 v1 = CGraphicsPipeline::ScreenTransform(f3Projected1);
	XMFLOAT3 v2 = CGraphicsPipeline::ScreenTransform(f3Projected2);

	POINT pts[3] =
	{
		{ (LONG)std::round(v0.x), (LONG)std::round(v0.y) },
		{ (LONG)std::round(v1.x), (LONG)std::round(v1.y) },
		{ (LONG)std::round(v2.x), (LONG)std::round(v2.y) }
	};

	HPEN hPen = CreatePen(PS_SOLID, 1, color);
	HBRUSH hBrush = CreateSolidBrush(color);

	HPEN oldPen = (HPEN)SelectObject(hDCFrameBuffer, hPen);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hDCFrameBuffer, hBrush);

	Polygon(hDCFrameBuffer, pts, 3);

	SelectObject(hDCFrameBuffer, oldPen);
	SelectObject(hDCFrameBuffer, oldBrush);

	DeleteObject(hPen);
	DeleteObject(hBrush);
}

void CMesh::Render(HDC hDCFrameBuffer)
{
	for (int j = 0; j < m_nPolygons; j++)
	{
		int nVertices = m_ppPolygons[j]->m_nVertices;
		CVertex* pVertices = m_ppPolygons[j]->m_pVertices;

		// 정점이 3개 미만이면 면을 만들 수 없음
		if (nVertices < 3) continue;

		// polygon의 모든 정점을 투영 좌표계로 미리 변환
		XMFLOAT3* pProjected = new XMFLOAT3[nVertices];

		for (int i = 0; i < nVertices; i++)
		{
			pProjected[i] = CGraphicsPipeline::Project(pVertices[i].m_xmf3Position);
		}

		// triangle fan 방식:
		// (0,1,2), (0,2,3), (0,3,4), ...
		for (int i = 1; i < nVertices - 1; i++)
		{
			XMFLOAT3 f3Projected0 = pProjected[0];
			XMFLOAT3 f3Projected1 = pProjected[i];
			XMFLOAT3 f3Projected2 = pProjected[i + 1];

			bool bInside0 =
				(-1.0f <= f3Projected0.x) && (f3Projected0.x <= 1.0f) &&
				(-1.0f <= f3Projected0.y) && (f3Projected0.y <= 1.0f) &&
				(0.0f <= f3Projected0.z) && (f3Projected0.z <= 1.0f);

			bool bInside1 =
				(-1.0f <= f3Projected1.x) && (f3Projected1.x <= 1.0f) &&
				(-1.0f <= f3Projected1.y) && (f3Projected1.y <= 1.0f) &&
				(0.0f <= f3Projected1.z) && (f3Projected1.z <= 1.0f);

			bool bInside2 =
				(-1.0f <= f3Projected2.x) && (f3Projected2.x <= 1.0f) &&
				(-1.0f <= f3Projected2.y) && (f3Projected2.y <= 1.0f) &&
				(0.0f <= f3Projected2.z) && (f3Projected2.z <= 1.0f);

			// 삼각형 3점이 전부 화면 안에 있을 때만 채움
			if (bInside0 && bInside1 && bInside2)
			{
				FillTriangle2D(hDCFrameBuffer,
					f3Projected0,
					f3Projected1,
					f3Projected2,
					m_ppPolygons[j]->GetColor());
			}
		}

		delete[] pProjected;
	}
}

CCubeMesh::CCubeMesh(float fWidth, float fHeight, float fDepth) : CMesh(6)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;

	CPolygon* pFrontFace = new CPolygon(4);
	pFrontFace->SetColor(ColorUtils::GetRandomColor());
	pFrontFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	SetPolygon(0, pFrontFace);

	CPolygon* pTopFace = new CPolygon(4);
	pTopFace->SetColor(ColorUtils::GetRandomColor());
	pTopFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	pTopFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pTopFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pTopFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	SetPolygon(1, pTopFace);

	CPolygon* pBackFace = new CPolygon(4);
	pBackFace->SetColor(ColorUtils::GetRandomColor());
	pBackFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	SetPolygon(2, pBackFace);

	CPolygon* pBottomFace = new CPolygon(4);
	pBottomFace->SetColor(ColorUtils::GetRandomColor());
	pBottomFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	pBottomFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	pBottomFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBottomFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	SetPolygon(3, pBottomFace);

	CPolygon* pLeftFace = new CPolygon(4);
	pLeftFace->SetColor(ColorUtils::GetRandomColor());
	pLeftFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	pLeftFace->SetVertex(1, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	pLeftFace->SetVertex(2, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	pLeftFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	SetPolygon(4, pLeftFace);

	CPolygon* pRightFace = new CPolygon(4);
	pRightFace->SetColor(ColorUtils::GetRandomColor());
	pRightFace->SetVertex(0, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pRightFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pRightFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pRightFace->SetVertex(3, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	SetPolygon(5, pRightFace);
}

CCubeMesh::~CCubeMesh()
{

}

CAirplaneMesh::CAirplaneMesh(float fWidth, float fHeight, float fDepth) : CMesh(24)
{
	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	float x1 = fx * 0.2f, y1 = fy * 0.2f, 
		x2 = fx * 0.1f, y3 = fy * 0.3f, y2 = ((y1 - (fy - y3)) / x1) * x2 + (fy - y3);
	int i = 0;

	// 비행기 메쉬의 위쪽 면
	CPolygon* pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);

	// 비행기 메쉬의 아래쪽 면
	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, +fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, +fz));
	SetPolygon(i++, pFace);

	// 비행기 메쉬의 오른쪽 면
	pFace = new CPolygon(3);
	pFace->SetColor(RGB(255, 255, 255));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(2, CVertex(+x2, +y2, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(255, 255, 255));
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(2, CVertex(+x2, +y2, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(255, 255, 255));
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(255, 255, 255));
	pFace->SetVertex(0, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(1, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);

	// 비행기 메쉬의 뒤쪽/오른쪽 면
	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, +fz));
	SetPolygon(i++, pFace);

	// 비행기 메쉬의 왼쪽 면
	pFace = new CPolygon(3);
	pFace->SetColor(RGB(255, 255, 255));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(2, CVertex(-x2, +y2, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(255, 255, 255));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(2, CVertex(-x2, +y2, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(255, 255, 255));
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(255, 255, 255));
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, +fz));
	SetPolygon(i++, pFace);

	// 비행기 메쉬의 뒤쪽/왼쪽 면
	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetColor(RGB(100, 100, 255));
	pFace->SetVertex(0, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);
}
