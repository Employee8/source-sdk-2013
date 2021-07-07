//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_Bec : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_Bec, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

					C_Bec();
	virtual			~C_Bec();

private:
	C_Bec( const C_Bec & ); // not defined, not accessible
};

IMPLEMENT_CLIENTCLASS_DT(C_Bec, DT_NPC_Bec, CNPC_Bec)
END_RECV_TABLE()

C_Bec::C_Bec()
{
}


C_Bec::~C_Bec()
{
}


