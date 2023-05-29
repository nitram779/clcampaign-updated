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


LINK_ENTITY_TO_CLASS(weapon_needle, CNeedle);

void CNeedle::Spawn()
{
	Precache();
	m_iId = WEAPON_NEEDLE;
	SET_MODEL(ENT(pev), "models/w_needle.mdl");
	m_iClip = -1;

	FallInit(); // get ready to fall down.
}


void CNeedle::Precache()
{
	PRECACHE_MODEL("models/v_needle.mdl");
	PRECACHE_MODEL("models/w_needle.mdl");
	PRECACHE_MODEL("models/p_needle.mdl");
	PRECACHE_SOUND("weapons/needleshot.wav");

	m_usNeedle = PRECACHE_EVENT(1, "events/needle.sc");
}

bool CNeedle::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 3;
	p->iId = WEAPON_NEEDLE;
	p->iWeight = CROWBAR_WEIGHT;
	return true;
}



bool CNeedle::Deploy()
{
	return DefaultDeploy("models/v_needle.mdl", "models/p_needle.mdl", NEEDLE_DRAW, "needle");
}


void CNeedle::PrimaryAttack()
{
	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usNeedle);
	m_pPlayer->TakeHealth(25, DMG_GENERIC);
	m_flNextPrimaryAttack = GetNextAttackDelay(60.0);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 19.0 / 5.0;
}

void CNeedle::WeaponIdle()
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim(NEEDLE_IDLE);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.0;
}
