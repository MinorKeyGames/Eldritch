#include "core.h"
#include "wbactioneldaddblock.h"
#include "Components/wbcompeldtransform.h"
#include "Components/wbcompeldcamera.h"
#include "configmanager.h"
#include "wbactionstack.h"
#include "eldritchworld.h"
#include "eldritchframework.h"
#include "wbeventmanager.h"
#include "ray.h"
#include "collisioninfo.h"

WBActionEldAddBlock::WBActionEldAddBlock()
:	m_SourcePE()
,	m_VoxelValue( 0 )
{
}

WBActionEldAddBlock::~WBActionEldAddBlock()
{
}

/*virtual*/ void WBActionEldAddBlock::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Source );
	const SimpleString SourceDef = ConfigManager::GetString( sSource, "", sDefinitionName );
	m_SourcePE.InitializeFromDefinition( SourceDef );

	STATICHASH( VoxelValue );
	m_VoxelValue = static_cast<byte>( ConfigManager::GetInt( sVoxelValue, 0, sDefinitionName ) );
}

/*virtual*/ void WBActionEldAddBlock::Execute()
{
	WBAction::Execute();

	STATIC_HASHED_STRING( EventOwner );
	WBEntity* const pEntity = WBActionStack::Top().GetEntity( sEventOwner );

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = pEntity;

	m_SourcePE.Evaluate( PEContext );
	WBEntity* const pSourceEntity = m_SourcePE.GetEntity();
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

	if( !pWorld->Trace( TraceRay, Info ) )
	{
		return;
	}

	const Vector HitLoc = Info.m_Intersection + Info.m_Plane.m_Normal * 0.1f;
	if( !pWorld->AddVoxelAt( HitLoc, m_VoxelValue ) )
	{
		return;
	}

	// Notify issuing entity that the block was added
	WB_MAKE_EVENT( OnBlockAdded, pEntity );
	WB_SET_AUTO( OnBlockAdded, Vector, HitLocation, HitLoc );
	WB_SET_AUTO( OnBlockAdded, Vector, VoxelLocation, pWorld->GetVoxelCenter( HitLoc ) );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), OnBlockAdded, pEntity );
}