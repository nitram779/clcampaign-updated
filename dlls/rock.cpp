/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"


LINK_ENTITY_TO_CLASS(weapon_rock, CRock);

void CRock::Spawn()
{
	Precache();
	m_iId = WEAPON_ROCK;
	SET_MODEL(ENT(pev), "models/w_rock.mdl");
	m_iClip = -1;

	FallInit(); // get ready to fall down.
}


void CRock::Precache()
{
	PRECACHE_MODEL("models/v_rock.mdl");
	PRECACHE_MODEL("models/w_rock.mdl");
	PRECACHE_MODEL("models/p_rock.mdl");
}

bool CRock::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 4;
	p->iId = WEAPON_ROCK;
	p->iWeight = CROWBAR_WEIGHT;
	return true;
}



bool CRock::Deploy()
{
	return DefaultDeploy("models/v_rock.mdl", "models/p_rock.mdl", ROCK_DRAW, "crowbar");
}


void CRock::PrimaryAttack()
{
	Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

	if (angThrow.x < 0)
		angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
	else
		angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

	float flVel = (90 - angThrow.x) * 4;
	if (flVel > 500)
		flVel = 1300;

	UTIL_MakeVectors(angThrow);
	Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;

	Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
	CGrenadeRock::ShootTimed(m_pPlayer->pev, vecSrc, vecThrow * 1.5);

	SendWeaponAnim(ROCK_THROW);

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_fRockThrow = true;

	m_flNextPrimaryAttack = GetNextAttackDelay(0.7);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7;
}

void CRock::WeaponIdle()
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_fRockThrow)
	{
		m_fRockThrow = false;
		SendWeaponAnim(ROCK_DRAW);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
	}
	else
	{
		SendWeaponAnim(ROCK_IDLE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
	}
}
