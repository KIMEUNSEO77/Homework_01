// Mesh.h: 메쉬 클래스 선언

#pragma once

class CPolygon;

class CMesh
{
public:
	CMesh() {}
	CMesh(int nPolygons);
	virtual ~CMesh();

private:
	// 인스턴싱을 위해 메쉬는 게임 객체들에게 공유될 수 있음
	// 다음 참조 값은 메쉬가 공유되는 게임 객체들의 개수를 나타냄
	int m_nReferences = 1;

public:
	// 메쉬가 게임 객체에 공유될 때마다 참조 값을 1씩 증가 시킴
	void AddRef() { m_nReferences++; }
	// 메쉬를 공유하는 게임 객체가 소멸될 때마다 참조값을 1씩 감소 시킴
	// 참조값이 0이 되면 메쉬를 소멸시킴
	void Release() {
		m_nReferences--; if (m_nReferences <= 0) delete this;
	}
	
private:
	// 메쉬를 구성하는 다각형들의 리스트
	int m_nPolygons = 0;    // 메쉬를 구성하는 폴리곤(면) 개수
	CPolygon** m_ppPolygons = nullptr;   // 폴리곤 포인터를 저장하는 배열의 주소
	// 폴리곤을 포인터로 저장하기 때문에 이중 포인터를 사용
	// 다형성, 메모리 관리, 객체 공유 가능한 이유 때문.

public:
	void SetPolygon(int nIndex, CPolygon* pPolygon);

	// 메쉬를 렌더링
	virtual void Render(HDC hDCFrameBuffer);
};

// 직육면체 클래스 선언
class CCubeMesh : public CMesh
{
public:
	CCubeMesh(float fWidth = 4.0f, float fHeight = 4.0f, float fDepth = 4.0f);
	virtual ~CCubeMesh();
};

class CPoint3D
{
public:
	CPoint3D() {}
	CPoint3D(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	virtual ~CPoint3D() {}

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

class CVertex
{
public:
	CVertex() {}
	CVertex(float x, float y, float z) { m_xmf3Position = XMFLOAT3(x, y, z); }
	virtual ~CVertex() {}

	// DirectX 제공 구조체
	XMFLOAT3 m_xmf3Position;
};

// 하나의 면
class CPolygon
{
public:
	CPolygon() {}
	CPolygon(int nVertices);
	virtual ~CPolygon();

	// 다각형(면)을 구성하는 정점들의 리스트
	int m_nVertices = 0;    // 폴리곤 구성하는 정점 개수
	CVertex* m_pVertices = nullptr;   // 정점을 저장하는 배열의 주소

	void SetVertex(int nIndex, CVertex vertex);
};

// 비행기 클래스 선언
class CAirplaneMesh : public CMesh
{
public:
	CAirplaneMesh(float fWidth, float fHeight, float fDepth);
	virtual ~CAirplaneMesh() {}
};

