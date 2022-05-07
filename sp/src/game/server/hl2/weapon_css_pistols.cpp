//=============================================================================//
//
// Purpose: CS:S weapons recreated from scratch in Source SDK 2013 for usage in a Half-Life 2 setting.
//
// Author: Blixibon
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "weapon_css_base.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar pistol_use_new_accuracy;

//-----------------------------------------------------------------------------
// CWeapon_CSS_HL2_Glock18
//-----------------------------------------------------------------------------
class CWeapon_CSS_HL2_Glock18 : public CBase_CSS_HL2_Pistol
{
public:
	DECLARE_CLASS( CWeapon_CSS_HL2_Glock18, CBase_CSS_HL2_Pistol );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeapon_CSS_HL2_Glock18(void);

	bool	Deploy( void );

	void	PrimaryAttack();
	void	SecondaryAttack();

	void	BurstThink( void );

	Activity	GetPrimaryAttackActivity( void );

	virtual const Vector& GetBulletSpread( void )
	{		
		// Handle NPCs first
		static Vector npcCone = VECTOR_CONE_5DEGREES;
		if ( GetOwner() && GetOwner()->IsNPC() )
			return npcCone;

		static Vector cone;

		if ( pistol_use_new_accuracy.GetBool() )
		{
			float ramp = RemapValClamped(	GetAccuracyPenalty(), 
											0.0f, 
											GetMaxShotPenalty(), 
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

	virtual float	GetBurstCycleRate( void ) { return 1.0f; }
	virtual int		GetBurstSize( void ) { return 3; };
	
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	virtual float GetFireRate( void ) { return m_bInBurst ? 0.025f : 0.5f; }
	virtual float GetRefireRate( void ) { return 0.15f;	}
	virtual float GetDryRefireRate( void ) { return 0.2f; }

	virtual float GetShotPenaltyTime() { return 0.2f; }
	virtual float GetMaxShotPenalty() { return 1.5f; }

private:
	bool m_bInBurstMode;
	bool m_bInBurst;
	int m_iBurstSize;
};

IMPLEMENT_SERVERCLASS_ST( CWeapon_CSS_HL2_Glock18, DT_Weapon_CSS_HL2_Glock18 )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_css_glock, CWeapon_CSS_HL2_Glock18 );
PRECACHE_WEAPON_REGISTER( weapon_css_glock );

BEGIN_DATADESC( CWeapon_CSS_HL2_Glock18 )

	DEFINE_FIELD( m_bInBurstMode, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bInBurst, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iBurstSize, FIELD_INTEGER ),
	
	// Function pinters
	DEFINE_FUNCTION( BurstThink ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon_CSS_HL2_Glock18::CWeapon_CSS_HL2_Glock18( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeapon_CSS_HL2_Glock18::Deploy( void )
{
	// Forget about any bursts this weapon was firing when holstered
	m_iBurstSize = 0;
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_CSS_HL2_Glock18::PrimaryAttack( void )
{
	if (m_bFireOnEmpty)
	{
		return;
	}

	if (m_bInBurstMode)
	{
		m_bInBurst = true;
		m_flNextPrimaryAttack = gpGlobals->curtime - ((float)(GetBurstSize()-1) * GetFireRate());
		BaseClass::PrimaryAttack();
		m_bInBurst = false;
		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();

		/*
		m_iBurstSize = GetBurstSize();

		// Call the think function directly so that the first round gets fired immediately.
		BurstThink();
		SetThink( &CWeapon_CSS_HL2_Glock18::BurstThink );
		m_flNextPrimaryAttack = gpGlobals->curtime + GetBurstCycleRate();
		m_flNextSecondaryAttack = gpGlobals->curtime + GetBurstCycleRate();

		// Pick up the rest of the burst through the think function.
		SetNextThink( gpGlobals->curtime + GetFireRate() );
		*/
	}
	else
	{
		BaseClass::PrimaryAttack();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_CSS_HL2_Glock18::SecondaryAttack( void )
{
	if (m_bInBurstMode)
	{
		// Burst off
		m_bInBurstMode = false;
		WeaponSound( SPECIAL1 );
	}
	else
	{
		// Burst on
		m_bInBurstMode = true;
		WeaponSound( SPECIAL2 );
	}

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f; // TODO: Real cooldown?
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeapon_CSS_HL2_Glock18::BurstThink( void )
{
	//m_flNextPrimaryAttack = gpGlobals->curtime; // HACK?
	BaseClass::PrimaryAttack();

	m_iBurstSize--;

	if( m_iBurstSize == 0 )
	{
		// The burst is over!
		SetThink(NULL);

		// idle immediately to stop the firing animation
		//SetWeaponIdleTime( gpGlobals->curtime );
		return;
	}

	SetNextThink( gpGlobals->curtime + GetFireRate() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
Activity CWeapon_CSS_HL2_Glock18::GetPrimaryAttackActivity( void )
{
	if (m_bInBurstMode)
		return ACT_VM_SECONDARYATTACK;

	return ACT_VM_PRIMARYATTACK;
}

//-----------------------------------------------------------------------------
// CWeapon_CSS_HL2_USP
//-----------------------------------------------------------------------------
class CWeapon_CSS_HL2_USP : public CBase_CSS_HL2_Pistol
{
public:
	DECLARE_CLASS( CWeapon_CSS_HL2_USP, CBase_CSS_HL2_Pistol );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeapon_CSS_HL2_USP(void);

	virtual const Vector& GetBulletSpread( void )
	{		
		// Handle NPCs first
		static Vector npcCone = VECTOR_CONE_5DEGREES;
		if ( GetOwner() && GetOwner()->IsNPC() )
			return npcCone;

		static Vector cone;

		if ( pistol_use_new_accuracy.GetBool() )
		{
			float ramp = RemapValClamped(	GetAccuracyPenalty(), 
											0.0f, 
											GetMaxShotPenalty(), 
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
	
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	virtual float GetFireRate( void ) { return 0.5f; }
	virtual float GetRefireRate( void ) { return 0.15f;	}
	virtual float GetDryRefireRate( void ) { return 0.2f; }

	virtual float GetShotPenaltyTime() { return 0.2f; }
	virtual float GetMaxShotPenalty() { return 1.5f; }

	// Silencer damage adjustment
	// Player damage: 8 -> 6.4 (6)
	// NPC damage: 3 -> 3
	virtual float GetDamageMultiplier() const { return IsSilenced() ? 0.8f : 1.0f; }
	virtual float GetNPCDamageMultiplier() const { return 1.0f; }
};

IMPLEMENT_SERVERCLASS_ST( CWeapon_CSS_HL2_USP, DT_Weapon_CSS_HL2_USP )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_css_usp, CWeapon_CSS_HL2_USP );
PRECACHE_WEAPON_REGISTER( weapon_css_usp );

BEGIN_DATADESC( CWeapon_CSS_HL2_USP )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon_CSS_HL2_USP::CWeapon_CSS_HL2_USP( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;

	m_bCanToggleSilencer = true;
}

//-----------------------------------------------------------------------------
// CWeapon_CSS_HL2_P228
//-----------------------------------------------------------------------------
class CWeapon_CSS_HL2_P228 : public CBase_CSS_HL2_Pistol
{
public:
	DECLARE_CLASS( CWeapon_CSS_HL2_P228, CBase_CSS_HL2_Pistol );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeapon_CSS_HL2_P228(void);

	virtual const Vector& GetBulletSpread( void )
	{		
		// Handle NPCs first
		static Vector npcCone = VECTOR_CONE_5DEGREES;
		if ( GetOwner() && GetOwner()->IsNPC() )
			return npcCone;

		static Vector cone;

		if ( pistol_use_new_accuracy.GetBool() )
		{
			float ramp = RemapValClamped(	GetAccuracyPenalty(), 
											0.0f, 
											GetMaxShotPenalty(), 
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
	
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	virtual float GetFireRate( void ) { return 0.5f; }
	virtual float GetRefireRate( void ) { return 0.15f;	}
	virtual float GetDryRefireRate( void ) { return 0.2f; }

	virtual float GetShotPenaltyTime() { return 0.3f; }
	virtual float GetMaxShotPenalty() { return 1.5f; }
};

IMPLEMENT_SERVERCLASS_ST( CWeapon_CSS_HL2_P228, DT_Weapon_CSS_HL2_P228 )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_css_p228, CWeapon_CSS_HL2_P228 );
PRECACHE_WEAPON_REGISTER( weapon_css_p228 );

BEGIN_DATADESC( CWeapon_CSS_HL2_P228 )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon_CSS_HL2_P228::CWeapon_CSS_HL2_P228( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// CWeapon_CSS_HL2_Deagle
//-----------------------------------------------------------------------------
class CWeapon_CSS_HL2_Deagle : public CBase_CSS_HL2_Pistol
{
public:
	DECLARE_CLASS( CWeapon_CSS_HL2_Deagle, CBase_CSS_HL2_Pistol );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeapon_CSS_HL2_Deagle(void);

	virtual const Vector& GetBulletSpread( void )
	{		
		// Handle NPCs first
		static Vector npcCone = VECTOR_CONE_5DEGREES;
		if ( GetOwner() && GetOwner()->IsNPC() )
			return npcCone;

		static Vector cone;

		if ( pistol_use_new_accuracy.GetBool() )
		{
			float ramp = RemapValClamped(	GetAccuracyPenalty(), 
											0.0f, 
											GetMaxShotPenalty(), 
											0.0f, 
											1.0f ); 

			// We lerp from very accurate to inaccurate over time
			VectorLerp( VECTOR_CONE_1DEGREES, VECTOR_CONE_10DEGREES, ramp, cone );
		}
		else
		{
			// Old value
			cone = VECTOR_CONE_4DEGREES;
		}

		return cone;
	}
	
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	virtual float GetFireRate( void ) { return 0.5f; }
	virtual float GetRefireRate( void ) { return 0.225f; }
	virtual float GetDryRefireRate( void ) { return 0.25f; }

	virtual float GetShotPenaltyTime() { return 0.3f; }
	virtual float GetMaxShotPenalty() { return 1.5f; }
};

IMPLEMENT_SERVERCLASS_ST( CWeapon_CSS_HL2_Deagle, DT_Weapon_CSS_HL2_Deagle )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_css_deagle, CWeapon_CSS_HL2_Deagle );
PRECACHE_WEAPON_REGISTER( weapon_css_deagle );

BEGIN_DATADESC( CWeapon_CSS_HL2_Deagle )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon_CSS_HL2_Deagle::CWeapon_CSS_HL2_Deagle( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// CWeapon_CSS_HL2_FiveSeveN
//-----------------------------------------------------------------------------
class CWeapon_CSS_HL2_FiveSeveN : public CBase_CSS_HL2_Pistol
{
public:
	DECLARE_CLASS( CWeapon_CSS_HL2_FiveSeveN, CBase_CSS_HL2_Pistol );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeapon_CSS_HL2_FiveSeveN(void);

	virtual const Vector& GetBulletSpread( void )
	{		
		// Handle NPCs first
		static Vector npcCone = VECTOR_CONE_3DEGREES;
		if ( GetOwner() && GetOwner()->IsNPC() )
			return npcCone;

		static Vector cone;

		if ( pistol_use_new_accuracy.GetBool() )
		{
			float ramp = RemapValClamped(	GetAccuracyPenalty(), 
											0.0f, 
											GetMaxShotPenalty(), 
											0.0f, 
											1.0f ); 

			// We lerp from very accurate to inaccurate over time
			VectorLerp( VECTOR_CONE_1DEGREES, VECTOR_CONE_5DEGREES, ramp, cone );
		}
		else
		{
			// Old value
			cone = VECTOR_CONE_3DEGREES;
		}

		return cone;
	}
	
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	virtual float GetFireRate( void ) { return 0.5f; }
	virtual float GetRefireRate( void ) { return 0.15f; }
	virtual float GetDryRefireRate( void ) { return 0.25f; }

	virtual float GetShotPenaltyTime() { return 0.1f; }
	virtual float GetMaxShotPenalty() { return 1.5f; }
};

IMPLEMENT_SERVERCLASS_ST( CWeapon_CSS_HL2_FiveSeveN, DT_Weapon_CSS_HL2_FiveSeveN )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_css_fiveseven, CWeapon_CSS_HL2_FiveSeveN );
PRECACHE_WEAPON_REGISTER( weapon_css_fiveseven );

BEGIN_DATADESC( CWeapon_CSS_HL2_FiveSeveN )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon_CSS_HL2_FiveSeveN::CWeapon_CSS_HL2_FiveSeveN( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// CWeapon_CSS_HL2_DualBerettas
//-----------------------------------------------------------------------------
class CWeapon_CSS_HL2_DualBerettas : public CBase_CSS_HL2_Pistol
{
public:
	DECLARE_CLASS( CWeapon_CSS_HL2_DualBerettas, CBase_CSS_HL2_Pistol );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeapon_CSS_HL2_DualBerettas(void);

	void	PrimaryAttack();
	Activity	GetPrimaryAttackActivity( void );

	virtual const Vector& GetBulletSpread( void )
	{		
		// Handle NPCs first
		static Vector npcCone = VECTOR_CONE_5DEGREES;
		if ( GetOwner() && GetOwner()->IsNPC() )
			return npcCone;

		static Vector cone;

		if ( pistol_use_new_accuracy.GetBool() )
		{
			float ramp = RemapValClamped(	GetAccuracyPenalty(), 
											0.0f, 
											GetMaxShotPenalty(), 
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
	
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	virtual float GetFireRate( void ) { return 0.5f; }
	virtual float GetRefireRate( void ) { return 0.12f; }
	virtual float GetDryRefireRate( void ) { return 0.25f; }

	virtual float GetShotPenaltyTime() { return 0.3f; }
	virtual float GetMaxShotPenalty() { return 1.5f; }

	// Tries to replicate CS:S's boosted damage without going crazy
	// Player damage: 5 -> 7
	// NPC damage: 3 -> 3
	virtual float GetDamageMultiplier() const { return 1.4f; }
	virtual float GetNPCDamageMultiplier() const { return 1.0f; }

private:
	bool m_bGunMode;
};

IMPLEMENT_SERVERCLASS_ST( CWeapon_CSS_HL2_DualBerettas, DT_Weapon_CSS_HL2_DualBerettas )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_css_elite, CWeapon_CSS_HL2_DualBerettas );
PRECACHE_WEAPON_REGISTER( weapon_css_elite );

BEGIN_DATADESC( CWeapon_CSS_HL2_DualBerettas )

	DEFINE_FIELD( m_bGunMode, FIELD_BOOLEAN ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon_CSS_HL2_DualBerettas::CWeapon_CSS_HL2_DualBerettas( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapon_CSS_HL2_DualBerettas::PrimaryAttack( void )
{
	BaseClass::PrimaryAttack();
	m_bGunMode = !m_bGunMode;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
Activity CWeapon_CSS_HL2_DualBerettas::GetPrimaryAttackActivity( void )
{
	if (m_bGunMode)
		return ACT_VM_SECONDARYATTACK;

	return ACT_VM_PRIMARYATTACK;
}
