// Copyright 2021, Dakota Dawe, All rights reserved

#include "FirearmParts/BaseClasses/FPSTemplate_SightBase.h"
#include "Actors/FPSTemplateFirearm.h"

#include "Materials/MaterialInstanceDynamic.h"

AFPSTemplate_SightBase::AFPSTemplate_SightBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	PartStats.Weight = 0.1f;
	PartStats.ErgonomicsChangePercentage = -2.0f;
	CameraSettings.CameraFOVZoom = 10.0f;
	CameraSettings.CameraFOVZoomSpeed = 10.0f;
	CameraSettings.CameraDistance = 0.0f;
	CameraSettings.bUsedFixedCameraDistance = false;

	bIsAimable = true;

	PartType = EPartType::Sight;

	SightZero = FSightZero();

	ReticleIndex = 0;
	ReticleBrightnessIndex = 0;
	TurretAdjustmentType = EScopeAdjustment::MRAD;

	AnimationIndex = -1;
	AimInterpolationMultiplier = 50.0f;
	RotationLagInterpolationMultiplier = 50.0f;
	MovementLagInterpolationMultiplier = 50.0f;
	GripSocket = FName("cc_FirearmGrip");
	HeadAimRotation = FRotator(0.0f, 0.0f, 0.0f);

	MilliradianAdjustment = 0.005625f;
	MOAAdjustment = 0.004167f;
}

// Called when the game starts or when spawned
void AFPSTemplate_SightBase::BeginPlay()
{
	Super::BeginPlay();

	ReticleBrightnessIndex = ReticleSettings.ReticleBrightness.ReticleDefaultBrightnessIndex;
	if (ReticleBrightnessIndex > ReticleSettings.ReticleBrightness.ReticleBrightnessSettings.Num())
	{
		ReticleBrightnessIndex = ReticleSettings.ReticleBrightness.ReticleBrightnessSettings.Num();
		ReticleSettings.ReticleBrightness.ReticleDefaultBrightnessIndex = ReticleBrightnessIndex;
	}

	for (FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
	{
		if (Reticle.ReticleMaterial)
		{
			Reticle.DynamicReticleMaterial = UMaterialInstanceDynamic::Create(Reticle.ReticleMaterial, this);
			Reticle.DynamicReticleMaterial->GetScalarParameterValue(FName("ReticleSize"), Reticle.ReticleSize);
		}
	}

	SetReticleBrightness(ReticleBrightnessIndex);
	
	SetReticle(ReticleIndex);
}

void AFPSTemplate_SightBase::PostInitProperties()
{
	Super::PostInitProperties();

	if (PartMesh.IsValid())
	{
		PartMesh->SetCollisionResponseToChannel(FirearmCollisionChannel, ECR_Ignore);
	}
}

void AFPSTemplate_SightBase::SetReticleBrightness(uint8 Index)
{
	if (Index < ReticleSettings.ReticleBrightness.ReticleBrightnessSettings.Num())
	{
		for (FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
		{
			if (Reticle.DynamicReticleMaterial)
			{
				Reticle.DynamicReticleMaterial->SetScalarParameterValue(FName("ReticleBrightness"), ReticleSettings.ReticleBrightness.ReticleBrightnessSettings[Index]);
			}
		}
	}
}

void AFPSTemplate_SightBase::CycleReticle()
{
	if (ReticleSettings.ReticleMaterials.Num())
	{
		if (++ReticleIndex >= ReticleSettings.ReticleMaterials.Num())
		{
			ReticleIndex = 0;
		}
		SetReticle(ReticleIndex);
	}
}

void AFPSTemplate_SightBase::SetReticle(uint8 Index)
{
	if (PartMesh.IsValid() && Index < ReticleSettings.ReticleMaterials.Num() && ReticleSettings.ReticleMaterials[Index].DynamicReticleMaterial)
	{
		ReticleIndex = Index;
		PartMesh->SetMaterial(ReticleSettings.ReticleMaterialIndex, ReticleSettings.ReticleMaterials[ReticleIndex].DynamicReticleMaterial);
	}
}

void AFPSTemplate_SightBase::IncreaseBrightness()
{
	if (++ReticleBrightnessIndex < ReticleSettings.ReticleBrightness.ReticleBrightnessSettings.Num())
	{
		SetReticleBrightness(ReticleBrightnessIndex);
	}
	else
	{
		ReticleBrightnessIndex = ReticleSettings.ReticleBrightness.ReticleBrightnessSettings.Num() - 1;
	}
}

void AFPSTemplate_SightBase::DecreaseBrightness()
{
	if (ReticleBrightnessIndex - 1 >= 0)
	{
		--ReticleBrightnessIndex;
		SetReticleBrightness(ReticleBrightnessIndex);
	}
	else
	{
		ReticleBrightnessIndex = 0;
	}
}