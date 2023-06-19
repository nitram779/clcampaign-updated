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


#define KATANA_BODYHIT_VOLUME 128
#define KATANA_WALLHIT_VOLUME 512

LINK_ENTITY_TO_CLASS(weapon_katana, CKatana);

void CKatana::Spawn()
{
	Precache();
	m_iId = WEAPON_KATANA;
	SET_MODEL(ENT(pev), "models/w_katana.mdl");
	m_iClip = -1;

	FallInit(); // get ready to fall down.
}


void CKatana::Precache()
{
	PRECACHE_MODEL("models/v_katana.mdl");
	PRECACHE_MODEL("models/w_katana.mdl");
	PRECACHE_MODEL("models/p_katana.mdl");
	PRECACHE_MODEL("models/fedora.mdl");
	PRECACHE_SOUND("weapons/katana_draw.wav");
	PRECACHE_SOUND("weapons/katana_hit1.wav");
	PRECACHE_SOUND("weapons/katana_hit2.wav");
	PRECACHE_SOUND("weapons/katana_hitbod1.wav");
	PRECACHE_SOUND("weapons/katana_hitbod2.wav");
	PRECACHE_SOUND("weapons/katana_hitbod3.wav");
	PRECACHE_SOUND("weapons/katana_miss1.wav");
	PRECACHE_SOUND("weapons/katana_tip.wav");

	m_usKatana = PRECACHE_EVENT(1, "events/katana.sc");
}

bool CKatana::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = WEAPON_KATANA;
	p->iWeight = CROWBAR_WEIGHT;
	return true;
}



bool CKatana::Deploy()
{
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/katana_draw.wav", 1, ATTN_NORM);
	return DefaultDeploy("models/v_katana.mdl", "models/p_katana.mdl", KATANA_DRAW, "crowbar");
}

void CKatana::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(KATANA_HOLSTER);
}


void CKatana::PrimaryAttack()
{
	if (!Swing(true))
	{
		SetThink(&CKatana::SwingAgain);
		pev->nextthink = gpGlobals->time + 0.6;
	}
}


void CKatana::SecondaryAttack()
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
	CGrenadeFedora::ShootTimed(m_pPlayer->pev, vecSrc, vecThrow * 1.5);
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/katana_tip.wav", 1, ATTN_NORM);
	m_flNextSecondaryAttack = GetNextAttackDelay(2.0);
}


void CKatana::Smack()
{
	DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
}


void CKatana::SwingAgain()
{
	Swing(false);
}


bool CKatana::Swing(bool fFirst)
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
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usKatana,
			0.0, g_vecZero, g_vecZero, 0, 0, 0,
			0.0, 0, 0.0);
	}


	if (tr.flFraction >= 1.0)
	{
		if (fFirst)
		{
			// miss
			m_flNextPrimaryAttack = GetNextAttackDelay(0.6);

			// player "shoot" animation
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		}
	}
	else
	{
		switch (((m_iSwing++) % 2) + 1)
		{
		case 0:
			SendWeaponAnim(KATANA_ATTACK1HIT);
			break;
		case 1:
			SendWeaponAnim(KATANA_ATTACK2HIT);
			break;
		case 2:
			SendWeaponAnim(KATANA_ATTACK3HIT);
			break;
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifndef CLIENT_DLL

		// hit
		fDidHit = true;
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage();
		pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgKatana, gpGlobals->v_forward, &tr, DMG_CLUB);
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

#endif

		m_flNextPrimaryAttack = GetNextAttackDelay(0.6);

#ifndef CLIENT_DLL
		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool fHitWorld = true;

		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				// play thwack or smack sound
				switch (RANDOM_LONG(0, 2))
				{
				case 0:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/katana_hitbod1.wav", 1, ATTN_NORM);
					break;
				case 1:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/katana_hitbod2.wav", 1, ATTN_NORM);
					break;
				case 2:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/katana_hitbod3.wav", 1, ATTN_NORM);
					break;
				}
				m_pPlayer->m_iWeaponVolume = KATANA_BODYHIT_VOLUME;
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
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/katana_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			case 1:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/katana_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * KATANA_WALLHIT_VOLUME;
#endif
		SetThink(&CKatana::Smack);
		pev->nextthink = gpGlobals->time + 0.2;
	}
	return fDidHit;
}
