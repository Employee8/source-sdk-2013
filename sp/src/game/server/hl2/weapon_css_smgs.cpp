//=============================================================================//
//
// Purpose: CS:S weapons recreated from scratch in Source SDK 2013 for usage in a Half-Life 2 setting.
//
// Author: Blixibon
//
//=============================================================================//

#include "cbase.h"
#include "weapon_css_base.h"

//-----------------------------------------------------------------------------
// CWeapon_CSS_HL2_TMP
//-----------------------------------------------------------------------------
class CWeapon_CSS_HL2_TMP : public CBase_CSS_HL2_SMG
{
public:
	DECLARE_CLASS( CWeapon_CSS_HL2_TMP, CBase_CSS_HL2_SMG );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeapon_CSS_HL2_TMP(void);
	
	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = VECTOR_CONE_5DEGREES;
		return cone;
	}
	
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	virtual float GetFireRate( void ) { return 0.07f; }
};

IMPLEMENT_SERVERCLASS_ST( CWeapon_CSS_HL2_TMP, DT_Weapon_CSS_HL2_TMP )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_css_tmp, CWeapon_CSS_HL2_TMP );
PRECACHE_WEAPON_REGISTER( weapon_css_tmp );

BEGIN_DATADESC( CWeapon_CSS_HL2_TMP )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon_CSS_HL2_TMP::CWeapon_CSS_HL2_TMP( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// CWeapon_CSS_HL2_MP5Navy
//-----------------------------------------------------------------------------
class CWeapon_CSS_HL2_MP5Navy : public CBase_CSS_HL2_SMG
{
public:
	DECLARE_CLASS( CWeapon_CSS_HL2_MP5Navy, CBase_CSS_HL2_SMG );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeapon_CSS_HL2_MP5Navy(void);
	
	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = VECTOR_CONE_5DEGREES;
		return cone;
	}
	
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	virtual float GetFireRate( void ) { return 0.08f; }
};

IMPLEMENT_SERVERCLASS_ST( CWeapon_CSS_HL2_MP5Navy, DT_Weapon_CSS_HL2_MP5Navy )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_css_mp5navy, CWeapon_CSS_HL2_MP5Navy );
PRECACHE_WEAPON_REGISTER( weapon_css_mp5navy );

BEGIN_DATADESC( CWeapon_CSS_HL2_MP5Navy )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon_CSS_HL2_MP5Navy::CWeapon_CSS_HL2_MP5Navy( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// CWeapon_CSS_HL2_UMP45
//-----------------------------------------------------------------------------
class CWeapon_CSS_HL2_UMP45 : public CBase_CSS_HL2_SMG
{
public:
	DECLARE_CLASS( CWeapon_CSS_HL2_UMP45, CBase_CSS_HL2_SMG );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeapon_CSS_HL2_UMP45(void);
	
	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = VECTOR_CONE_5DEGREES;
		return cone;
	}
	
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	virtual float GetFireRate( void ) { return 0.105f; }

	// Player damage: 8 -> 7.2 (7)
	// NPC damage: 3 -> 3
	virtual float GetDamageMultiplier() const { return 0.9f; }
	virtual float GetNPCDamageMultiplier() const { return 1.0f; }
};

IMPLEMENT_SERVERCLASS_ST( CWeapon_CSS_HL2_UMP45, DT_Weapon_CSS_HL2_UMP45 )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_css_ump45, CWeapon_CSS_HL2_UMP45 );
PRECACHE_WEAPON_REGISTER( weapon_css_ump45 );

BEGIN_DATADESC( CWeapon_CSS_HL2_UMP45 )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon_CSS_HL2_UMP45::CWeapon_CSS_HL2_UMP45( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// CWeapon_CSS_HL2_P90
//-----------------------------------------------------------------------------
class CWeapon_CSS_HL2_P90 : public CBase_CSS_HL2_SMG
{
public:
	DECLARE_CLASS( CWeapon_CSS_HL2_P90, CBase_CSS_HL2_SMG );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeapon_CSS_HL2_P90(void);
	
	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = VECTOR_CONE_5DEGREES;
		return cone;
	}
	
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	virtual float GetFireRate( void ) { return 0.07f; }
};

IMPLEMENT_SERVERCLASS_ST( CWeapon_CSS_HL2_P90, DT_Weapon_CSS_HL2_P90 )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_css_p90, CWeapon_CSS_HL2_P90 );
PRECACHE_WEAPON_REGISTER( weapon_css_p90 );

BEGIN_DATADESC( CWeapon_CSS_HL2_P90 )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon_CSS_HL2_P90::CWeapon_CSS_HL2_P90( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// CWeapon_CSS_HL2_MAC10
//-----------------------------------------------------------------------------
class CWeapon_CSS_HL2_MAC10 : public CBase_CSS_HL2_SMG
{
public:
	DECLARE_CLASS( CWeapon_CSS_HL2_MAC10, CBase_CSS_HL2_SMG );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeapon_CSS_HL2_MAC10(void);
	
	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = VECTOR_CONE_5DEGREES;
		return cone;
	}
	
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 3; }

	virtual float GetFireRate( void ) { return 0.075f; }

	// Player damage: 8 -> 7.2 (7)
	// NPC damage: 3 -> 3
	virtual float GetDamageMultiplier() const { return 0.9f; }
	virtual float GetNPCDamageMultiplier() const { return 1.0f; }
};

IMPLEMENT_SERVERCLASS_ST( CWeapon_CSS_HL2_MAC10, DT_Weapon_CSS_HL2_MAC10 )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_css_mac10, CWeapon_CSS_HL2_MAC10 );
PRECACHE_WEAPON_REGISTER( weapon_css_mac10 );

BEGIN_DATADESC( CWeapon_CSS_HL2_MAC10 )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon_CSS_HL2_MAC10::CWeapon_CSS_HL2_MAC10( void )
{
	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}
