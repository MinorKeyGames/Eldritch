#include "core.h"
#include "wbactioneldspawnentity.h"
#include "configmanager.h"
#include "angles.h"
#include "Components/wbcompeldtransform.h"
#include "Components/wbcompeldcamera.h"
#include "Components/wbcompeldheadtracker.h"
#include "Components/wbcompowner.h"
#include "Components/wbcompeldcollision.h"
#include "wbactionstack.h"
#include "wbeventmanager.h"
#include "eldritchframework.h"
#include "eldritchworld.h"
#include "collisioninfo.h"

WBActionEldSpawnEntity::WBActionEldSpawnEntity()
:	m_EntityDef()
,	m_EntityDefPE()
,	m_LocationOverridePE()
,	m_OrientationOverridePE()
,	m_YawOnly( false )
,	m_SpawnImpulseZ( 0.0f )
,	m_SpawnImpulse( 0.0f )
,	m_SpawnImpulsePE()
,	m_SpawnOffsetZ( 0.0f )
{
}

WBActionEldSpawnEntity::~WBActionEldSpawnEntity()
{
}

/*virtual*/ void WBActionEldSpawnEntity::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Entity );
	m_EntityDef = ConfigManager::GetString( sEntity, "", sDefinitionName );

	STATICHASH( EntityPE );
	const SimpleString EntityDefPE = ConfigManager::GetString( sEntityPE, "", sDefinitionName );
	m_EntityDefPE.InitializeFromDefinition( EntityDefPE );

	STATICHASH( LocationOverridePE );
	const SimpleString LocationOverridePE = ConfigManager::GetString( sLocationOverridePE, "", sDefinitionName );
	m_LocationOverridePE.InitializeFromDefinition( LocationOverridePE );

	STATICHASH( OrientationOverridePE );
	const SimpleString OrientationOverridePE = ConfigManager::GetString( sOrientationOverridePE, "", sDefinitionName );
	m_OrientationOverridePE.InitializeFromDefinition( OrientationOverridePE );

	STATICHASH( YawOnly );
	m_YawOnly = ConfigManager::GetBool( sYawOnly, false, sDefinitionName );

	STATICHASH( SpawnImpulseZ );
	m_SpawnImpulseZ = ConfigManager::GetFloat( sSpawnImpulseZ, 0.0f, sDefinitionName );

	STATICHASH( SpawnImpulse );
	m_SpawnImpulse = ConfigManager::GetFloat( sSpawnImpulse, 0.0f, sDefinitionName );

	STATICHASH( SpawnImpulsePE );
	const SimpleString SpawnImpulsePE = ConfigManager::GetString( sSpawnImpulsePE, "", sDefinitionName );
	m_SpawnImpulsePE.InitializeFromDefinition( SpawnImpulsePE );

	STATICHASH( SpawnOffsetZ );
	m_SpawnOffsetZ = ConfigManager::GetFloat( sSpawnOffsetZ, 0.0f, sDefinitionName );
}

// Borrowed from WBCompEldItem spawn drop code. Maybe unify?
/*virtual*/ void WBActionEldSpawnEntity::Execute()
{
	WBAction::Execute();

	WBEntity* const				pEntity				= GetOwner();

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = pEntity;
	m_EntityDefPE.Evaluate( PEContext );
	const SimpleString			EntityDef			= ( m_EntityDefPE.GetType() == WBParamEvaluator::EPT_String ) ? m_EntityDefPE.GetString() : m_EntityDef;

	WBEntity* const				pSpawnedEntity		= WBWorld::GetInstance()->CreateEntity( EntityDef );

	WBCompEldTransform* const	pSpawnedTransform	= pSpawnedEntity->GetTransformComponent<WBCompEldTransform>();
	ASSERT( pSpawnedTransform );

	WBCompOwner* const			pSpawnedOwner		= GET_WBCOMP( pSpawnedEntity, Owner );

	if( pSpawnedOwner )
	{
		pSpawnedOwner->SetOwner( pEntity );
	}

	Vector SpawnLocation;
	Vector SpawnImpulse;
	Angles SpawnOrientation;
	GetSpawnTransform( pSpawnedEntity, SpawnLocation, SpawnImpulse, SpawnOrientation );

	pSpawnedTransform->SetLocation(		SpawnLocation );
	pSpawnedTransform->SetOrientation(	SpawnOrientation );
	pSpawnedTransform->ApplyImpulse(	SpawnImpulse );

	WB_MAKE_EVENT( OnInitialOrientationSet, pSpawnedEntity );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), OnInitialOrientationSet, pSpawnedEntity );

	// Notify instigator that we spawned this thing
	WB_MAKE_EVENT( OnSpawnedEntityAction, GetEntity() );
	WB_SET_AUTO( OnSpawnedEntityAction, Entity, SpawnedEntity, pSpawnedEntity );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), OnSpawnedEntityAction, GetEntity() );
}

// Borrowed (with simplifications) from WBCompEldItem spawn drop code. Maybe unify?
void WBActionEldSpawnEntity::GetSpawnTransform( WBEntity* const pSpawnedEntity, Vector& OutLocation, Vector& OutImpulse, Angles& OutOrientation )
{
	WBEntity* const				pEntity			= GetOwner();
	DEVASSERT( pEntity );

	WBCompEldTransform* const	pTransform		= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompEldCamera* const		pCamera			= GET_WBCOMP( pEntity, EldCamera );

	WBCompEldHeadTracker* const	pHeadTracker	= GET_WBCOMP( pEntity, EldHeadTracker );

	// Get location
	{
		if( m_LocationOverridePE.HasRoot() )
		{
			WBParamEvaluator::SPEContext PEContext;
			PEContext.m_Entity = pEntity;
			m_LocationOverridePE.Evaluate( PEContext );
			OutLocation = m_LocationOverridePE.GetVector();
		}
		else
		{
			if( pHeadTracker )
			{
				OutLocation			= pHeadTracker->GetEyesLocation();
			}
			else
			{
				OutLocation			= pTransform->GetLocation();
				if( pCamera )
				{
					OutLocation		+= pCamera->GetViewTranslationOffset( WBCompEldCamera::EVM_All );
				}
			}
		}

		OutLocation.z			+= m_SpawnOffsetZ;

		WBCompEldCollision* const pCollision = GET_WBCOMP( pSpawnedEntity, EldCollision );
		if( pCollision )
		{
			CollisionInfo Info;
			Info.m_CollideWorld		= true;
			Info.m_CollideEntities	= true;
			Info.m_CollidingEntity	= pEntity;	// Using the owner, not the spawned entity (which should be at origin at the moment)
			Info.m_UserFlags		= EECF_EntityCollision;

			EldritchWorld* const pWorld = EldritchFramework::GetInstance()->GetWorld();
			pWorld->FindSpot( OutLocation, pCollision->GetExtents(), Info );
		}
	}

	// Get orientation
	{
		if( m_OrientationOverridePE.HasRoot() )
		{
			WBParamEvaluator::SPEContext PEContext;
			PEContext.m_Entity = pEntity;
			m_OrientationOverridePE.Evaluate( PEContext );
			OutOrientation = m_OrientationOverridePE.GetVector().ToAngles();
		}
		else
		{
			if( pHeadTracker )
			{
				OutOrientation		= pHeadTracker->GetLookDirection().ToAngles();
			}
			else
			{
				OutOrientation		= pTransform->GetOrientation();
				if( pCamera )
				{
					OutOrientation	+= pCamera->GetViewOrientationOffset( WBCompEldCamera::EVM_All );
				}
			}
		}

		if( m_YawOnly )
		{
			OutOrientation.Pitch	= 0.0f;
			OutOrientation.Roll		= 0.0f;
		}
	}

	// Get impulse
	{
		if( pHeadTracker )
		{
			OutImpulse			= pHeadTracker->GetLookDirection();
		}
		else
		{
			OutImpulse			= pTransform->GetOrientation().ToVector();
		}

		OutImpulse.z			+= m_SpawnImpulseZ;
		OutImpulse.FastNormalize();

		WBParamEvaluator::SPEContext PEContext;
		PEContext.m_Entity = pEntity;
		m_SpawnImpulsePE.Evaluate( PEContext );
		const float SpawnImpulseSize = ( m_SpawnImpulsePE.GetType() == WBParamEvaluator::EPT_Float ) ? m_SpawnImpulsePE.GetFloat() : m_SpawnImpulse;
		OutImpulse *= SpawnImpulseSize;
	}
}