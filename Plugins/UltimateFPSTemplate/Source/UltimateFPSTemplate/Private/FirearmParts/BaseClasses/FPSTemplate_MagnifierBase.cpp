// Copyright 2021, Dakota Dawe, All rights reserved


#include "FirearmParts/BaseClasses/FPSTemplate_MagnifierBase.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Components/FPSTemplate_CharacterComponent.h"
#include "Components/FPS_SceneCaptureComponent2D.h"

#include "Net/UnrealNetwork.h"

AFPSTemplate_MagnifierBase::AFPSTemplate_MagnifierBase()
{
	PrimaryActorTick.bCanEverTick = false;
	NetUpdateFrequency = 1.0f;
	PartType = EPartType::Magnifier;

	bFlippedOut = false;
	bFullyFlippedOut = false;
}

void AFPSTemplate_MagnifierBase::OnRep_FlippedOut()
{
	OnUse();
}

void AFPSTemplate_MagnifierBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (PartMesh.IsValid())
	{
		FTimerHandle TempHandle;
		GetWorld()->GetTimerManager().SetTimer(TempHandle, this, &AFPSTemplate_MagnifierBase::SetupMagnifier, 0.2f, false);
	}
}

void AFPSTemplate_MagnifierBase::OnRep_Owner()
{
	if (PartMesh.IsValid())
	{
		FTimerHandle TempHandle;
		GetWorld()->GetTimerManager().SetTimer(TempHandle, this, &AFPSTemplate_MagnifierBase::SetupMagnifier, 0.2f, false);
	}
}

void AFPSTemplate_MagnifierBase::SetupMagnifier()
{
	if (PartMesh.IsValid())
	{
		FVector Start = GetAimSocketTransform().GetLocation();
		FVector End = Start + GetAimSocketTransform().Rotator().Vector() * 10.0f;
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		if (GetOwningActor())
		{
			QueryParams.AddIgnoredActor(OwningActor);
		}
		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
		{
			if (AFPSTemplate_SightBase* HitSight = Cast<AFPSTemplate_SightBase>(HitResult.GetActor()))
			{
				IFPSTemplate_AimInterface::Execute_SetMagnifier(HitSight, this);
				SightInfront = HitSight;
			}
		}
	}
}

void AFPSTemplate_MagnifierBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AFPSTemplate_MagnifierBase, bFlippedOut, COND_SkipOwner);
}

bool AFPSTemplate_MagnifierBase::Server_Flip_Validate(bool bFlip)
{
	return true;
}

void AFPSTemplate_MagnifierBase::Server_Flip_Implementation(bool bFlip)
{
	Use();
}

void AFPSTemplate_MagnifierBase::Use_Implementation()
{
	bFlippedOut = !bFlippedOut;
	bFullyFlippedOut = false;
	
	if (bFlippedOut)
	{
		DisableRenderTarget(true);
	}
	else
	{
		UFPSTemplate_CharacterComponent* CharacterComponent = GetOwningCharacterComponent();
		if (IsValid(CharacterComponent) && CharacterComponent->IsAiming())
		{
			DisableRenderTarget(false);
		}
	}
	
	OnUse();
	
	if (!HasAuthority())
	{
		Server_Flip(bFlippedOut);
	}
}
