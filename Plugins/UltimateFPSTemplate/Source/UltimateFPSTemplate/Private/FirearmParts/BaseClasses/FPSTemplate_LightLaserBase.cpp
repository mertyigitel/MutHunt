// Copyright 2021, Dakota Dawe, All rights reserved


#include "FirearmParts/BaseClasses/FPSTemplate_LightLaserBase.h"

#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"

AFPSTemplate_LightLaserBase::AFPSTemplate_LightLaserBase()
{
	PrimaryActorTick.bCanEverTick = true;
	NetUpdateFrequency = 6.5f;
	MinNetUpdateFrequency = 1.0f;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = Root;
	
	SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLightComponent"));
	SpotLightComponent->SetVisibility(false);
	
	LaserMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserMeshComponent"));
	LaserMesh->CastShadow = false;
	LaserMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LaserDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	LaserDecalComponent->SetupAttachment(RootComponent);
	LaserDecalComponent->SetVisibility(false);
	LaserDecalComponent->DecalSize = FVector(3.0f, 3.0f, 3.0f);
	LaserDecalComponent->FadeScreenSize = 0.0f;

	SpotLightComponent->bUseInverseSquaredFalloff = false;
	SpotLightComponent->Intensity = 2.0f;
	SpotLightComponent->AttenuationRadius = 50000.0f;
	SpotLightComponent->InnerConeAngle = 5.0f;
	SpotLightComponent->OuterConeAngle = 12.5;

	bIsLight = true;
	bIsLaser = false;
	CollisionChannel = ECollisionChannel::ECC_Visibility;
	MaxLaserDistance = 10000.0f;

	LightPowerIntensityLevels = {SpotLightComponent->Intensity, 0.8f, 0.2f };
	LightPowerIntensityIndex = 0;
	
	LaserSocket = FName("S_Laser");
	AimSocket = FName("S_Aim");
	
	bLightOn = false;
	bLaserOn = false;

	PartType = EPartType::LightLaser;
	
	LaserColorIndex = 0;
	LazerScaleMultiplier = 45.0f;
}

void AFPSTemplate_LightLaserBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSTemplate_LightLaserBase, bLightOn);
	DOREPLIFETIME(AFPSTemplate_LightLaserBase, bLaserOn);
	DOREPLIFETIME_CONDITION(AFPSTemplate_LightLaserBase, LightPowerIntensityIndex, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AFPSTemplate_LightLaserBase, LaserColorIndex, COND_SkipOwner);
}

void AFPSTemplate_LightLaserBase::PostInitProperties()
{
	Super::PostInitProperties();
	if (!bIsLaser)
	{
		PrimaryActorTick.bCanEverTick = false;
	}
}

void AFPSTemplate_LightLaserBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (PartMesh.IsValid() && bLaserOn)
	{
		if (PartMesh->DoesSocketExist(LaserSocket))
		{
			FVector Start = PartMesh->GetSocketLocation(LaserSocket);
			FRotator Rot = PartMesh->GetSocketRotation(LaserSocket);
			FVector End = Start + Rot.Vector() * MaxLaserDistance;
        
			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
    
			if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, CollisionChannel, Params))
			{
				if (LaserDecalComponent)
				{
					LaserDecalComponent->SetVisibility(true);
					FRotator NormalizedRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
					LaserDecalComponent->SetWorldLocationAndRotation(HitResult.Location, NormalizedRotator);
					float ScaleAmount = HitResult.Distance / 10000.0f * LazerScaleMultiplier;
					ScaleAmount = FMath::Clamp(ScaleAmount, 3.0f, 25.0f);

					LaserDecalComponent->DecalSize = FVector(ScaleAmount);
				}
				LaserMesh->SetWorldScale3D(FVector(1.0f, HitResult.Distance / 20.0f, 1.0f));
				OnLaserScaleChanged(HitResult.Distance);
			}
			else
			{
				if (LaserDecalComponent)
				{
					LaserDecalComponent->SetVisibility(false);
				}
			}
		}
	}
}

void AFPSTemplate_LightLaserBase::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(false);
	if (LaserMesh)
	{
		if (!bLaserOn)
		{
			LaserMesh->SetVisibility(false);
		}
	}
	if (LaserMaterials.Num())
	{
		SetLaserColor(LaserColorIndex);
	}
	if (LightPowerIntensityLevels.Num())
	{
		SpotLightComponent->SetIntensity(LightPowerIntensityLevels[LightPowerIntensityIndex]);
	}
}

void AFPSTemplate_LightLaserBase::SetupPartMesh()
{
	Super::SetupPartMesh();
	if (PartMesh.IsValid())
	{
		LaserMesh->AttachToComponent(PartMesh.Get(), FAttachmentTransformRules::KeepRelativeTransform);
		SpotLightComponent->AttachToComponent(PartMesh.Get(), FAttachmentTransformRules::KeepRelativeTransform);
		LaserDecalComponent->AttachToComponent(PartMesh.Get(), FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void AFPSTemplate_LightLaserBase::OnRep_LightPowerIntensityIndex()
{
	SpotLightComponent->SetIntensity(LightPowerIntensityLevels[LightPowerIntensityIndex]);
}

void AFPSTemplate_LightLaserBase::OnRep_LightOn()
{
	SpotLightComponent->SetVisibility(bLightOn);
	OnUse();
}

void AFPSTemplate_LightLaserBase::OnRep_LaserOn()
{
	if (bLaserOn)
	{
		LaserMesh->SetVisibility(true);
		LaserDecalComponent->SetVisibility(true);
		SetActorTickEnabled(true);
	}
	else
	{
		LaserMesh->SetVisibility(false);
		LaserDecalComponent->SetVisibility(false);
		SetActorTickEnabled(false);
	}
}

void AFPSTemplate_LightLaserBase::OnRep_LaserColorIndex()
{
	if (LaserMesh && LaserColorIndex < LaserMaterials.Num() && LaserMaterials[LaserColorIndex])
	{
		LaserMesh->SetMaterial(0, LaserMaterials[LaserColorIndex]);
		if (LaserDecalComponent && LaserColorIndex < LaserDotMaterials.Num() && LaserDotMaterials[LaserColorIndex])
		{
			LaserDecalComponent->SetMaterial(0, LaserDotMaterials[LaserColorIndex]);
		}
	}
}

bool AFPSTemplate_LightLaserBase::Server_ToggleLight_Validate()
{
	return true;
}

void AFPSTemplate_LightLaserBase::Server_ToggleLight_Implementation()
{
	ToggleLight();
}

bool AFPSTemplate_LightLaserBase::Server_ToggleLaser_Validate()
{
	return true;
}

void AFPSTemplate_LightLaserBase::Server_ToggleLaser_Implementation()
{
	ToggleLaser();
}

bool AFPSTemplate_LightLaserBase::Server_CycleLaserColor_Validate(uint8 Index)
{
	return true;
}

void AFPSTemplate_LightLaserBase::Server_CycleLaserColor_Implementation(uint8 Index)
{
	LaserColorIndex = Index;
	OnRep_LaserColorIndex();
}

void AFPSTemplate_LightLaserBase::ToggleLight()
{
	if (!bIsLight)
	{
		return;
	}
	bLightOn = !bLightOn;
    OnRep_LightOn();
    if (!HasAuthority())
    {
		Server_ToggleLight();
    }
}

void AFPSTemplate_LightLaserBase::ToggleLaser()
{
	if (!bIsLaser)
	{
		return;
	}
	bLaserOn = !bLaserOn;
	OnRep_LaserOn();
	if (!HasAuthority())
	{
		Server_ToggleLaser();
	}
}

bool AFPSTemplate_LightLaserBase::Server_ToggleLightAndLaser_Validate(bool bSync)
{
	return true;
}

void AFPSTemplate_LightLaserBase::Server_ToggleLightAndLaser_Implementation(bool bSync)
{
	ToggleLightAndLaser(bSync);
}

void AFPSTemplate_LightLaserBase::ToggleLightAndLaser(bool bSync)
{
	if (bSync)
	{
		bool ToggleValue;
		if (bIsLight)
		{
			ToggleValue = !bLightOn;
			bLightOn = ToggleValue;
			OnRep_LightOn();
			bLaserOn = ToggleValue;
			OnRep_LaserOn();
		}
		else if (bIsLaser)
		{
			ToggleValue = !bLaserOn;
			bLightOn = ToggleValue;
			OnRep_LightOn();
			bLaserOn = ToggleValue;
			OnRep_LaserOn();
		}
	}
	else
	{
		if (bIsLight)
		{
			bLightOn = !bLightOn;
			OnRep_LightOn();
		}
	
		if (bIsLaser)
		{
			bLaserOn = !bLaserOn;
			OnRep_LaserOn();
		}
	}
	
	if (!HasAuthority())
	{
		Server_ToggleLightAndLaser(bSync);
	}
}

bool AFPSTemplate_LightLaserBase::Server_CyclePowerModes_Validate(uint8 Index)
{
	return true;
}

void AFPSTemplate_LightLaserBase::Server_CyclePowerModes_Implementation(uint8 Index)
{
	SetPowerMode(Index);
}

void AFPSTemplate_LightLaserBase::CyclePowerModes()
{
	if (LightPowerIntensityLevels.Num() == 0)
	{
		return;
	}

	if (++LightPowerIntensityIndex > LightPowerIntensityLevels.Num() - 1)
	{
		LightPowerIntensityIndex = 0;
	}

	SetPowerMode(LightPowerIntensityIndex);

	if (!HasAuthority())
	{
		Server_CyclePowerModes(LightPowerIntensityIndex);
	}
}

void AFPSTemplate_LightLaserBase::SetPowerMode(uint8 Index)
{
	if (Index < LightPowerIntensityLevels.Num())
	{
		LightPowerIntensityIndex = Index;
	}
	SpotLightComponent->SetIntensity(LightPowerIntensityLevels[LightPowerIntensityIndex]);
}

void AFPSTemplate_LightLaserBase::CycleThroughModes()
{
	if (CycleModes.Num() == 0)
	{
		return;
	}

	switch (CycleModes[CycleModesIndex])
	{
	case ELightLaser::Laser :
		if (!bLaserOn)
		{
			ToggleLaser();
		}
		if (bLightOn)
		{
			ToggleLight();
		} break;
	case ELightLaser::Light :
		if (!bLightOn)
		{
			ToggleLight();
		}
		if (bLaserOn)
		{
			ToggleLaser();
		} break;
	case ELightLaser::Off :
		if (bLaserOn)
		{
			ToggleLaser();
		}
		if (bLightOn)
		{
			ToggleLight();
		} break;
	case ELightLaser::Both :
		if (!bLaserOn)
		{
			ToggleLaser();
		}
		if (!bLightOn)
		{
			ToggleLight();
		} break;
	}

	if (++CycleModesIndex >= CycleModes.Num())
	{
		CycleModesIndex = 0;
	}
}

void AFPSTemplate_LightLaserBase::CycleLaserColor()
{
	if (++LaserColorIndex >= LaserMaterials.Num())
	{
		LaserColorIndex = 0;
	}
	OnRep_LaserColorIndex();
	
	if (!HasAuthority())
	{
		Server_CycleLaserColor(LaserColorIndex);
	}
}

void AFPSTemplate_LightLaserBase::SetLaserColor(uint8 Index)
{
	if (Index < LaserMaterials.Num())
	{
		LaserColorIndex = Index;
		OnRep_LaserColorIndex();
	
		if (!HasAuthority())
		{
			Server_CycleLaserColor(LaserColorIndex);
		}
	}
}