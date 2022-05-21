// Copyright 2021, Dakota Dawe, All rights reserved

#include "FirearmParts/BaseClasses/FPSTemplate_SightMagnifiedRTBase.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Components/FPSTemplate_CharacterComponent.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInterface.h"

constexpr float FOVDivider = 15.0f;

AFPSTemplate_SightMagnifiedRTBase::AFPSTemplate_SightMagnifiedRTBase()
{
	PrimaryActorTick.bCanEverTick = true;
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;
	
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	SceneCapture->SetupAttachment(RootComponent);
	SceneCapture->bHiddenInGame = true;
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->bAlwaysPersistRenderingState = true;
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;
	
	SceneCapture->ShowFlags.DynamicShadows = true;
	SceneCapture->ShowFlags.AmbientOcclusion = false;
	SceneCapture->ShowFlags.AmbientCubemap = false;
	SceneCapture->ShowFlags.DistanceFieldAO = false;
	SceneCapture->ShowFlags.LightFunctions = false;
	SceneCapture->ShowFlags.LightShafts = false;
	SceneCapture->ShowFlags.ReflectionEnvironment = false;
	SceneCapture->ShowFlags.ScreenSpaceReflections = false;
	SceneCapture->ShowFlags.TexturedLightProfiles = false;
	SceneCapture->ShowFlags.VolumetricFog = false;
	SceneCapture->ShowFlags.MotionBlur = 0;
	
	bHasRenderTarget = true;

	AnimationIndex = -1;
	AimInterpolationMultiplier = 50.0f;
	GripSocket = FName("cc_FirearmGrip");
	HeadAimRotation = FRotator(0.0f, 0.0f, 0.0f);
}

void AFPSTemplate_SightMagnifiedRTBase::BeginPlay()
{
	Super::BeginPlay();
	
	DisableRenderTarget(true);
	HandleReticleZoom();
}

void AFPSTemplate_SightMagnifiedRTBase::PostInitProperties()
{
	Super::PostInitProperties();
	
	SceneCapture->SetHiddenInGame(false);
	StartingSceneCaptureRot = SceneCapture->GetRelativeRotation();
	
	if (MagnificationSettings.Magnifications.Num())
	{
		MagnificationSettings.CurrentMagnification = MagnificationSettings.Magnifications[MagnificationSettings.MagnificationIndex];
	}
	else
	{
		MagnificationSettings.CurrentMagnification = 1.0f;
	}
	SetFOVAngle();
	
	SetActorTickInterval(1 / ScopeOptimization.ScopeRefreshRate);
}

void AFPSTemplate_SightMagnifiedRTBase::HandleRenderTarget()
{
	if (PartType == EPartType::Magnifier)
	{
		if (UFPSTemplate_CharacterComponent* CharacterComponent = GetOwningCharacterComponent())
		{
			if (CharacterComponent->IsLocallyControlled())
			{
				DisableRenderTarget(false);
			}
		}
	}
	else
	{
		DisableRenderTarget(true);
	}
}

void AFPSTemplate_SightMagnifiedRTBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SceneCapture->CaptureScene();
}

void AFPSTemplate_SightMagnifiedRTBase::SetFOVAngle() const
{
	SceneCapture->FOVAngle = FOVDivider / MagnificationSettings.CurrentMagnification;
}

void AFPSTemplate_SightMagnifiedRTBase::HandleReticleZoom()
{
	if (MagnificationSettings.bIsFirstFocalPlane)
	{
		for (const FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
		{
			if (Reticle.DynamicReticleMaterial)
			{
				Reticle.DynamicReticleMaterial->SetScalarParameterValue(FName("ReticleSize"), (MagnificationSettings.CurrentMagnification / MagnificationSettings.DecreaseReticleScaleAmount) * Reticle.ReticleSize);
			}
		}
	}
}

void AFPSTemplate_SightMagnifiedRTBase::ZoomIn()
{
	if (++MagnificationSettings.MagnificationIndex < MagnificationSettings.Magnifications.Num())
	{
		const float OldMagnification = MagnificationSettings.CurrentMagnification;
		MagnificationSettings.CurrentMagnification = MagnificationSettings.Magnifications[MagnificationSettings.MagnificationIndex];

		HandleReticleZoom();
		
		for (const FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
		{
			if (Reticle.DynamicReticleMaterial)
			{
				float EyeboxRange;
				Reticle.DynamicReticleMaterial->GetScalarParameterValue(FName("EyeboxRange"), EyeboxRange);
				if (EyeboxRange)
				{
					EyeboxRange -= 150.0f;
					Reticle.DynamicReticleMaterial->SetScalarParameterValue(FName("EyeboxRange"), EyeboxRange);
				}
			}
		}
		
		OpticZoomedIn(OldMagnification);
		SetFOVAngle();
	}
	else
	{
		MagnificationSettings.MagnificationIndex = MagnificationSettings.Magnifications.Num() - 1;
	}
}

void AFPSTemplate_SightMagnifiedRTBase::ZoomOut()
{
	if (MagnificationSettings.MagnificationIndex - 1 >= 0)
	{
		const float OldMagnification = MagnificationSettings.CurrentMagnification;
		--MagnificationSettings.MagnificationIndex;
		MagnificationSettings.CurrentMagnification = MagnificationSettings.Magnifications[MagnificationSettings.MagnificationIndex];

		HandleReticleZoom();
		
		for (const FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
		{
			if (Reticle.DynamicReticleMaterial)
			{
				float EyeboxRange;
				Reticle.DynamicReticleMaterial->GetScalarParameterValue(FName("EyeboxRange"), EyeboxRange);
				if (EyeboxRange)
				{
					EyeboxRange += 150.0f;
					Reticle.DynamicReticleMaterial->SetScalarParameterValue(FName("EyeboxRange"), EyeboxRange);
				}
			}
		}
		
		OpticZoomedIn(OldMagnification);
		SetFOVAngle();
	}
	else
	{
		MagnificationSettings.MagnificationIndex = 0;
	}
}

void AFPSTemplate_SightMagnifiedRTBase::ReturnToZeroElevation()
{
	OnReturnToZeroElevation();
	SightZero.Elevation = 0.0f;
	FRotator CurrentRot = SceneCapture->GetRelativeRotation();
	CurrentRot.Pitch = StartingSceneCaptureRot.Pitch;
	SceneCapture->SetRelativeRotation(CurrentRot);
}

void AFPSTemplate_SightMagnifiedRTBase::ReturnToZeroWindage()
{
	OnReturnToZeroWindage();
	SightZero.Windage = 0.0f;
	FRotator CurrentRot = SceneCapture->GetRelativeRotation();
	CurrentRot.Yaw = StartingSceneCaptureRot.Yaw;
	SceneCapture->SetRelativeRotation(CurrentRot);
}

void AFPSTemplate_SightMagnifiedRTBase::SetNewZeroElevation()
{
	OnReturnToZeroElevation();
	SightZero.Elevation = 0.0f;
	StartingSceneCaptureRot.Pitch = SceneCapture->GetRelativeRotation().Pitch;
}

void AFPSTemplate_SightMagnifiedRTBase::SetNewZeroWindage()
{
	OnReturnToZeroWindage();
	SightZero.Windage = 0.0f;
	StartingSceneCaptureRot.Yaw = SceneCapture->GetRelativeRotation().Yaw;
}

void AFPSTemplate_SightMagnifiedRTBase::PointOfImpactUp()
{
	if (TurretAdjustmentType == EScopeAdjustment::MRAD)
	{
		OnElevationChanged(SightZero.Elevation, SightZero.Elevation + MilliradianClick);
		SightZero.Elevation += MilliradianClick;
		FRotator CurrentRot = SceneCapture->GetRelativeRotation();
		CurrentRot.Pitch -= MilliradianAdjustment;
		SceneCapture->SetRelativeRotation(CurrentRot);
	}
	else
	{
		OnElevationChanged(SightZero.Elevation, SightZero.Elevation + MOAClick);
		SightZero.Elevation += MOAClick;
		FRotator CurrentRot = SceneCapture->GetRelativeRotation();
		CurrentRot.Pitch -= MOAAdjustment;
		SceneCapture->SetRelativeRotation(CurrentRot);
	}
}

void AFPSTemplate_SightMagnifiedRTBase::PointOfImpactDown()
{
	if (TurretAdjustmentType == EScopeAdjustment::MRAD)
	{
		OnElevationChanged(SightZero.Elevation, SightZero.Elevation - MilliradianClick);
		SightZero.Elevation -= MilliradianClick;
		FRotator CurrentRot = SceneCapture->GetRelativeRotation();
		CurrentRot.Pitch += MilliradianAdjustment;
		SceneCapture->SetRelativeRotation(CurrentRot);
	}
	else
	{
		OnElevationChanged(SightZero.Elevation, SightZero.Elevation - MOAClick);
		SightZero.Elevation -= MOAClick;
		FRotator CurrentRot = SceneCapture->GetRelativeRotation();
		CurrentRot.Pitch += MOAAdjustment;
		SceneCapture->SetRelativeRotation(CurrentRot);
	}
}

void AFPSTemplate_SightMagnifiedRTBase::PointOfImpactLeft()
{
	if (TurretAdjustmentType == EScopeAdjustment::MRAD)
	{
		OnWindageChanged(SightZero.Windage, SightZero.Windage - MilliradianClick);
		SightZero.Windage -= MilliradianClick;
		FRotator CurrentRot = SceneCapture->GetRelativeRotation();
		CurrentRot.Yaw += MilliradianAdjustment;
		SceneCapture->SetRelativeRotation(CurrentRot);
	}
	else
	{
		OnElevationChanged(SightZero.Elevation, SightZero.Elevation - MOAClick);
		SightZero.Elevation -= MOAClick;
		FRotator CurrentRot = SceneCapture->GetRelativeRotation();
		CurrentRot.Pitch += MOAAdjustment;
		SceneCapture->SetRelativeRotation(CurrentRot);
	}
}

void AFPSTemplate_SightMagnifiedRTBase::PointOfImpactRight()
{
	if (TurretAdjustmentType == EScopeAdjustment::MRAD)
	{
		OnWindageChanged(SightZero.Windage, SightZero.Windage + MilliradianClick);
		SightZero.Windage += MilliradianClick;
		FRotator CurrentRot = SceneCapture->GetRelativeRotation();
		CurrentRot.Yaw -= MilliradianAdjustment;
		SceneCapture->SetRelativeRotation(CurrentRot);
	}
	else
	{
		OnElevationChanged(SightZero.Elevation, SightZero.Elevation - MOAClick);
		SightZero.Elevation -= MOAClick;
		FRotator CurrentRot = SceneCapture->GetRelativeRotation();
		CurrentRot.Pitch += MOAAdjustment;
		SceneCapture->SetRelativeRotation(CurrentRot);
	}
}

void AFPSTemplate_SightMagnifiedRTBase::DisableRenderTarget(bool Disable)
{
	if (!ScopeOptimization.bDisableWhenNotAiming)
	{
		if (Disable)
		{
			SetActorTickInterval(1 / ScopeOptimization.NotAimingRefreshRate);
		}
		else
		{
			SceneCapture->CaptureScene();
			SetActorTickInterval(1 / ScopeOptimization.ScopeRefreshRate);
		}
	}
	else if (PartMesh.IsValid())
	{
		if (Disable)
		{
			bool bClearedWithMaterial = false;
			if (ScopeOptimization.bClearScopeWithMaterial && ScopeOptimization.ClearedScopeMaterial)
			{
				PartMesh->SetMaterial(ReticleSettings.ReticleMaterialIndex, ScopeOptimization.ClearedScopeMaterial);
				bClearedWithMaterial = true;
			}
			if (ScopeOptimization.bClearScopeWithColor || !bClearedWithMaterial)
			{
				UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), SceneCapture->TextureTarget, ScopeOptimization.ScopeClearedColor);
			}
		}
		else
		{
			if (ScopeOptimization.bClearScopeWithMaterial && ReticleIndex < ReticleSettings.ReticleMaterials.Num() && ReticleSettings.ReticleMaterials[ReticleIndex].DynamicReticleMaterial)
			{
				PartMesh->SetMaterial(ReticleSettings.ReticleMaterialIndex, ReticleSettings.ReticleMaterials[ReticleIndex].DynamicReticleMaterial);
			}
		}
		SetActorTickEnabled(!Disable);
	}
}

void AFPSTemplate_SightMagnifiedRTBase::ZoomOptic(bool bZoom)
{
	if (bZoom)
	{
		ZoomIn();
	}
	else
	{
		ZoomOut();
	}
}
