//=============================================================================//
//
// Purpose: CS:S weapons recreated from scratch in Source SDK 2013 for usage in a Half-Life 2 setting.
//
// Author: Blixibon
//
//=============================================================================//

#ifndef WEAPON_CSS_BASE_H
#define WEAPON_CSS_BASE_H

#include "basehlcombatweapon.h"

template <class BASE_WEAPON>
class CBase_CSS_HL2_Weapon : public BASE_WEAPON
{
	DECLARE_CLASS_NOFRIEND( CBase_CSS_HL2_Weapon, BASE_WEAPON );

public:

	// This is used to replicate CS:S's damage differences between weapons
	virtual float GetDamageMultiplier() const { return 1.0f; }
	virtual float GetNPCDamageMultiplier() const { return 1.0f; }

	bool IsSilenced() const { return m_bSilenced; }
	bool CanToggleSilencer() const { return m_bCanToggleSilencer; }

	bool SendWeaponAnim( int iActivity )
	{
		if (IsSilenced())
		{
			switch (iActivity)
			{
				case ACT_VM_PRIMARYATTACK:
					iActivity = ACT_VM_PRIMARYATTACK_SILENCED;
					break;

				case ACT_VM_RELOAD:
					iActivity = ACT_VM_RELOAD_SILENCED;
					break;

				case ACT_VM_DRYFIRE:
					iActivity = ACT_VM_DRYFIRE_SILENCED;
					break;

				case ACT_VM_IDLE:
					iActivity = ACT_VM_IDLE_SILENCED;
					break;

				case ACT_VM_DRAW:
					iActivity = ACT_VM_DRAW_SILENCED;
					break;
			}
		}

		return BaseClass::SendWeaponAnim( iActivity );
	}

	void WeaponSound( WeaponSound_t sound_type, float soundtime = 0.0f )
	{
		if (IsSilenced())
		{
			switch (sound_type)
			{
				case SINGLE:
				case SINGLE_NPC:
					sound_type = SPECIAL1;
					break;
			}
		}

		BaseClass::WeaponSound( sound_type, soundtime );
	}

protected:
	bool	m_bSilenced;
	bool	m_bCanToggleSilencer;
};

template <class BASE_ITEM>
class CBase_CSS_HL2_Item : public BASE_ITEM
{
	DECLARE_CLASS_NOFRIEND( CBase_CSS_HL2_Item, BASE_ITEM );

public:
};

//---------------------------------------------------

#ifdef MAPBASE
extern acttable_t *GetPistolActtable();
extern int GetPistolActtableCount();
#endif

class CBase_CSS_HL2_Pistol : public CBase_CSS_HL2_Weapon<CBaseHLCombatWeapon>
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CBase_CSS_HL2_Pistol, CBase_CSS_HL2_Weapon<CBaseHLCombatWeapon> );

	CBase_CSS_HL2_Pistol(void);

	void	Precache( void );
	void	ItemPostFrame( void );
	void	ItemPreFrame( void );
	void	ItemBusyFrame( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	AddViewKick( void );
	void	DryFire( void );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void	FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void	Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );

	void	UpdatePenaltyTime( void );

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	Activity	GetPrimaryAttackActivity( void );

	virtual bool Reload( void );

	//-----------------------------------------------------------------------------

	inline float GetAccuracyPenalty() { return m_flAccuracyPenalty; }

	virtual float GetRefireRate() { return 0.1f; }
	virtual float GetDryRefireRate() { return 0.2f; }

	virtual float GetShotPenaltyTime() { return 0.2f; }
	virtual float GetMaxShotPenalty() { return 1.5f; }
	
	//-----------------------------------------------------------------------------

#ifdef MAPBASE
	virtual acttable_t		*GetBackupActivityList() { return GetPistolActtable(); }
	virtual int				GetBackupActivityListCount() { return GetPistolActtableCount(); }
#endif

	DECLARE_ACTTABLE();

private:
	float	m_flSoonestPrimaryAttack;
	float	m_flLastAttackTime;
	float	m_flAccuracyPenalty;
	int		m_nNumShotsFired;
};

//---------------------------------------------------

#ifdef MAPBASE
extern acttable_t *GetSMG1Acttable();
extern int GetSMG1ActtableCount();

extern acttable_t *GetAR2Acttable();
extern int GetAR2ActtableCount();
#endif

class CBase_CSS_HL2_MachineGun : public CBase_CSS_HL2_Weapon<CHLMachineGun>
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CBase_CSS_HL2_MachineGun, CBase_CSS_HL2_Weapon<CHLMachineGun> );

	CBase_CSS_HL2_MachineGun(void);
	
	void	Precache( void );
	void	AddViewKick( void );

	int		GetMinBurst() { return 2; }
	int		GetMaxBurst() { return 5; }

	float	GetFireRate( void ) { return 0.075f; }	// 13.3hz
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	Activity	GetPrimaryAttackActivity( void );

	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = VECTOR_CONE_5DEGREES;
		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	
	//-----------------------------------------------------------------------------

#ifdef MAPBASE
	virtual acttable_t		*GetBackupActivityList() { return GetSMG1Acttable(); }
	virtual int				GetBackupActivityListCount() { return GetSMG1ActtableCount(); }
#endif

	DECLARE_ACTTABLE();

private:
};

class CBase_CSS_HL2_SMG : public CBase_CSS_HL2_MachineGun
{
public:
	DECLARE_CLASS( CBase_CSS_HL2_SMG, CBase_CSS_HL2_MachineGun );

#ifdef MAPBASE
	virtual acttable_t		*GetBackupActivityList() { return GetSMG1Acttable(); }
	virtual int				GetBackupActivityListCount() { return GetSMG1ActtableCount(); }
#endif

private:
};

class CBase_CSS_HL2_Rifle : public CBase_CSS_HL2_MachineGun
{
public:
	DECLARE_CLASS( CBase_CSS_HL2_Rifle, CBase_CSS_HL2_MachineGun );

#ifdef MAPBASE
	virtual acttable_t		*GetBackupActivityList() { return GetAR2Acttable(); }
	virtual int				GetBackupActivityListCount() { return GetAR2ActtableCount(); }
#endif

private:
};

#endif // WEAPON_CSS_BASE_H
