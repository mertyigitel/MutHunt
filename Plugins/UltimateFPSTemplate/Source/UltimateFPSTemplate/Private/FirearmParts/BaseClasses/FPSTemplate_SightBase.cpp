// Copyright 2021, Dakota Dawe, All rights reserved

#include "FirearmParts/BaseClasses/FPSTemplate_SightBase.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Camera/CameraComponent.h"
#include "FirearmParts/BaseClasses/FPSTemplate_MagnifierBase.h"
#include "Components/FPS_SceneCaptureComponent2D.h"
#include "Components/FPSTemplate_CharacterComponent.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Net/UnrealNetwork.h"

#define OpticLocation FName("OpticLocation")
#define DistanceFromOptic FName("DistanceFromOptic")
#define OpticRadius FName("OpticRadius")

AFPSTemplate_SightBase::AFPSTemplate_SightBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	NetUpdateFrequency = 1.0f;
	
	PartStats.Weight = 0.1f;
	PartStats.ErgonomicsChangePercentage = -2.0f;
	CameraSettings.CameraFOVZoom = 10.0f;
	CameraSettings.CameraFOVZoomSpeed = 10.0f;
	CameraSettings.CameraDistance = 0.0f;
	CameraSettings.bUsedFixedCameraDistance = false;

	bIsAimable = true;

	PartType = EPartType::Sight;

	DotZero = FSightZero();

	ReticleIndex = 0;
	ReticleBrightnessIndex = 0;

	AnimationIndex = -1;
	AimInterpolationMultiplier = 1.0f;
	RotationLagInterpolationMultiplier = 50.0f;
	MovementLagInterpolationMultiplier = 50.0f;
	GripSocket = FName("cc_FirearmGrip");
	HeadAimRotation = FRotator(0.0f, 0.0f, 0.0f);

	bCanBeZeroed = true;
	bStartWithRandomZero = false;
	RandomMaxElevationStartClicks = 25;
	RandomMaxWindageStartClicks = 25;
	
	MilliradianAdjustment = 0.005625f;
	MOAAdjustment = 0.004167f;

	RedDotZeroAmount = 0.0005f;
	CurrentDotElevation = 0.0f;
	CurrentDotWindage = 0.0f;
	StartingDotElevation = 0.0f;
	StartingDotWindage = 0.0f;
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

	if (IsValid(SceneCapture))
	{
		SceneCapture->SetHiddenInGame(false);
		SceneCapture->SetReticleSettings(ReticleSettings);
		SceneCapture->SetOwningMesh(PartMesh.Get());
		DisableRenderTarget(true);
	}
	if (bStartWithRandomZero)
	{
		SetRandomZero();
	}

	if (MPC)
	{
		MPCInstance = GetWorld()->GetParameterCollectionInstance(MPC);
		if (MPCInstance)
		{
			CacheCharacterAndFirearm();
			if (IsValid(OwningCharacterComponent) && OwningCharacterComponent->IsLocallyControlled())
			{
				//SetActorTickEnabled(true);
			}
		}
	}
}

void AFPSTemplate_SightBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SceneCapture = FindComponentByClass<UFPS_SceneCaptureComponent2D>();
	if (IsValid(SceneCapture))
	{
		bHasRenderTarget = true;
		SceneCapture->SetReticleSettings(ReticleSettings);
	}
}

void AFPSTemplate_SightBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MPCInstance && OwningCharacterComponent && OwningCharacterComponent->GetCurrentSight() == this)
	{
		MPCInstance->SetVectorParameterValue(OpticLocation, GetAimSocketTransform().GetLocation());
		const float DistanceToOptic = FVector::Distance(GetAimSocketTransform().GetLocation(), OwningCharacterComponent->GetCameraComponent()->GetComponentLocation());
		MPCInstance->SetScalarParameterValue(DistanceFromOptic, DistanceToOptic);
		MPCInstance->SetScalarParameterValue(OpticRadius, ReticleSettings.Radius);
		//UE_LOG(LogTemp, Warning, TEXT("DistFromOptic: %f   Optic: %s"), DistanceToOptic, *GetName());
	}
}

void AFPSTemplate_SightBase::SetRandomZero()
{
	const uint8 ElevationStart = FMath::RandRange(0, RandomMaxElevationStartClicks);
	const uint8 WindageStart = FMath::RandRange(0, RandomMaxWindageStartClicks);
	if (FMath::RandBool())
	{
		PointOfImpactUp(ElevationStart);
	}
	else
	{
		PointOfImpactDown(ElevationStart);
	}
	if (FMath::RandBool())
	{
		PointOfImpactLeft(WindageStart);
	}
	else
	{
		PointOfImpactRight(WindageStart);
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

void AFPSTemplate_SightBase::ReturnToZeroElevation()
{
	if (bCanBeZeroed)
	{
		if (IsValid(SceneCapture))
		{
			SceneCapture->ReturnToZeroElevation();
		}
		else
		{
			DotZero.Elevation = 0.0f;
			CurrentDotElevation = StartingDotElevation;
			for (const FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
			{
				if (Reticle.DynamicReticleMaterial)
				{
					Reticle.DynamicReticleMaterial->SetVectorParameterValue(ReticleZeroName, FLinearColor(CurrentDotWindage, CurrentDotElevation, 0.0f));
				}
			}
		}
		OnReturnToZeroElevation();
	}
}

void AFPSTemplate_SightBase::ReturnToZeroWindage()
{
	if (bCanBeZeroed)
	{
		if (IsValid(SceneCapture))
		{
			SceneCapture->ReturnToZeroWindage();
		}
		else
		{
			DotZero.Windage = 0.0f;
			CurrentDotWindage = StartingDotWindage;
			for (const FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
			{
				if (Reticle.DynamicReticleMaterial)
				{
					Reticle.DynamicReticleMaterial->SetVectorParameterValue(ReticleZeroName, FLinearColor(CurrentDotWindage, CurrentDotElevation, 0.0f));
				}
			}
		}
		OnReturnToZeroWindage();
	}
}

void AFPSTemplate_SightBase::SetNewZeroElevation()
{
	if (bCanBeZeroed)
	{
		if (IsValid(SceneCapture))
		{
			SceneCapture->SetNewZeroElevation();
		}
		else
		{
			DotZero.Elevation = 0.0f;
			StartingDotElevation = CurrentDotElevation;
		}
		OnReturnToZeroElevation();
	}
}

void AFPSTemplate_SightBase::SetNewZeroWindage()
{
	if (bCanBeZeroed)
	{
		if (IsValid(SceneCapture))
		{
			SceneCapture->SetNewZeroWindage();
		}
		else
		{
			DotZero.Windage = 0.0f;
			StartingDotWindage = CurrentDotWindage;
		}
		OnReturnToZeroWindage();
	}
}

void AFPSTemplate_SightBase::PointOfImpactUp(uint8 Clicks)
{
	if (bCanBeZeroed)
	{
		FVector2D AdjustmentMade;
		if (IsValid(SceneCapture))
		{
			AdjustmentMade = SceneCapture->PointOfImpactUp(Clicks);
		}
		else
		{
			AdjustmentMade.X = CurrentDotElevation;
			CurrentDotElevation -= RedDotZeroAmount * Clicks;
			DotZero.Elevation -= RedDotZeroAmount * Clicks;
			AdjustmentMade.Y = CurrentDotElevation;
			for (const FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
			{
				if (Reticle.DynamicReticleMaterial)
				{
					Reticle.DynamicReticleMaterial->SetVectorParameterValue(ReticleZeroName, FLinearColor(CurrentDotWindage, CurrentDotElevation, 0.0f));
				}
			}
		}
		OnElevationChanged(AdjustmentMade.X, AdjustmentMade.Y, Clicks);
	}
}

void AFPSTemplate_SightBase::PointOfImpactDown(uint8 Clicks)
{
	if (bCanBeZeroed)
	{
		FVector2D AdjustmentMade;
		if (IsValid(SceneCapture))
		{
			AdjustmentMade = SceneCapture->PointOfImpactDown(Clicks);
		}
		else
		{
			AdjustmentMade.X = CurrentDotElevation;
			CurrentDotElevation += RedDotZeroAmount * Clicks;
			DotZero.Elevation += RedDotZeroAmount * Clicks;
			AdjustmentMade.Y = CurrentDotElevation;
			for (const FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
			{
				if (Reticle.DynamicReticleMaterial)
				{
					Reticle.DynamicReticleMaterial->SetVectorParameterValue(ReticleZeroName, FLinearColor(CurrentDotWindage, CurrentDotElevation, 0.0f));
				}
			}
		}
		OnElevationChanged(AdjustmentMade.X, AdjustmentMade.Y, Clicks);
	}
}

void AFPSTemplate_SightBase::PointOfImpactLeft(uint8 Clicks)
{
	if (bCanBeZeroed)
	{
		FVector2D AdjustmentMade;
		if (IsValid(SceneCapture))
		{
			AdjustmentMade = SceneCapture->PointOfImpactLeft(Clicks);
		}
		else
		{
			AdjustmentMade.X = CurrentDotWindage;
			CurrentDotWindage -= RedDotZeroAmount * Clicks;
			DotZero.Windage -= RedDotZeroAmount * Clicks;
			AdjustmentMade.Y = CurrentDotWindage;
			for (const FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
			{
				if (Reticle.DynamicReticleMaterial)
				{
					Reticle.DynamicReticleMaterial->SetVectorParameterValue(ReticleZeroName, FLinearColor(CurrentDotWindage, CurrentDotElevation, 0.0f));
				}
			}
		}
		OnWindageChanged(AdjustmentMade.X, AdjustmentMade.Y, Clicks);
	}
}

void AFPSTemplate_SightBase::PointOfImpactRight(uint8 Clicks)
{
	if (bCanBeZeroed)
	{
		FVector2D AdjustmentMade;
		if (IsValid(SceneCapture))
		{
			AdjustmentMade = SceneCapture->PointOfImpactRight(Clicks);
		}
		else
		{
			AdjustmentMade.X = CurrentDotWindage;
			CurrentDotWindage += RedDotZeroAmount * Clicks;
			DotZero.Windage += RedDotZeroAmount * Clicks;
			AdjustmentMade.Y = CurrentDotWindage;
			for (const FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
			{
				if (Reticle.DynamicReticleMaterial)
				{
					Reticle.DynamicReticleMaterial->SetVectorParameterValue(ReticleZeroName, FLinearColor(CurrentDotWindage, CurrentDotElevation, 0.0f));
				}
			}
		}
		OnWindageChanged(AdjustmentMade.X, AdjustmentMade.Y, Clicks);
	}
}

FSightZero AFPSTemplate_SightBase::GetSightZero() const
{
	if (IsValid(SceneCapture))
	{
		return SceneCapture->GetSightZero();
	}
	return DotZero;
}

FTransform AFPSTemplate_SightBase::GetSightSocketTransform_Implementation()
{
	if (IsValid(Magnifier) && Magnifier->IsFullyFlipped() && !Magnifier->IsFlippedOut() && Magnifier->GetSightInfront() == this)
	{
		return Magnifier->GetAimSocketTransform();
	}
	return Super::GetAimSocketTransform();
}

void AFPSTemplate_SightBase::GetCameraSettings_Implementation(FAimCameraSettings& OutCameraSettings)
{
	if (IsValid(Magnifier) && !Magnifier->IsFlippedOut() && Magnifier->GetSightInfront() == this)
	{
		if (Magnifier->Implements<UFPSTemplate_AimInterface>())
		{
			Execute_GetCameraSettings(Magnifier, OutCameraSettings);
			return;
		}
	}
	OutCameraSettings = CameraSettings;
}

void AFPSTemplate_SightBase::ZoomOptic_Implementation(bool bZoom)
{
	if (IsValid(SceneCapture))
	{
		if (bZoom)
		{
			SceneCapture->ZoomIn();
		}
		else
		{
			SceneCapture->ZoomOut();
		}
	}
}

float AFPSTemplate_SightBase::GetMagnification() const
{
	if (IsValid(Magnifier) && !Magnifier->IsFlippedOut() && Magnifier->GetSightInfront() == this)
	{
		return Magnifier->GetMagnification();
	}
	if (IsValid(SceneCapture))
	{
		return SceneCapture->GetMagnification();
	}
	return 1.0f;
}

void AFPSTemplate_SightBase::DisableRenderTarget(bool Disable)
{
	if (IsValid(Magnifier) && !Magnifier->IsFlippedOut() && Magnifier->GetSightInfront() == this)
	{
		Magnifier->DisableRenderTarget(Disable);
		return;
	}
	if (IsValid(SceneCapture))
	{
		if (!Disable)
		{
			SceneCapture->StartCapture();
		}
		else
		{
			SceneCapture->StopCapture();
		}
	}
}

void AFPSTemplate_SightBase::CycleReticle()
{
	if (IsValid(SceneCapture))
	{
		SceneCapture->CycleReticle();
		return;
	}
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
	if (IsValid(SceneCapture))
	{
		SceneCapture->SetReticle(Index);
		return;
	}
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