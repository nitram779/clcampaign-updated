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
#include "monsters.h"
#include "player.h"
#include "gamerules.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(weapon_snipars, CSnipars);

bool CSnipars::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "snipars";
	p->iMaxAmmo1 = SNIPARS_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SNIPARS_MAX_CLIP;
	p->iFlags = 0;
	p->iSlot = 2;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_SNIPARS;
	p->iWeight = PYTHON_WEIGHT;

	return true;
}

void CSnipars::Spawn()
{
	pev->classname = MAKE_STRING("weapon_snipars"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_SNIPARS;
	SET_MODEL(ENT(pev), "models/w_sniper.mdl");

	m_iDefaultAmmo = SNIPARS_DEFAULT_GIVE;

	FallInit(); // get ready to fall down.
}


void CSnipars::Precache()
{
	PRECACHE_MODEL("models/v_sniper.mdl");
	PRECACHE_MODEL("models/w_sniper.mdl");
	PRECACHE_MODEL("models/p_sniper.mdl");

	PRECACHE_MODEL("models/w_357ammobox.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/sniper_shoot1.wav");
	PRECACHE_SOUND("weapons/sniper_shoot2.wav");
	PRECACHE_SOUND("weapons/sniper_shoot3.wav");
	PRECACHE_SOUND("weapons/sniper_shoot4.wav");
	PRECACHE_SOUND("weapons/sniper_shoot5.wav");

	m_usFireSnipars = PRECACHE_EVENT(1, "events/snipars.sc");
}

bool CSnipars::Deploy()
{
#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		// enable laser sight geometry.
		pev->body = 1;
	}
	else
	{
		pev->body = 0;
	}

	return DefaultDeploy("models/v_sniper.mdl", "models/p_sniper.mdl", SNIPARS_DRAW, "snipars", pev->body);
}


void CSnipars::Holster()
{
	m_fInReload = false; // cancel any reload in progress.

	if (m_pPlayer->m_iFOV != 0)
	{
		SecondaryAttack();
	}

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}

void CSnipars::SecondaryAttack()
{
//#ifdef CLIENT_DLL
//	if (!bIsMultiplayer())
//#else
//	if (!g_pGameRules->IsMultiplayer())
//#endif
//	{
//		return;
//	}

	if (m_pPlayer->m_iFOV != 0)
	{
		m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
	}
	else if (m_pPlayer->m_iFOV != 40)
	{
		m_pPlayer->m_iFOV = 40;
	}

	m_flNextSecondaryAttack = 0.5;
}

void CSnipars::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);


	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_SNIPARS, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireSnipars, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);

	if (0 == m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

	m_flNextPrimaryAttack = 1.50;
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}


void CSnipars::Reload()
{
	if (m_pPlayer->ammo_357 <= 0)
		return;

	if (m_pPlayer->m_iFOV != 0)
	{
		m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
	}

	bool bUseScope = false;
#ifdef CLIENT_DLL
	bUseScope = bIsMultiplayer();
#else
	bUseScope = g_pGameRules->IsMultiplayer();
#endif

	if (DefaultReload(10, SNIPARS_RELOAD, 2.0, bUseScope ? 1 : 0))
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 12.0 / 3.0;
	}
}


void CSnipars::WeaponIdle()
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = 4.0;

	bool bUseScope = false;
#ifdef CLIENT_DLL
	bUseScope = bIsMultiplayer();
#else
	bUseScope = g_pGameRules->IsMultiplayer();
#endif

	SendWeaponAnim(SNIPARS_IDLE1, bUseScope ? 1 : 0);
}


class CSniparsAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_357ammobox.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_357ammobox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBaseEntity* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_SNIPARSBOX_GIVE, "snipars", SNIPARS_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_snipars, CSniparsAmmo);
