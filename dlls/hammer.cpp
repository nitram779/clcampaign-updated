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
#include "crowbar.h"


#define HAMMER_BODYHIT_VOLUME 128
#define HAMMER_WALLHIT_VOLUME 512

LINK_ENTITY_TO_CLASS(weapon_hammer, CHammer);

void CHammer::Spawn()
{
	Precache();
	m_iId = WEAPON_HAMMER;
	SET_MODEL(ENT(pev), "models/w_hammer.mdl");
	m_iClip = -1;

	FallInit(); // get ready to fall down.
}


void CHammer::Precache()
{
	PRECACHE_MODEL("models/v_hammer.mdl");
	PRECACHE_MODEL("models/w_hammer.mdl");
	PRECACHE_MODEL("models/p_hammer.mdl");
	PRECACHE_SOUND("weapons/hammer_hit.wav");
	PRECACHE_SOUND("weapons/hammer_hitbod.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	m_usHammer = PRECACHE_EVENT(1, "events/hammer.sc");
}

bool CHammer::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = WEAPON_HAMMER;
	p->iWeight = CROWBAR_WEIGHT;
	return true;
}



bool CHammer::Deploy()
{
	return DefaultDeploy("models/v_hammer.mdl", "models/p_hammer.mdl", HAMMER_DRAW, "crowbar");
}


void CHammer::PrimaryAttack()
{
	if (!Swing(true))
	{
		SetThink(&CHammer::SwingAgain);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}


void CHammer::Smack()
{
	DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
}


void CHammer::SwingAgain()
{
	Swing(false);
}


bool CHammer::Swing(bool fFirst)
{
	bool fDidHit = false;

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos; // This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	if (fFirst)
	{
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usHammer,
			0.0, g_vecZero, g_vecZero, 0, 0, 0,
			0.0, 0, 0.0);
	}


	if (tr.flFraction >= 1.0)
	{
		if (fFirst)
		{
			// miss
			m_flNextPrimaryAttack = GetNextAttackDelay(1.0);

			// player "shoot" animation
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		}
	}
	else
	{
		SendWeaponAnim(HAMMER_ATTACK);

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifndef CLIENT_DLL

		// hit
		fDidHit = true;
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage();
		pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgHammer, gpGlobals->v_forward, &tr, DMG_CLUB);
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

#endif

		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);

#ifndef CLIENT_DLL
		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool fHitWorld = true;

		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				// play thwack or smack sound
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/hammer_hitbod.wav", 1, ATTN_NORM);
				m_pPlayer->m_iWeaponVolume = HAMMER_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
					return true;
				else
					flVol = 0.1;

				fHitWorld = false;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (fHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

			if (g_pGameRules->IsMultiplayer())
			{
				// override the volume here, cause we don't play texture sounds in multiplayer,
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play crowbar strike
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/hammer_hit.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * HAMMER_WALLHIT_VOLUME;
#endif
		SetThink(&CHammer::Smack);
		pev->nextthink = gpGlobals->time + 0.2;
	}
	return fDidHit;
}
