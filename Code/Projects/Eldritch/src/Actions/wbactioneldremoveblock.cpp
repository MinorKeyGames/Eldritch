#include "core.h"
#include "wbactioneldremoveblock.h"
#include "Components/wbcompeldtransform.h"
#include "Components/wbcompeldcamera.h"
#include "configmanager.h"
#include "wbactionstack.h"
#include "eldritchworld.h"
#include "eldritchframework.h"
#include "wbevent.h"
#include "ray.h"
#include "collisioninfo.h"

WBActionEldRemoveBlock::WBActionEldRemoveBlock()
:	m_SourcePE()
{
}

WBActionEldRemoveBlock::~WBActionEldRemoveBlock()
{
}

/*virtual*/ void WBActionEldRemoveBlock::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Source );
	const SimpleString SourceDef = ConfigManager::GetString( sSource, "", sDefinitionName );
	m_SourcePE.InitializeFromDefinition( SourceDef );
}

void WBActionEldRemoveBlock::TraceFromSourceEntity( WBEntity* const pSourceEntity ) const
{
	if( !pSourceEntity )
	{
		return;
	}

	WBCompEldTransform* const pTransform = pSourceEntity->GetTransformComponent<WBCompEldTransform>();
	if( !pTransform )
	{
		return;
	}

	// TODO: If I reuse this stuff a lot in weapons and powers, wrap it up.

	WBCompEldCamera* const pCamera = GET_WBCOMP( pSourceEntity, EldCamera );

	EldritchWorld* const pWorld = EldritchFramework::GetInstance()->GetWorld();
	ASSERT( pWorld );

	Vector EyeLoc = pTransform->GetLocation();
	Angles EyeRot = pTransform->GetOrientation();

	if( pCamera )
	{
		EyeLoc += pCamera->GetViewTranslationOffset( WBCompEldCamera::EVM_All );
		EyeRot += pCamera->GetViewOrientationOffset( WBCompEldCamera::EVM_All );
	}

	const Vector EyeDir = EyeRot.ToVector();
	const Ray TraceRay( EyeLoc, EyeDir );

	CollisionInfo Info;
	Info.m_CollideWorld		= true;
	Info.m_CollideEntities	= true;
	Info.m_CollidingEntity	= pSourceEntity;
	Info.m_UserFlags		= EECF_Trace;

	if( pWorld->Trace( TraceRay, Info ) )
	{
		const Vector HitLoc = Info.m_Intersection - Info.m_Plane.m_Normal * 0.1f;
		pWorld->RemoveVoxelAt( HitLoc );
	}
}

void WBActionEldRemoveBlock::RemoveAtSourceVector( const Vector& SourceLocation ) const
{
	EldritchWorld* const pWorld = EldritchFramework::GetInstance()->GetWorld();
	ASSERT( pWorld );

	pWorld->RemoveVoxelAt( SourceLocation );
}

/*virtual*/ void WBActionEldRemoveBlock::Execute()
{
	WBAction::Execute();

	STATIC_HASHED_STRING( EventOwner );
	WBEntity* const pEntity = WBActionStack::Top().GetEntity( sEventOwner );

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = pEntity;

	m_SourcePE.Evaluate( PEContext );

	if( m_SourcePE.GetType() == WBParamEvaluator::EPT_Entity )
	{
		TraceFromSourceEntity( m_SourcePE.GetEntity() );
	}
	else if( m_SourcePE.GetType() == WBParamEvaluator::EPT_Vector )
	{
		RemoveAtSourceVector( m_SourcePE.GetVector() );
	}
}