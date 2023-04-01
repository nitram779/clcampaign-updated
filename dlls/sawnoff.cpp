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

// special deathmatch shotgun spreads
#define VECTOR_CONE_DM_SAWNOFF Vector(0.17365, 0.04362, 0.00) // 20 degrees by 5 degrees

LINK_ENTITY_TO_CLASS(weapon_sawnoff, CSawnoff);

void CSawnoff::Spawn()
{
	Precache();
	m_iId = WEAPON_SAWNOFF;
	SET_MODEL(ENT(pev), "models/w_sawnoff.mdl");

	m_iDefaultAmmo = SAWNOFF_DEFAULT_GIVE;

	FallInit(); // get ready to fall
}


void CSawnoff::Precache()
{
	PRECACHE_MODEL("models/v_sawnoff.mdl");
	PRECACHE_MODEL("models/w_sawnoff.mdl");
	PRECACHE_MODEL("models/p_sawnoff.mdl");

	m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl"); // shotgun shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("weapons/sshotgun_shoot.wav"); //shotgun
	PRECACHE_SOUND("weapons/sshotgun_reload.wav"); // shotgun reload
	PRECACHE_SOUND("weapons/sshotgun_cock3.wav"); // gun empty sound

	m_usSawnoff = PRECACHE_EVENT(1, "events/sawnoff.sc");
}

bool CSawnoff::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = BUCKSHOT_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SAWNOFF_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 3;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SAWNOFF;
	p->iWeight = SHOTGUN_WEIGHT;

	return true;
}



bool CSawnoff::Deploy()
{
	return DefaultDeploy("models/v_sawnoff.mdl", "models/p_sawnoff.mdl", SAWNOFF_DRAW, "sawnoff");
}

void CSawnoff::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3 || m_iClip <= 1)
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sshotgun_cock3.wav", 1, ATTN_NORM);
		m_flNextPrimaryAttack = GetNextAttackDelay(0.15);
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip -= 2;


	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	Vector vecDir;

#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		// tuned for deathmatch
		vecDir = m_pPlayer->FireBulletsPlayer(4, vecSrc, vecAiming, VECTOR_CONE_DM_SAWNOFF, 2048, BULLET_PLAYER_SAWNOFF, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);
	}
	else
	{
		// untouched default single player
		vecDir = m_pPlayer->FireBulletsPlayer(6, vecSrc, vecAiming, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_SAWNOFF, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);
	}

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usSawnoff, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);

	if (0 == m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
	if (m_iClip != 0)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.0;
	else
		m_flTimeWeaponIdle = 0.5;
}


void CSawnoff::Reload()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == SAWNOFF_MAX_CLIP)
		return;
	DefaultReload(SAWNOFF_MAX_CLIP, SAWNOFF_RELOAD, 1.5);
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sshotgun_reload.wav", 1, ATTN_NORM, 0, 100);
}


void CSawnoff::WeaponIdle()
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_iClip == 0 && 0 != m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload();
		}
		else
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
			SendWeaponAnim(SAWNOFF_IDLE);
		}
	}
}
