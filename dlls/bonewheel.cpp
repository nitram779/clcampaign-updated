/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Bonewheel (Based on Zombie code)
//=========================================================

// UNDONE: Don't flinch every time you get hit

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"


//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define BONEWHEEL_AE_ATTACK 0x01
#define BONEWHEEL_FLINCH_DELAY 2 // at most one flinch every n secs

class CBonewheel : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	int IgnoreConditions() override;

	float m_flNextFlinch;

	void PainSound() override;
	void DeathSound() override;

	static const char* pPainSounds[];
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pDeathSounds[];

	// No range attacks
	bool CheckRangeAttack1(float flDot, float flDist) override { return false; }
	bool CheckRangeAttack2(float flDot, float flDist) override { return false; }
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
};

LINK_ENTITY_TO_CLASS(monster_bonewheel, CBonewheel);

const char* CBonewheel::pAttackHitSounds[] =
	{
		"zombie/claw_strike1.wav",
		"zombie/claw_strike2.wav",
		"zombie/claw_strike3.wav",
};

const char* CBonewheel::pAttackMissSounds[] =
	{
		"zombie/claw_miss1.wav",
		"zombie/claw_miss2.wav",
};

const char* CBonewheel::pPainSounds[] =
	{
		"spooky/s_pain1.wav",
		"spooky/s_pain2.wav",
		"spooky/s_pain3.wav",
};

const char* CBonewheel::pDeathSounds[] =
	{
		"spooky/s_die.wav",
		"spooky/s_die2.wav",
		"spooky/s_die3.wav",
};

//=========================================================
// Classify - indicates this monster's place in the
// relationship table.
//=========================================================
int CBonewheel::Classify()
{
	return CLASS_ALIEN_MONSTER;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CBonewheel::SetYawSpeed()
{
	int ys;

	ys = 120;

#if 0
	switch ( m_Activity )
	{
	}
#endif

	pev->yaw_speed = ys;
}

bool CBonewheel::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// Take 30% damage from bullets
	if (bitsDamageType == DMG_BULLET)
	{
		Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();
		float flForce = DamageForce(flDamage);
		pev->velocity = pev->velocity + vecDir * flForce;
		flDamage *= 0.3;
	}

	// HACK HACK -- until we fix this.
	if (IsAlive())
		DeathSound();
	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CBonewheel::PainSound()
{
	int pitch = 95 + RANDOM_LONG(0, 9);

	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CBonewheel::DeathSound()
{
	int pitch = 95 + RANDOM_LONG(0, 9);

	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_NORM, 0, pitch);
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CBonewheel::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case BONEWHEEL_AE_ATTACK:
	{
		// do stuff for this event.
		//		ALERT( at_console, "Slash right!\n" );
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.bonewheelDmgSlash, DMG_SLASH);
		if (pHurt)
		{
			if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100;
			}
			// Play a random attack hit sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else // Play a random attack miss sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CBonewheel::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/bonewheel.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = DONT_BLEED;
	pev->health = gSkillData.bonewheelHealth;
	pev->view_ofs = VEC_VIEW; // position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.5;	  // indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_DOORS_GROUP;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CBonewheel::Precache()
{
	PRECACHE_MODEL("models/bonewheel.mdl");

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================



int CBonewheel::IgnoreConditions()
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if ((m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK1))
	{
#if 0
		if (pev->health < 20)
			iIgnore |= (bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE);
		else
#endif
		if (m_flNextFlinch >= gpGlobals->time)
			iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	}

	if ((m_Activity == ACT_SMALL_FLINCH) || (m_Activity == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->time)
			m_flNextFlinch = gpGlobals->time + BONEWHEEL_FLINCH_DELAY;
	}

	return iIgnore;
}
