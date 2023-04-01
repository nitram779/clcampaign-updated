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
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"
#include "UserMessages.h"

// special deathmatch pepsigun spreads
#define VECTOR_CONE_DM_PEPSIGUN Vector(0.08716, 0.04362, 0.00)		// 10 degrees by 5 degrees
#define VECTOR_CONE_DM_DOUBLEPEPSIGUN Vector(0.17365, 0.04362, 0.00) // 20 degrees by 5 degrees

LINK_ENTITY_TO_CLASS(weapon_pepsigun, CPepsiGun);

void CPepsiGun::Spawn()
{
	Precache();
	m_iId = WEAPON_PEPSIGUN;
	SET_MODEL(ENT(pev), "models/w_pepsigun.mdl");

	m_iDefaultAmmo = PEPSIGUN_DEFAULT_GIVE;

	FallInit(); // get ready to fall
}


void CPepsiGun::Precache()
{
	PRECACHE_MODEL("models/v_pepsigun.mdl");
	PRECACHE_MODEL("models/w_pepsigun.mdl");
	PRECACHE_MODEL("models/p_pepsigun.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/pepsigun_shoot.wav"); //pepsigun

	PRECACHE_SOUND("weapons/reload1.wav"); // pepsigun reload
	PRECACHE_SOUND("weapons/reload3.wav"); // pepsigun reload

	//	PRECACHE_SOUND ("weapons/sshell1.wav");	// pepsigun reload - played on client
	//	PRECACHE_SOUND ("weapons/sshell3.wav");	// pepsigun reload - played on client

	PRECACHE_SOUND("weapons/357_cock1.wav"); // gun empty sound
	PRECACHE_SOUND("weapons/scock1.wav");	 // cock gun

	//m_usSingleFire = PRECACHE_EVENT(1, "events/pepsigun.sc");
}

bool CPepsiGun::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Hand Grenade";
	p->iMaxAmmo1 = HANDGRENADE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SHOTGUN_MAX_CLIP;
	p->iSlot = 4;
	p->iPosition = 4;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_PEPSIGUN;
	p->iWeight = SHOTGUN_WEIGHT;

	return true;
}



bool CPepsiGun::Deploy()
{
	return DefaultDeploy("models/v_pepsigun.mdl", "models/p_pepsigun.mdl", PEPSIGUN_DRAW, "pepsigun");
}

void CPepsiGun::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = GetNextAttackDelay(0.15);
		return;
	}

	if (m_iClip <= 0)
	{
		Reload();
		if (m_iClip == 0)
			PlayEmptySound();
		return;
	}

	m_iClip--;
	Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

	if (angThrow.x < 0)
		angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
	else
		angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

	float flVel = (90 - angThrow.x) * 4;
	if (flVel > 500)
		flVel = 500;

	UTIL_MakeVectors(angThrow);

	Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;

	Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
	CGrenade::ShootTimed(m_pPlayer->pev, vecSrc, vecThrow * 2, 3);

	SendWeaponAnim(PEPSIGUN_FIRE);

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/pepsigun_shoot.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));

	if (0 == m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.75);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;
	m_fInSpecialReload = 0;
}


void CPepsiGun::Reload()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == SHOTGUN_MAX_CLIP)
		return;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	// check to see if we're ready to reload
	if (m_fInSpecialReload == 0)
	{
		SendWeaponAnim(PEPSIGUN_OPEN);
		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.6;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6;
		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
		return;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;
		// was waiting for gun to move to side
		m_fInSpecialReload = 2;

		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload1.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload3.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));

		SendWeaponAnim(PEPSIGUN_INSERT);

		m_flNextReload = UTIL_WeaponTimeBase() + 0.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
	else
	{
		// Add them to the clip
		m_iClip += 1;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;
		m_fInSpecialReload = 1;
	}
}


void CPepsiGun::WeaponIdle()
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_iClip == 0 && m_fInSpecialReload == 0 && 0 != m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload();
		}
		else if (m_fInSpecialReload != 0)
		{
			if (m_iClip != 8 && 0 != m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload();
			}
			else
			{
				// reload debounce has timed out
				SendWeaponAnim(PEPSIGUN_CLOSE);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
			}
		}
	}
}
