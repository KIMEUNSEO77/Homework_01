// Scene.h
#pragma once
#include <vector>
#include "GameObject.h"
#include "Camera.h"
#include "Player.h"
#include "Bullet.h"
#include "Fragment.h"
#include "EnemyObject.h"

// 마우스 피킹을 위한 Ray 구조체
struct Ray
{
	XMFLOAT3 origin;
	XMFLOAT3 direction;
};

class CScene
{
public:
	CScene(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	virtual ~CScene() {}

private:
	// 게임 객체들의 리스트
	std::vector<CGameObject*> m_Objects;

	CPlayer* m_pPlayer = nullptr;

	std::vector<CBullet*> m_Bullets;  // 총알 리스트

	CCubeMesh* m_pFragmentMesh = NULL;   // 파편용 작은 큐브 메쉬
	std::vector<CFragment*> m_Fragments; // 파편 리스트

	CCubeMesh* m_pEnemyMesh = NULL;      // 적 객체용 큐브 메쉬

	// 적 객체 스폰 타이머
	float m_fEnemySpawnElapsed = 0.0f;
	float m_fEnemySpawnInterval = 3.0f;

	// 일반 큐브 스폰 타이머
	float m_fObjectSpawnElapsed = 0.0f;
	float m_fObjectSpawnInterval = 5.0f;

	CGameObject* m_pFocusedTarget = nullptr;  // 현재 조준된 타겟

	int m_nScore = 0;  // 현재 점수
	bool m_bGameOver = false;  // 게임 오버 상태

public:
	// 게임 객체들을 생성하고 소멸
	virtual void BuildObjects();
	virtual void ReleaseObjects();

	// 게임 객체들을 애니메이션
	virtual void Animate(float fElapsedTime);

	// 게임 객체들을 렌더링
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	// 충돌 체크 함수
	void CheckBulletCollisions();
	// 죽은 총알 처리
	void RemoveDeadBullets();

	// 랜덤한 방향 벡터 생성
	XMFLOAT3 GetRandomDirection();
	// 랜덤한 위치 생성
	XMFLOAT3 GetRandomPosition(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
	// 파편 생성
	void CreateFragments(const XMFLOAT3& xmf3Position, DWORD dwColor);
	// 죽은 파편 처리
	void RemoveDeadFragments();

	// 적 생성
	void CreateEnemy();
	// 일반 큐브 생성
	void CreateObject();
	// 총알 생성
	void CreateBullet(const XMFLOAT3& xmf3Position,
		const XMFLOAT3& xmf3Direction,
		DWORD dwColor, BulletOwner owner);

	// 현재 조준된 타겟 반환
	CGameObject* GetFocusedTarget() const { return m_pFocusedTarget; }
	void FocusTargetByMouse(int x, int y);
	void ClearFocusedTarget();
	XMFLOAT3 GetFireDirection() const;
	bool HasFocusedTarget() const { return (m_pFocusedTarget != nullptr); }

	// 점수 관리
	int GetScore() const { return m_nScore; }
	void AddScore(int nScore) { m_nScore += nScore; }
	void ResetScore() { m_nScore = 0; }

	float m_fPlayTime = 0.0f;  // 게임 플레이 시간
	bool IsGameOver() const { return m_bGameOver; }  // 게임 오버 상태
	void GameOver();  // 게임 오버 처리

	// 윈도우 메시지(키보드, 마우스)를 처리
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID,
		WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID,
		WPARAM wParam, LPARAM lParam) { }

private:
	// 피킹 관련
	Ray GeneratePickingRay(int x, int y, CCamera* pCamera);
	bool IntersectRaySphere(const Ray& ray, const BoundingSphere& sphere, float& fHitDistance);
	CGameObject* PickObjectByRay(const Ray& ray, float* pfHitDistance = nullptr);
	void FireBulletToTarget(CGameObject* pTarget);
};

