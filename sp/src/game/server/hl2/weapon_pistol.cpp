//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Pistol - hand gun
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "ammodef.h"
#include "te_effect_dispatch.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	PISTOL_FASTEST_REFIRE_TIME		0.1f
#define	PISTOL_FASTEST_DRY_REFIRE_TIME	0.2f

#define	PISTOL_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
#define	PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out

ConVar	pistol_use_new_accuracy( "pistol_use_new_accuracy", "1" );

//-----------------------------------------------------------------------------
// CWeaponPistol
//-----------------------------------------------------------------------------

class CWeaponPistol : public CHLMachineGun
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CWeaponPistol, CHLMachineGun);

	CWeaponPistol(void);

	DECLARE_SERVERCLASS();

	void	Precache( void );
#ifndef EZ
	void	ItemPostFrame( void );
#else
	virtual void	ItemPostFrame( void );
#endif
	void	ItemPreFrame( void );
	void	ItemBusyFrame( void );
	void	PrimaryAttack( void );
	void	AddViewKick( void );
	void	DryFire( void );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	void	UpdatePenaltyTime( void );

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	Activity	GetPrimaryAttackActivity( void );

	virtual bool Reload( void );

	virtual const Vector& GetBulletSpread( void )
	{		
		// Handle NPCs first
		static Vector npcCone = VECTOR_CONE_5DEGREES;
		if ( GetOwner() && GetOwner()->IsNPC() )
			return npcCone;
			
		static Vector cone;

		if ( pistol_use_new_accuracy.GetBool() )
		{
			float ramp = RemapValClamped(	m_flAccuracyPenalty, 
											0.0f, 
											PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME, 
											0.0f, 
											1.0f ); 

			// We lerp from very accurate to inaccurate over time
			VectorLerp( VECTOR_CONE_1DEGREES, VECTOR_CONE_6DEGREES, ramp, cone );
		}
		else
		{
			// Old value
			cone = VECTOR_CONE_4DEGREES;
		}

		return cone;
	}
	
	virtual int	GetMinBurst() 
	{ 
		return 1; 
	}

	virtual int	GetMaxBurst() 
	{ 
		return 3; 
	}

	virtual float GetFireRate( void ) 
	{
		return 0.5f; 
	}

	DECLARE_ACTTABLE();

protected:
	float	m_flSoonestPrimaryAttack;
	float	m_flLastAttackTime;
	float	m_flAccuracyPenalty;
	int		m_nNumShotsFired;
};


IMPLEMENT_SERVERCLASS_ST(CWeaponPistol, DT_WeaponPistol)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_pistol, CWeaponPistol );
PRECACHE_WEAPON_REGISTER( weapon_pistol );

BEGIN_DATADESC( CWeaponPistol )

	DEFINE_FIELD( m_flSoonestPrimaryAttack, FIELD_TIME ),
	DEFINE_FIELD( m_flLastAttackTime,		FIELD_TIME ),
	DEFINE_FIELD( m_flAccuracyPenalty,		FIELD_FLOAT ), //NOTENOTE: This is NOT tracking game time
	DEFINE_FIELD( m_nNumShotsFired,			FIELD_INTEGER ),

END_DATADESC()

acttable_t	CWeaponPistol::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	false },
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_WALK,						ACT_WALK_PISTOL,				false },
	{ ACT_RUN,						ACT_RUN_PISTOL,					false },
};


IMPLEMENT_ACTTABLE( CWeaponPistol );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponPistol::CWeaponPistol( void )
{
	m_flSoonestPrimaryAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0.0f;

	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPistol::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponPistol::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_PISTOL_FIRE:
		{
			Vector vecShootOrigin, vecShootDir;
			vecShootOrigin = pOperator->Weapon_ShootPosition();

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );

			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );

			WeaponSound( SINGLE_NPC );
			pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );
			pOperator->DoMuzzleFlash();
			m_iClip1 = m_iClip1 - 1;
		}
		break;
		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponPistol::DryFire( void )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flSoonestPrimaryAttack	= gpGlobals->curtime + PISTOL_FASTEST_DRY_REFIRE_TIME;
	m_flNextPrimaryAttack		= gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponPistol::PrimaryAttack( void )
{
	if ( ( gpGlobals->curtime - m_flLastAttackTime ) > 0.5f )
	{
		m_nNumShotsFired = 0;
	}
	else
	{
		m_nNumShotsFired++;
	}

	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + PISTOL_FASTEST_REFIRE_TIME;
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, GetOwner() );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if( pOwner )
	{
		// Each time the player fires the pistol, reset the view punch. This prevents
		// the aim from 'drifting off' when the player fires very quickly. This may
		// not be the ideal way to achieve this, but it's cheap and it works, which is
		// great for a feature we're evaluating. (sjb)
		pOwner->ViewPunchReset();
	}

	BaseClass::PrimaryAttack();

	// Add an accuracy penalty which can move past our maximum penalty time if we're really spastic
	m_flAccuracyPenalty += PISTOL_ACCURACY_SHOT_PENALTY_TIME;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pOwner, true, GetClassname() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPistol::UpdatePenaltyTime( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	// Check our penalty time decay
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flAccuracyPenalty -= gpGlobals->frametime;
		m_flAccuracyPenalty = clamp( m_flAccuracyPenalty, 0.0f, PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPistol::ItemPreFrame( void )
{
	UpdatePenaltyTime();

	BaseClass::ItemPreFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPistol::ItemBusyFrame( void )
{
	UpdatePenaltyTime();

	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as button is pressed
//-----------------------------------------------------------------------------
void CWeaponPistol::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	if ( m_bInReload )
		return;
	
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	//Allow a refire as fast as the player can click
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}
	else if ( ( pOwner->m_nButtons & IN_ATTACK ) && ( m_flNextPrimaryAttack < gpGlobals->curtime ) && ( m_iClip1 <= 0 ) )
	{
		DryFire();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
Activity CWeaponPistol::GetPrimaryAttackActivity( void )
{
	if ( m_nNumShotsFired < 1 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nNumShotsFired < 2 )
		return ACT_VM_RECOIL1;

	if ( m_nNumShotsFired < 3 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponPistol::Reload( void )
{
	bool fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		WeaponSound( RELOAD );
		m_flAccuracyPenalty = 0.0f;
	}
	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPistol::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle	viewPunch;

	viewPunch.x = random->RandomFloat( 0.25f, 0.5f );
	viewPunch.y = random->RandomFloat( -.6f, .6f );
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch( viewPunch );
}

#ifdef EZ2
#define	PULSE_PISTOL_FASTEST_REFIRE_TIME		0.4f
#define	PULSE_PISTOL_FASTEST_DRY_REFIRE_TIME	1.5f
#define	PULSE_PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME	3.5f


//-----------------------------------------------------------------------------
// CWeaponPulsePistol, the famous melee replacement taken to a separate class.
//-----------------------------------------------------------------------------
class CWeaponPulsePistol : public CWeaponPistol
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponPulsePistol, CWeaponPistol);
	DECLARE_SERVERCLASS();

	void	Activate( void );
	void	Precache();
	void	ItemPostFrame( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	AddViewKick( void );
	void	DryFire( void );

	virtual bool Reload( void ) { return false; } // The pulse pistol does not reload

	virtual const Vector& GetBulletSpread( void )
	{
		// Handle NPCs first
		static Vector npcCone = VECTOR_CONE_5DEGREES;
		if (GetOwner() && GetOwner()->IsNPC())
			return npcCone;

		static Vector cone;

		if (pistol_use_new_accuracy.GetBool())
		{
			float ramp = RemapValClamped( m_flAccuracyPenalty,
				0.0f,
				PULSE_PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME,
				0.0f,
				1.0f );

			// We lerp from very accurate to inaccurate over time
			VectorLerp( VECTOR_CONE_1DEGREES, VECTOR_CONE_6DEGREES, ramp, cone );
		}
		else
		{
			// Old value
			cone = VECTOR_CONE_4DEGREES;
		}

		return cone;
	}

	virtual float GetFireRate( void )
	{
		return 3.0f;
	}

	virtual void	UpdateOnRemove( void );

	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	void	DoImpactEffect( trace_t &tr, int nDamageType );
	void	MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType );
	void	DoMuzzleFlash( void );

	void	StartChargeEffects( void );
	void	SetChargeEffectBrightness( float alpha );
	void	KillChargeEffects();

protected:
	CHandle<CSprite>	m_hChargeSprite;

private:
	// For recharging the ammo
	void			RechargeAmmo();
	int				m_nAmmoCount = 50;
	int				m_nChargeAttackAmmo = 0;
	float			m_flDrainRemainder;
	float			m_flChargeRemainder;
	float			m_flLastChargeTime;
	float			m_flLastChargeSoundTime;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::Activate( void )
{
	BaseClass::Activate();

	// Charge up and fire attack
	StartChargeEffects();
}

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::Precache( void )
{
	PrecacheModel( "effects/fluttercore.vmt" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::UpdateOnRemove( void )
{
	//Turn off charge glow
	KillChargeEffects();

	// Chain at end to mimic destructor unwind order
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: Recharge ammo before handling post-frame
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::ItemPostFrame( void )
{
	// Charge up and fire attack
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	// Player has released attack 2 and there is more than 1 ammo charged to fire
	if (!(pOwner->m_nButtons & IN_ATTACK2) && ( m_nChargeAttackAmmo > 0 ) && (m_flNextPrimaryAttack <= gpGlobals->curtime) )
	{
		// Reset the next attack to the current time to avoid multiple shots queuing up
		m_flNextPrimaryAttack = gpGlobals->curtime;

		// Fire
		PrimaryAttack();

		// Turn off sprite
		SetChargeEffectBrightness( 0.0f );

		return;
	}
	else if ((pOwner->m_nButtons & IN_ATTACK2) )
	{
		// Charge the next shot
		SecondaryAttack();
		return;
	}
	// If the charge up and fire wasn't either charged or shot this frame, reset so the gun doesn't go off unexpectedly
	m_nChargeAttackAmmo = 0;

	RechargeAmmo();

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: The pulse pistol has a longer dryfire refire time than the 9mm
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::DryFire( void )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );

	m_flSoonestPrimaryAttack	= gpGlobals->curtime + PULSE_PISTOL_FASTEST_DRY_REFIRE_TIME;
	m_flNextPrimaryAttack		= gpGlobals->curtime + SequenceDuration();
}


//-----------------------------------------------------------------------------
// Purpose: Recharge ammo before handling weapon deploy
//-----------------------------------------------------------------------------
bool CWeaponPulsePistol::Deploy( void )
{
	RechargeAmmo();
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPulsePistol::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if (m_nChargeAttackAmmo > 0)
	{
		// If there is any charge, play a click
		// Mostly just to cancel the current charge sound
		WeaponSound( EMPTY );
	}

	// Holstering resets charge
	m_nChargeAttackAmmo = 0;
	SetChargeEffectBrightness( 0.0f );

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: Replenish ammo based on time elapsed since last charge
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::RechargeAmmo( void )
{
	// If there is a fully charged shot waiting, don't recharge
	if (m_nChargeAttackAmmo >= 40)
	{
		return;
	}

	// 1upD - how long as the pistol been charging?
	float flChargeInterval = gpGlobals->curtime - m_flLastChargeTime;
	m_flLastChargeTime = gpGlobals->curtime;

	// This code is inherited from the airboat. I've changed it to work for the pistol.
	int nMaxAmmo = 50;
	if (m_iClip1 == nMaxAmmo)
	{
		return;
	}

	float flRechargeRate = 5;
	float flChargeAmount = flRechargeRate * flChargeInterval;
	if (m_flDrainRemainder != 0.0f)
	{
		if (m_flDrainRemainder >= flChargeAmount)
		{
			m_flDrainRemainder -= flChargeAmount;
			return;
		}
		else
		{
			flChargeAmount -= m_flDrainRemainder;
			m_flDrainRemainder = 0.0f;
		}
	}

	m_flChargeRemainder += flChargeAmount;
	int nAmmoToAdd = (int)m_flChargeRemainder;
	m_flChargeRemainder -= nAmmoToAdd;
	m_iClip1 += nAmmoToAdd;
	if (m_iClip1 > nMaxAmmo)
	{
		m_iClip1 = nMaxAmmo;
		m_flChargeRemainder = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: BREADMAN - look in fx_hl2_tracers.cpp
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
	float flTracerDist;
	Vector vecDir;
	Vector vecEndPos;

	vecDir = tr.endpos - vecTracerSrc;

	flTracerDist = VectorNormalize( vecDir );

	UTIL_Tracer( vecTracerSrc, tr.endpos, 1, TRACER_FLAG_USEATTACHMENT, 5000, true, "GaussTracer" );
}

//-----------------------------------------------------------------------------
// Purpose: all BREADMAN
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::PrimaryAttack( void )
{
	if (m_iClip1 < 10) // Don't allow firing if we only have one shot left
	{
		DryFire();
	}
	else
	{
		// Only the player fires this way so we can cast
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
		if (!pPlayer)
			return;

		// Abort here to handle burst and auto fire modes
		if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount( m_iPrimaryAmmoType )))
			return;

		m_nNumShotsFired++;

		pPlayer->DoMuzzleFlash();

		// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
		// especially if the weapon we're firing has a really fast rate of fire.
		int iBulletsToFire = 0;
		float fireRate = GetFireRate();

		// MUST call sound before removing a round from the clip of a CHLMachineGun
		while (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
			// Pulse pistol fires two bullets
			iBulletsToFire += 2;
			// The pulse pistol fires one extra round per ten ammo charged
			iBulletsToFire += m_nChargeAttackAmmo / 10;

			// If the shot is charged, play a special sound
			if ( iBulletsToFire >= 4 )
			{
				// 'Burst' will be used for the charge up sound
				WeaponSound( BURST, m_flNextPrimaryAttack );
			}
			else
			{
				WeaponSound( SINGLE, m_flNextPrimaryAttack );
			}
		}

		// Subtract the charge
		m_iClip1 = m_iClip1 - 10;
		m_nChargeAttackAmmo = 0;

		m_iPrimaryAttacks++;
		gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );

		//int	bulletType = GetAmmoDef()->Index("AR2");

		// Fire the bullets
		FireBulletsInfo_t info;
		info.m_iShots = iBulletsToFire;
		info.m_vecSrc = pPlayer->Weapon_ShootPosition();
		info.m_vecDirShooting = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
		//info.m_vecSpread = pPlayer->GetAttackSpread(this);
		info.m_vecSpread = VECTOR_CONE_1DEGREES;
		info.m_flDistance = MAX_TRACE_LENGTH;
		info.m_iAmmoType = m_iPrimaryAmmoType;
		info.m_iTracerFreq = 1;
		FireBullets( info );

		//Factor in the view kick
		AddViewKick();

		DoMuzzleFlash();

		CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pPlayer );

		if (!m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0)
		{
			// HEV suit - indicate out of ammo condition
			pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
		}

		SendWeaponAnim( GetPrimaryAttackActivity() );
		pPlayer->SetAnimation( PLAYER_ATTACK1 );

		// Register a muzzleflash for the AI
		pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

		m_iPrimaryAttacks++;
		gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );

		m_flSoonestPrimaryAttack = gpGlobals->curtime + PULSE_PISTOL_FASTEST_REFIRE_TIME;
		m_flNextPrimaryAttack = gpGlobals->curtime + PULSE_PISTOL_FASTEST_REFIRE_TIME;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Charge shot with ammo
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::SecondaryAttack( void )
{
	int nMaxCharge = 40;
	// If there is only one shot left or the charge has reached maximum, do not charge!
	if (m_iClip1 <= 10 || m_nChargeAttackAmmo == nMaxCharge)
	{
		RechargeAmmo();
		return;
	}

	// Play sound
	if (gpGlobals->curtime - m_flLastChargeSoundTime > 4.0f && m_nChargeAttackAmmo == 0)
	{
		WeaponSound( SPECIAL1 );
		m_flLastChargeSoundTime = gpGlobals->curtime;
	}

	float flChargeInterval = gpGlobals->curtime - m_flLastChargeTime;
	m_flLastChargeTime = gpGlobals->curtime;

	// The charge attack charges twice as quickly as ammo recharges
	float flRechargeRate = 10;
	float flChargeAmount = flRechargeRate * flChargeInterval;

	m_flChargeRemainder += flChargeAmount;
	int nAmmoToAdd = (int)m_flChargeRemainder;
	m_flChargeRemainder -= nAmmoToAdd;
	m_nChargeAttackAmmo += nAmmoToAdd;
	m_iClip1 -= nAmmoToAdd;
	if (m_nChargeAttackAmmo > nMaxCharge)
	{
		m_nChargeAttackAmmo = nMaxCharge;
		m_flChargeRemainder = 0.0f;
	}

	// Display the charge sprite
	StartChargeEffects();
	SetChargeEffectBrightness( m_nChargeAttackAmmo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );

	if (pPlayer == NULL)
		return;

	QAngle	viewPunch;

	viewPunch.x = random->RandomFloat( -4.5f, -4.5f );
	viewPunch.y = random->RandomFloat( -.13f, .13f );
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch( viewPunch );
}

//-----------------------------------------------------------------------	------
// Purpose: BREADMAN
// Input  : &tr - 
//			nDamageType - 
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::DoImpactEffect( trace_t &tr, int nDamageType )
{
	CEffectData data;

	data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
	data.m_vNormal = tr.plane.normal;

	DispatchEffect( "AR2Impact", data );

	BaseClass::DoImpactEffect( tr, nDamageType );
}

//-----------------------------------------------------------------------------
// Purpose: Particle muzzle flash for pulse pistol
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::DoMuzzleFlash( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if (pOwner == NULL)
		return;

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	if (pViewModel == NULL)
		return;

	CEffectData data;
	data.m_nEntIndex = pViewModel->entindex();
	data.m_nAttachmentIndex = 1;
	data.m_flScale = 0.25f; // The pistol is much smaller than a Hunter-Chopper ( ._.)
	DispatchEffect( "ChopperMuzzleFlash", data );

	BaseClass::DoMuzzleFlash();
}

//-----------------------------------------------------------------------------
// Charge up effect methods
//
// I'm content with but not super enthusiastic about how these effects look
// right now. Perhaps in a later phase of development we can improve the look
// and sound of the pulse pistol alt-fire, but this is sufficent for alpha
// testing at the very least.
//
// 1upD
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Create effects for pulse pistol alt-fire
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::StartChargeEffects()
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	//Create the charge glow
	if (pOwner != NULL && m_hChargeSprite == NULL)
	{
		m_hChargeSprite = CSprite::SpriteCreate( "effects/fluttercore.vmt", GetAbsOrigin(), false );

		m_hChargeSprite->SetAsTemporary();
		m_hChargeSprite->SetAttachment( pOwner->GetViewModel(), 1 );
		m_hChargeSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone );
		m_hChargeSprite->SetBrightness( 0, 0.1f );
		m_hChargeSprite->SetScale( 0.05f, 0.05f );
		m_hChargeSprite->TurnOn();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Change the brightness of the alt-fire effects
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::SetChargeEffectBrightness( float alpha )
{
	if (m_hChargeSprite != NULL)
	{
		m_hChargeSprite->SetBrightness( m_nChargeAttackAmmo, 0.1f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove pulse pistol alt-fire effects
//-----------------------------------------------------------------------------
void CWeaponPulsePistol::KillChargeEffects()
{
	if (m_hChargeSprite != NULL)
	{
		UTIL_Remove( m_hChargeSprite );
		m_hChargeSprite = NULL;
	}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST(CWeaponPulsePistol, DT_WeaponPulsePistol)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_pulsepistol, CWeaponPulsePistol );
PRECACHE_WEAPON_REGISTER( weapon_pulsepistol );

BEGIN_DATADESC( CWeaponPulsePistol )
	DEFINE_FIELD( m_flLastChargeTime, FIELD_TIME ),
	DEFINE_FIELD( m_flLastChargeSoundTime, FIELD_TIME ),
	DEFINE_FIELD( m_hChargeSprite, FIELD_EHANDLE )
END_DATADESC()
#endif
