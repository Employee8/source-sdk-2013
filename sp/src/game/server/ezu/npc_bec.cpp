//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_node.h"
#include "ai_hull.h"
#include "ai_hint.h"
#include "ai_squad.h"
#include "ai_senses.h"
#include "ai_navigator.h"
#include "ai_motor.h"
#include "ai_behavior.h"
#include "ai_baseactor.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_follow.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_assault.h"
#include "npc_playercompanion.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "activitylist.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "sceneentity.h"
#include "ai_behavior_functank.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BEC_MODEL "models/police.mdl"
#define BEC_SKIN 3

// Bec's ConVar stuff
ConVar	sk_bec_health( "sk_bec_health","0");

// Proficiency ConVars
ConVar  sk_bec_ar2_proficiency("sk_bec_ar2_proficiency", "2"); // (from npc_citizen17.cpp) Added by 1upD. Skill rating 0 - 4 of how accurate the AR2 should be
ConVar  sk_bec_default_proficiency("sk_bec_default_proficiency", "1"); // (from npc_citizen17.cpp) Added by 1upD. Skill rating 0 - 4 of how accurate all weapons but the AR2 should be

// Damage adjusters
ConVar	sk_bec_head_dmg("sk_bec_head_dmg", "1");
ConVar	sk_bec_chest_dmg("sk_bec_chest_dmg", "1");
ConVar	sk_bec_stomach_dmg("sk_bec_stomach_dmg", "1");
ConVar	sk_bec_arm_dmg("sk_bec_arm_dmg", "1");
ConVar	sk_bec_leg_dmg("sk_bec_leg_dmg", "1");

//=========================================================
// Bec activities
//=========================================================

class CNPC_Bec : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS( CNPC_Bec, CNPC_PlayerCompanion );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache()
	{
		// Prevents a warning
		SelectModel( );
		BaseClass::Precache();

		PrecacheScriptSound( "NPC_MetroPolice.FootstepLeft" );
		PrecacheScriptSound( "NPC_MetroPolice.FootstepRight" );
		PrecacheScriptSound( "NPC_MetroPolice.Die" );
	}

	void	Spawn( void );
	void	SelectModel();
	Class_T Classify( void );
	void	Weapon_Equip( CBaseCombatWeapon *pWeapon );

	bool CreateBehaviors( void );

	void HandleAnimEvent( animevent_t *pEvent );

	virtual float	GetHitgroupDamageMultiplier(int iHitGroup, const CTakeDamageInfo& info);

	virtual bool	ShouldRegenerateHealth(void);

	WeaponProficiency_t CalcWeaponProficiency(CBaseCombatWeapon* pWeapon); // Ported from Citizens, added to them by 1upD

	bool ShouldLookForBetterWeapon() { return false; }

	void OnChangeRunningBehavior( CAI_BehaviorBase *pOldBehavior,  CAI_BehaviorBase *pNewBehavior );

	void DeathSound( const CTakeDamageInfo &info );
	void GatherConditions();
	void UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	CAI_FuncTankBehavior		m_FuncTankBehavior;
	COutputEvent				m_OnPlayerUse;

	DEFINE_CUSTOM_AI;
};


LINK_ENTITY_TO_CLASS( npc_bec, CNPC_Bec );

//---------------------------------------------------------
// 
//---------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST(CNPC_Bec, DT_NPC_Bec)
END_SEND_TABLE()


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Bec )
//						m_FuncTankBehavior
	DEFINE_OUTPUT( m_OnPlayerUse, "OnPlayerUse" ),
	DEFINE_USEFUNC( UseFunc ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bec::SelectModel()
{
	SetModelName( AllocPooledString( BEC_MODEL ) );
}

//------------------------------------------------------------------------------
// Added by 1upD. Citizen weapon proficiency code
// Mark: Ported to Bec to make him have better accuracy than the average NPC
//------------------------------------------------------------------------------
WeaponProficiency_t CNPC_Bec::CalcWeaponProficiency(CBaseCombatWeapon* pWeapon)
{
	int proficiency;

	if (FClassnameIs(pWeapon, "weapon_ar2"))
	{
		proficiency = sk_bec_ar2_proficiency.GetInt();
	}
	else {
		proficiency = sk_bec_default_proficiency.GetInt();
	}

	// Clamp the upper range of the ConVar to "perfect"
	// Could also cast the int to the enum if you're feeling dangerous
	if (proficiency > 4) {
		return WEAPON_PROFICIENCY_PERFECT;
	}

	// Switch values to enum. You could cast the int to the enum here,
	// but I'm not sure how 'safe' that is since the enum values are
	// not explicity defined. This also clamps the range.
	switch (proficiency) {
	case 4:
		return WEAPON_PROFICIENCY_PERFECT;
	case 3:
		return WEAPON_PROFICIENCY_VERY_GOOD;
	case 2:
		return WEAPON_PROFICIENCY_GOOD;
	case 1:
		return WEAPON_PROFICIENCY_AVERAGE;
	default:
		return WEAPON_PROFICIENCY_POOR;
	}
}

//---------------------------------------------------------
// Return the number by which to multiply incoming damage
// based on the hitgroup it hits. This exists mainly so
// that individual NPC's can have more or less resistance
// to damage done to certain hitgroups.
// 
// Mark: Code ported so Bec receives less damage from
// getting shot cause yknow he's important and shouldn't
// die instantly.
//---------------------------------------------------------
float CNPC_Bec::GetHitgroupDamageMultiplier(int iHitGroup, const CTakeDamageInfo& info)
{
	switch (iHitGroup)
	{
	case HITGROUP_GENERIC:
		return 1.0f;

	case HITGROUP_HEAD:
		return sk_bec_head_dmg.GetFloat();

	case HITGROUP_CHEST:
		return sk_bec_chest_dmg.GetFloat();

	case HITGROUP_STOMACH:
		return sk_bec_stomach_dmg.GetFloat();

	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
		return sk_bec_arm_dmg.GetFloat();

	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		return sk_bec_leg_dmg.GetFloat();

	default:
		return 1.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bec::Spawn(void)
{
	Precache();

	m_iHealth = 80;

	m_nSkin = (BEC_SKIN);

	BaseClass::Spawn();

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );

	//----------------------------------------------------- 
	// Mark Zer0: Bec respects Social Distancing :^)
	//-----------------------------------------------------
	if (m_spawnEquipment != NULL_STRING)
		CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1);

	NPCInit();

	SetUse( &CNPC_Bec::UseFunc );
}

//-----------------------------------------------------------------------------
// 1upD	- Override PlayerAlly's ShouldRegenerateHealth() to check spawnflag
// (from npc_combine.cpp)
// 
// Mark: Ported to Bec so he regenerates. 
// By default he can go down pretty easily, since he's an important NPC
// the regeneration helps so the player doesn't have to babysit him.
//-----------------------------------------------------------------------------
bool CNPC_Bec::ShouldRegenerateHealth(void)
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Bec::Classify( void )
{
	return	CLASS_METROPOLICE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Bec::Weapon_Equip( CBaseCombatWeapon *pWeapon )
{
	BaseClass::Weapon_Equip( pWeapon );

		// Mark: Code that allows Bec to defend himself at point blank
		// inherited from Barney
		pWeapon->m_fMinRange1 = 0.0f;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Bec::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
	case NPC_EVENT_LEFTFOOT:
		{
			EmitSound( "NPC_MetroPolice.FootstepLeft", pEvent->eventtime );
		}
		break;
	case NPC_EVENT_RIGHTFOOT:
		{
			EmitSound( "NPC_MetroPolice.FootstepRight", pEvent->eventtime );
		}
		break;

	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CNPC_Bec::DeathSound( const CTakeDamageInfo &info )
{
	// Sentences don't play on dead NPCs
	SentenceStop();

	SpeakIfAllowed( TLK_DEATH );

}

bool CNPC_Bec::CreateBehaviors( void )
{
	BaseClass::CreateBehaviors();
	AddBehavior( &m_FuncTankBehavior );

	return true;
}

void CNPC_Bec::OnChangeRunningBehavior( CAI_BehaviorBase *pOldBehavior,  CAI_BehaviorBase *pNewBehavior )
{
	if ( pNewBehavior == &m_FuncTankBehavior )
	{
		m_bReadinessCapable = false;
	}
	else if ( pOldBehavior == &m_FuncTankBehavior )
	{
		m_bReadinessCapable = IsReadinessCapable();
	}

	BaseClass::OnChangeRunningBehavior( pOldBehavior, pNewBehavior );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Bec::GatherConditions()
{
	BaseClass::GatherConditions();

	// Handle speech AI. Don't do AI speech if we're in scripts unless permitted by the EnableSpeakWhileScripting input.
	if ( m_NPCState == NPC_STATE_IDLE || m_NPCState == NPC_STATE_ALERT || m_NPCState == NPC_STATE_COMBAT ||
		( ( m_NPCState == NPC_STATE_SCRIPT ) && CanSpeakWhileScripting() ) )
	{
		DoCustomSpeechAI();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Bec::UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_bDontUseSemaphore = true;
	SpeakIfAllowed( TLK_USE );
	m_bDontUseSemaphore = false;

	m_OnPlayerUse.FireOutput( pActivator, pCaller );
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_bec, CNPC_Bec )

AI_END_CUSTOM_NPC()
