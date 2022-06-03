// Copyright 2022, Dakota Dawe, All rights reserved

#include "Components/FPS_FirearmStabilizerComponent.h"

#include "DrawDebugHelpers.h"
#include "FPSTemplateAnimInstance.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Components/FPSTemplate_CharacterComponent.h"

UFPS_FirearmStabilizerComponent::UFPS_FirearmStabilizerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	DebugType = EDrawDebugTrace::None;
	bOnlyRunOnOwner = true;
	Size = FVector(8.0f, 10.0f, 5.0f);
	UpdateRate = 10.0f;

	bIsStabilized = false;
	bOnlyWithKeyPress = false;
}

void UFPS_FirearmStabilizerComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickInterval(1.0f / UpdateRate);
	SetComponentTickEnabled(false);

	CacheEssentials();

#if WITH_EDITOR
	bIsLive = true;
	if (EditorDebugHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(EditorDebugHandle);
	}
#endif
}

#if WITH_EDITOR
void UFPS_FirearmStabilizerComponent::PostInitProperties()
{
	Super::PostInitProperties();

	if (!bIsLive && GetWorld() && !EditorDebugHandle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(EditorDebugHandle, this, &UFPS_FirearmStabilizerComponent::DrawDebugEditor, 0.001f, true);
	}
}

void UFPS_FirearmStabilizerComponent::DrawDebugEditor()
{
	if (bIsLive)
	{
		GetWorld()->GetTimerManager().ClearTimer(EditorDebugHandle);
	}
	else
	{
		DrawDebugBox(GetWorld(), GetComponentLocation(), Size, FColor::Red, false, 0.001f);
	}
}
#endif

void UFPS_FirearmStabilizerComponent::CacheEssentials()
{
	Firearm = Cast<AFPSTemplateFirearm>(GetOwner());
	if (IsValid(Firearm))
	{
		CharacterComponent = Firearm->GetCharacterComponent();
	}
}

void UFPS_FirearmStabilizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (FindStabilizingSurface())
	{
		if (!bIsStabilized)
		{
			StabilizeFirearm();
		}
	}
	else if (bIsStabilized)
	{
		UnStabilizeFirearm();
	}
}

bool UFPS_FirearmStabilizerComponent::FindStabilizingSurface()
{
	FHitResult HitResult;
	TArray<AActor*> ActorsIgnored;
	ActorsIgnored.Add(GetOwner());

	if (IsValid(Firearm))
	{
		ActorsIgnored.Append(Firearm->GetFirearmParts());
	}
	if (IsValid(CharacterComponent))
	{
		ActorsIgnored.Add(CharacterComponent->GetOwner());
	}
	
	return UKismetSystemLibrary::BoxTraceSingle(GetWorld(), GetComponentLocation(), GetComponentLocation(), Size,
		GetComponentRotation(), CollisionChannel, false, ActorsIgnored, DebugType, HitResult, true);
}

void UFPS_FirearmStabilizerComponent::StabilizeFirearm()
{
	if (IsValid(Firearm) && IsValid(CharacterComponent) && IsValid(CharacterComponent->GetAnimationInstance()))
	{
		CharacterComponent->GetAnimationInstance()->PlayCustomCurve(StabilizedCurveData);
		Firearm->SetSwayMultiplier(0.15f);
		bIsStabilized = true;
		FirearmStabilized.Broadcast();
	}
}

void UFPS_FirearmStabilizerComponent::AttemptStabilizeFirearm()
{
	if (FindStabilizingSurface())
	{
		StabilizeFirearm();
		SetComponentTickEnabled(true);
	}
}

void UFPS_FirearmStabilizerComponent::UnStabilizeFirearm()
{
	if (IsValid(Firearm) && IsValid(CharacterComponent) && IsValid(CharacterComponent->GetAnimationInstance()))
	{
		CharacterComponent->GetAnimationInstance()->PlayCustomCurve(UnStabilizedCurveData);
		Firearm->ResetSwayMultiplier();
		bIsStabilized = false;
		FirearmUnStabilized.Broadcast();
	}
	if (bOnlyWithKeyPress)
	{
		SetComponentTickEnabled(false);
	}
}

void UFPS_FirearmStabilizerComponent::Enable()
{
	if (bOnlyWithKeyPress)
	{
		return;
	}
	if (bOnlyRunOnOwner)
	{
		if (IsValid(Firearm->GetOwner()))
		{
			const APawn* OwningPawn = Cast<APawn>(Firearm->GetOwner());
			if (IsValid(OwningPawn) && OwningPawn->IsLocallyControlled())
			{
				SetComponentTickInterval(0.1f);
				SetComponentTickEnabled(true);
			}
		}
	}
	else
	{
		SetComponentTickEnabled(true);
	}
}

void UFPS_FirearmStabilizerComponent::Disable()
{
	SetComponentTickEnabled(false);
}