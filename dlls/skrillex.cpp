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
// Skrillex (Based on Vortigaunt code)
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "squadmonster.h"
#include "schedule.h"
#include "effects.h"
#include "weapons.h"
#include "soundent.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define ISLAVE_AE_CLAW (1)
#define ISLAVE_AE_CLAWRAKE (2)
#define ISLAVE_AE_ZAP_POWERUP (3)
#define ISLAVE_AE_ZAP_SHOOT (4)
#define ISLAVE_AE_ZAP_DONE (5)

#define ISLAVE_MAX_BEAMS 8

class CSkrillex : public CSquadMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int ISoundMask() override;
	int Classify() override;
	int IRelationship(CBaseEntity* pTarget) override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	bool CheckRangeAttack1(float flDot, float flDist) override;
	bool CheckRangeAttack2(float flDot, float flDist) override;
	void CallForHelp(const char* szClassname, float flDist, EHANDLE hEnemy, Vector& vecLocation);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	void IdleSound() override;

	void Killed(entvars_t* pevAttacker, int iGib) override;

	void StartTask(Task_t* pTask) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	CUSTOM_SCHEDULES;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void ClearBeams();
	void ArmBeam(int side);
	void WackBeam(int side, CBaseEntity* pEntity);
	void ZapBeam(int side);
	void BeamGlow();

	int m_iBravery;

	CBeam* m_pBeam[ISLAVE_MAX_BEAMS];

	int m_iBeams;
	float m_flNextAttack;

	int m_voicePitch;

	EHANDLE m_hDead;
};
LINK_ENTITY_TO_CLASS(monster_skrillex, CSkrillex);


TYPEDESCRIPTION CSkrillex::m_SaveData[] =
	{
		DEFINE_FIELD(CSkrillex, m_iBravery, FIELD_INTEGER),

		DEFINE_ARRAY(CSkrillex, m_pBeam, FIELD_CLASSPTR, ISLAVE_MAX_BEAMS),
		DEFINE_FIELD(CSkrillex, m_iBeams, FIELD_INTEGER),
		DEFINE_FIELD(CSkrillex, m_flNextAttack, FIELD_TIME),

		DEFINE_FIELD(CSkrillex, m_voicePitch, FIELD_INTEGER),

		DEFINE_FIELD(CSkrillex, m_hDead, FIELD_EHANDLE),

};

IMPLEMENT_SAVERESTORE(CSkrillex, CSquadMonster);




//=========================================================
// Classify - indicates this monster's place in the
// relationship table.
//=========================================================
int CSkrillex::Classify()
{
	return CLASS_ALIEN_MILITARY;
}


int CSkrillex::IRelationship(CBaseEntity* pTarget)
{
	if ((pTarget->IsPlayer()))
		if ((pev->spawnflags & SF_MONSTER_WAIT_UNTIL_PROVOKED) != 0 && (m_afMemory & bits_MEMORY_PROVOKED) == 0)
			return R_NO;
	return CBaseMonster::IRelationship(pTarget);
}


void CSkrillex::CallForHelp(const char* szClassname, float flDist, EHANDLE hEnemy, Vector& vecLocation)
{
	// ALERT( at_aiconsole, "help " );

	// skip ones not on my netname
	if (FStringNull(pev->netname))
		return;

	CBaseEntity* pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByString(pEntity, "netname", STRING(pev->netname))) != NULL)
	{
		float d = (pev->origin - pEntity->pev->origin).Length();
		if (d < flDist)
		{
			CBaseMonster* pMonster = pEntity->MyMonsterPointer();
			if (pMonster)
			{
				pMonster->m_afMemory |= bits_MEMORY_PROVOKED;
				pMonster->PushEnemy(hEnemy, vecLocation);
			}
		}
	}
}


//=========================================================
// IdleSound
//=========================================================
void CSkrillex::IdleSound()
{
#if 0
	int side = RANDOM_LONG( 0, 1 ) * 2 - 1;

	ClearBeams( );
	ArmBeam( side );

	UTIL_MakeAimVectors( pev->angles );
	Vector vecSrc = pev->origin + gpGlobals->v_right * 2 * side;
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrc );
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(vecSrc.x);	// X
		WRITE_COORD(vecSrc.y);	// Y
		WRITE_COORD(vecSrc.z);	// Z
		WRITE_BYTE( 8 );		// radius * 0.1
		WRITE_BYTE( 255 );		// r
		WRITE_BYTE( 180 );		// g
		WRITE_BYTE( 96 );		// b
		WRITE_BYTE( 10 );		// time * 10
		WRITE_BYTE( 0 );		// decay * 0.1
	MESSAGE_END( );

	EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "debris/zap1.wav", 1, ATTN_NORM, 0, 100 );
#endif
}


//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards.
//=========================================================
int CSkrillex::ISoundMask()
{
	return bits_SOUND_WORLD |
		   bits_SOUND_COMBAT |
		   bits_SOUND_DANGER |
		   bits_SOUND_PLAYER;
}


void CSkrillex::Killed(entvars_t* pevAttacker, int iGib)
{
	ClearBeams();
	CSquadMonster::Killed(pevAttacker, iGib);
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CSkrillex::SetYawSpeed()
{
	int ys;

	switch (m_Activity)
	{
	case ACT_WALK:
		ys = 50;
		break;
	case ACT_RUN:
		ys = 70;
		break;
	case ACT_IDLE:
		ys = 50;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CSkrillex::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	// ALERT( at_console, "event %d : %f\n", pEvent->event, pev->frame );
	switch (pEvent->event)
	{
	case ISLAVE_AE_CLAW:
	{
		// SOUND HERE!
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.skrillexDmgClaw, DMG_SLASH);
		if (pHurt)
		{
			if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 5;
			}
		}
	}
	break;

	case ISLAVE_AE_CLAWRAKE:
	{
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.skrillexDmgClawrake, DMG_SLASH);
		if (pHurt)
		{
			if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 5;
			}
		}
	}
	break;

	case ISLAVE_AE_ZAP_POWERUP:
	{
		// speed up attack when on hard
		if (g_iSkillLevel == SKILL_HARD)
			pev->framerate = 1.5;

		UTIL_MakeAimVectors(pev->angles);

		if (m_iBeams == 0)
		{
			Vector vecSrc = pev->origin + gpGlobals->v_forward * 2;
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSrc);
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(vecSrc.x);			 // X
			WRITE_COORD(vecSrc.y);			 // Y
			WRITE_COORD(vecSrc.z);			 // Z
			WRITE_BYTE(12);					 // radius * 0.1
			WRITE_BYTE(255);				 // r
			WRITE_BYTE(180);				 // g
			WRITE_BYTE(96);					 // b
			WRITE_BYTE(20 / pev->framerate); // time * 10
			WRITE_BYTE(0);					 // decay * 0.1
			MESSAGE_END();
		}
		if (m_hDead != NULL)
		{
			WackBeam(-1, m_hDead);
			WackBeam(1, m_hDead);
		}
		else
		{
			ArmBeam(-1);
			ArmBeam(1);
			BeamGlow();
		}

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "skrillex/charge.wav", 1, ATTN_NORM, 0, 100);
		pev->skin = m_iBeams / 2;
	}
	break;

	case ISLAVE_AE_ZAP_SHOOT:
	{
		ClearBeams();

		if (m_hDead != NULL)
		{
			Vector vecDest = m_hDead->pev->origin + Vector(0, 0, 38);
			TraceResult trace;
			UTIL_TraceHull(vecDest, vecDest, dont_ignore_monsters, human_hull, m_hDead->edict(), &trace);

			if (0 == trace.fStartSolid)
			{
				CBaseEntity* pNew = Create("monster_skrillex", m_hDead->pev->origin, m_hDead->pev->angles);
				CBaseMonster* pNewMonster = pNew->MyMonsterPointer();
				pNew->pev->spawnflags |= 1;
				WackBeam(-1, pNew);
				WackBeam(1, pNew);
				UTIL_Remove(m_hDead);
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "hassault/hw_shoot1.wav", 1, ATTN_NORM, 0, RANDOM_LONG(130, 160));

				/*
					CBaseEntity *pEffect = Create( "test_effect", pNew->Center(), pev->angles );
					pEffect->Use( this, this, USE_ON, 1 );
					*/
				break;
			}
		}
		ClearMultiDamage();

		UTIL_MakeAimVectors(pev->angles);

		ZapBeam(-1);
		ZapBeam(1);

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "hassault/hw_shoot1.wav", 1, ATTN_NORM, 0, RANDOM_LONG(130, 160));
		// STOP_SOUND( ENT(pev), CHAN_WEAPON, "debris/zap4.wav" );
		ApplyMultiDamage(pev, pev);

		m_flNextAttack = gpGlobals->time + RANDOM_FLOAT(0.5, 4.0);
	}
	break;

	case ISLAVE_AE_ZAP_DONE:
	{
		ClearBeams();
	}
	break;

	default:
		CSquadMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// CheckRangeAttack1 - normal beam attack
//=========================================================
bool CSkrillex::CheckRangeAttack1(float flDot, float flDist)
{
	if (m_flNextAttack > gpGlobals->time)
	{
		return false;
	}

	return CSquadMonster::CheckRangeAttack1(flDot, flDist);
}

//=========================================================
// CheckRangeAttack2 - check bravery and try to resurect dead comrades
//=========================================================
bool CSkrillex::CheckRangeAttack2(float flDot, float flDist)
{
	return false;

	if (m_flNextAttack > gpGlobals->time)
	{
		return false;
	}

	m_hDead = NULL;
	m_iBravery = 0;

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "monster_skrillex")) != NULL)
	{
		TraceResult tr;

		UTIL_TraceLine(EyePosition(), pEntity->EyePosition(), ignore_monsters, ENT(pev), &tr);
		if (tr.flFraction == 1.0 || tr.pHit == pEntity->edict())
		{
			if (pEntity->pev->deadflag == DEAD_DEAD)
			{
				float d = (pev->origin - pEntity->pev->origin).Length();
				if (d < flDist)
				{
					m_hDead = pEntity;
					flDist = d;
				}
				m_iBravery--;
			}
			else
			{
				m_iBravery++;
			}
		}
	}
	if (m_hDead != NULL)
		return true;
	else
		return false;
}


//=========================================================
// StartTask
//=========================================================
void CSkrillex::StartTask(Task_t* pTask)
{
	ClearBeams();

	CSquadMonster::StartTask(pTask);
}


//=========================================================
// Spawn
//=========================================================
void CSkrillex::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/skrillex.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->effects = 0;
	pev->health = gSkillData.skrillexHealth;
	pev->view_ofs = Vector(0, 0, 64);  // position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_RANGE_ATTACK2 | bits_CAP_DOORS_GROUP;

	m_voicePitch = RANDOM_LONG(85, 110);

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CSkrillex::Precache()
{
	PRECACHE_MODEL("models/skrillex.mdl");
	PRECACHE_MODEL("sprites/lgtning.spr");
	PRECACHE_SOUND("debris/zap1.wav");
	PRECACHE_SOUND("skrillex/charge.wav");
	PRECACHE_SOUND("skrillex/fire.wav");
	PRECACHE_SOUND("hassault/hw_shoot1.wav");
	PRECACHE_SOUND("zombie/zo_pain2.wav");
	PRECACHE_SOUND("headcrab/hc_headbite.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	UTIL_PrecacheOther("test_effect");
}


//=========================================================
// TakeDamage - get provoked when injured
//=========================================================

bool CSkrillex::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// don't slash one of your own
	if ((bitsDamageType & DMG_SLASH) != 0 && pevAttacker && IRelationship(Instance(pevAttacker)) < R_DL)
		return false;

	m_afMemory |= bits_MEMORY_PROVOKED;
	return CSquadMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}


void CSkrillex::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	if ((bitsDamageType & DMG_SHOCK) != 0)
		return;

	CSquadMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}


//=========================================================
// AI Schedules Specific to this monster
//=========================================================



// primary range attack
Task_t tlSkrillexAttack1[] =
	{
		{TASK_STOP_MOVING, 0},
		{TASK_FACE_IDEAL, (float)0},
		{TASK_RANGE_ATTACK1, (float)0},
};

Schedule_t slSkrillexAttack1[] =
	{
		{tlSkrillexAttack1,
			ARRAYSIZE(tlSkrillexAttack1),
			bits_COND_CAN_MELEE_ATTACK1 |
				bits_COND_HEAR_SOUND |
				bits_COND_HEAVY_DAMAGE,

			bits_SOUND_DANGER,
			"Skrillex Range Attack1"},
};


DEFINE_CUSTOM_SCHEDULES(CSkrillex){
	slSkrillexAttack1,
};

IMPLEMENT_CUSTOM_SCHEDULES(CSkrillex, CSquadMonster);


//=========================================================
//=========================================================
Schedule_t* CSkrillex::GetSchedule()
{
	ClearBeams();

	/*
	if (pev->spawnflags)
	{
		pev->spawnflags = 0;
		return GetScheduleOfType( SCHED_RELOAD );
	}
*/

	if (HasConditions(bits_COND_HEAR_SOUND))
	{
		CSound* pSound;
		pSound = PBestSound();

		ASSERT(pSound != NULL);

		if (pSound && (pSound->m_iType & bits_SOUND_DANGER) != 0)
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
		if ((pSound->m_iType & bits_SOUND_COMBAT) != 0)
			m_afMemory |= bits_MEMORY_PROVOKED;
	}

	switch (m_MonsterState)
	{
	case MONSTERSTATE_COMBAT:
		// dead enemy
		if (HasConditions(bits_COND_ENEMY_DEAD))
		{
			// call base class, all code to handle dead enemies is centralized there.
			return CBaseMonster::GetSchedule();
		}

		if (pev->health < 20 || m_iBravery < 0)
		{
			if (!HasConditions(bits_COND_CAN_MELEE_ATTACK1))
			{
				m_failSchedule = SCHED_CHASE_ENEMY;
				if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
				{
					return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
				}
				if (HasConditions(bits_COND_SEE_ENEMY) && HasConditions(bits_COND_ENEMY_FACING_ME))
				{
					// ALERT( at_console, "exposed\n");
					return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
				}
			}
		}
		break;
	}
	return CSquadMonster::GetSchedule();
}


Schedule_t* CSkrillex::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_FAIL:
		if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
		{
			return CSquadMonster::GetScheduleOfType(SCHED_MELEE_ATTACK1);
		}
		break;
	case SCHED_RANGE_ATTACK1:
		return slSkrillexAttack1;
	case SCHED_RANGE_ATTACK2:
		return slSkrillexAttack1;
	}
	return CSquadMonster::GetScheduleOfType(Type);
}


//=========================================================
// ArmBeam - small beam from arm to nearby geometry
//=========================================================

void CSkrillex::ArmBeam(int side)
{
	TraceResult tr;
	float flDist = 1.0;

	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	UTIL_MakeAimVectors(pev->angles);
	Vector vecSrc = pev->origin + gpGlobals->v_up * 36 + gpGlobals->v_right * side * 16 + gpGlobals->v_forward * 32;

	for (int i = 0; i < 3; i++)
	{
		Vector vecAim = gpGlobals->v_right * side * RANDOM_FLOAT(0, 1) + gpGlobals->v_up * RANDOM_FLOAT(-1, 1);
		TraceResult tr1;
		UTIL_TraceLine(vecSrc, vecSrc + vecAim * 512, dont_ignore_monsters, ENT(pev), &tr1);
		if (flDist > tr1.flFraction)
		{
			tr = tr1;
			flDist = tr.flFraction;
		}
	}

	// Couldn't find anything close enough
	if (flDist == 1.0)
		return;

	DecalGunshot(&tr, BULLET_PLAYER_CROWBAR);

	m_pBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.spr", 30);
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit(tr.vecEndPos, entindex());
	m_pBeam[m_iBeams]->SetEndAttachment(side < 0 ? 2 : 1);
	// m_pBeam[m_iBeams]->SetColor( 180, 255, 96 );
	m_pBeam[m_iBeams]->SetColor(17, 231, 255);
	m_pBeam[m_iBeams]->SetBrightness(64);
	m_pBeam[m_iBeams]->SetNoise(80);
	m_pBeam[m_iBeams]->pev->spawnflags |= SF_BEAM_TEMPORARY; // Flag these to be destroyed on save/restore or level transition
	m_iBeams++;
}


//=========================================================
// BeamGlow - brighten all beams
//=========================================================
void CSkrillex::BeamGlow()
{
	int b = m_iBeams * 32;
	if (b > 255)
		b = 255;

	for (int i = 0; i < m_iBeams; i++)
	{
		if (m_pBeam[i]->GetBrightness() != 255)
		{
			m_pBeam[i]->SetBrightness(b);
		}
	}
}


//=========================================================
// WackBeam - regenerate dead colleagues
//=========================================================
void CSkrillex::WackBeam(int side, CBaseEntity* pEntity)
{
	Vector vecDest;
	float flDist = 1.0;

	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	if (pEntity == NULL)
		return;

	m_pBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.spr", 30);
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit(pEntity->Center(), entindex());
	m_pBeam[m_iBeams]->SetEndAttachment(side < 0 ? 2 : 1);
	m_pBeam[m_iBeams]->SetColor(17, 231, 255);
	m_pBeam[m_iBeams]->SetBrightness(255);
	m_pBeam[m_iBeams]->SetNoise(80);
	m_pBeam[m_iBeams]->pev->spawnflags |= SF_BEAM_TEMPORARY; // Flag these to be destroyed on save/restore or level transition
	m_iBeams++;
}

//=========================================================
// ZapBeam - heavy damage directly forward
//=========================================================
void CSkrillex::ZapBeam(int side)
{
	Vector vecSrc, vecAim;
	TraceResult tr;
	CBaseEntity* pEntity;

	if (m_iBeams >= ISLAVE_MAX_BEAMS)
		return;

	vecSrc = pev->origin + gpGlobals->v_up * 36;
	vecAim = ShootAtEnemy(vecSrc);
	float deflection = 0.01;
	vecAim = vecAim + side * gpGlobals->v_right * RANDOM_FLOAT(0, deflection) + gpGlobals->v_up * RANDOM_FLOAT(-deflection, deflection);
	UTIL_TraceLine(vecSrc, vecSrc + vecAim * 1024, dont_ignore_monsters, ENT(pev), &tr);

	m_pBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.spr", 50);
	if (!m_pBeam[m_iBeams])
		return;

	m_pBeam[m_iBeams]->PointEntInit(tr.vecEndPos, entindex());
	m_pBeam[m_iBeams]->SetEndAttachment(side < 0 ? 2 : 1);
	m_pBeam[m_iBeams]->SetColor(17, 231, 255);
	m_pBeam[m_iBeams]->SetBrightness(255);
	m_pBeam[m_iBeams]->SetNoise(20);
	m_pBeam[m_iBeams]->pev->spawnflags |= SF_BEAM_TEMPORARY; // Flag these to be destroyed on save/restore or level transition
	m_iBeams++;

	pEntity = CBaseEntity::Instance(tr.pHit);
	if (pEntity != NULL && 0 != pEntity->pev->takedamage)
	{
		pEntity->TraceAttack(pev, gSkillData.skrillexDmgZap, vecAim, &tr, DMG_SHOCK);
	}
	UTIL_EmitAmbientSound(ENT(pev), tr.vecEndPos, "skrillex/fire.wav", 0.5, ATTN_NORM, 0, 100);
}


//=========================================================
// ClearBeams - remove all beams
//=========================================================
void CSkrillex::ClearBeams()
{
	for (int i = 0; i < ISLAVE_MAX_BEAMS; i++)
	{
		if (m_pBeam[i])
		{
			UTIL_Remove(m_pBeam[i]);
			m_pBeam[i] = NULL;
		}
	}
	m_iBeams = 0;
	pev->skin = 0;

	STOP_SOUND(ENT(pev), CHAN_WEAPON, "skrillex/charge.wav");
}
