////////////////////////////////////////////////////////////////////////////
// 
//
// Author:
//   Joakim Eriksson
//
////////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "bullet.h"

////////////////////////////////////////////////////////////////////////////
// 
CBullet::CBullet()
{
	m_State = BS_NONE;
	m_Pos.Zero();
	m_Vel.Zero();
	m_Size = 2.0f;
}

////////////////////////////////////////////////////////////////////////////
// 
CBullet::~CBullet()
{
}

////////////////////////////////////////////////////////////////////////////
// 
void		CBullet::Fire(const CVector2& pos, const CVector2& vel)
{
	m_State = BS_ACTIVE;
	m_Pos = pos;
	m_Vel = vel;
}

////////////////////////////////////////////////////////////////////////////
// 
void		CBullet::Update(f32 dt)
{
	if (m_State != BS_ACTIVE)
		return;
	m_Pos += m_Vel*dt;

	// Removed if moved outside screen
	if ((m_Pos.x < 0.0f) || (m_Pos.x > gRender.m_Width)	|| (m_Pos.y < 0.0f) || (m_Pos.y > gRender.m_Height))
		m_State = BS_NONE;
}

////////////////////////////////////////////////////////////////////////////
// 
void		CBullet::Draw(CRenderD3D* render)
{
	if (m_State != BS_ACTIVE)
		return;

	CRGBA	col(1.0f, 1.0f, 1.0f, 1.0f);
	render->DrawLine(m_Pos, m_Pos+Normalized(m_Vel)*m_Size, col, col);
}
