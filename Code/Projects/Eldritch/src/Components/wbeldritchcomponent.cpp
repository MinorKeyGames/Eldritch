#include "core.h"
#include "wbeldritchcomponent.h"
#include "eldritchframework.h"
#include "wbworld.h"

WBEldritchComponent::WBEldritchComponent()
{
}

WBEldritchComponent::~WBEldritchComponent()
{
}

EldritchFramework* WBEldritchComponent::GetFramework() const
{
	return EldritchFramework::GetInstance();
}

EldritchGame* WBEldritchComponent::GetGame() const
{
	return EldritchFramework::GetInstance()->GetGame();
}

EldritchWorld* WBEldritchComponent::GetWorld() const
{
	return EldritchFramework::GetInstance()->GetWorld();
}