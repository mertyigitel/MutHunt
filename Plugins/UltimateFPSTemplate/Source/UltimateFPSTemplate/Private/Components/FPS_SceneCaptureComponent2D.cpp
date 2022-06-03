// Copyright 2022, Dakota Dawe, All rights reserved


#include "Components/FPS_SceneCaptureComponent2D.h"

#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

constexpr float FOVDivider = 15.0f;

UFPS_SceneCaptureComponent2D::UFPS_SceneCaptureComponent2D(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(false);
	
	bHiddenInGame = false;
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	bAlwaysPersistRenderingState = true;
	PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;
	
	ShowFlags.DynamicShadows = true;
	ShowFlags.AmbientOcclusion = false;
	ShowFlags.AmbientCubemap = false;
	ShowFlags.DistanceFieldAO = false;
	ShowFlags.LightFunctions = false;
	ShowFlags.LightShafts = false;
	ShowFlags.ReflectionEnvironment = true;
	ShowFlags.ScreenSpaceReflections = false;
	ShowFlags.TexturedLightProfiles = false;
	ShowFlags.VolumetricFog = false;
	ShowFlags.MotionBlur = 0;

	ReticleIndex = 0;
	ReticleBrightnessIndex = 0;
	
	TurretAdjustmentType = EScopeAdjustment::MRAD;

	MilliradianAdjustment = 0.005625f;
	MOAAdjustment = 0.004167f;

	RenderTargetMaterialVarName = FName("RenderTarget");
}

void UFPS_SceneCaptureComponent2D::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(false);
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	if (!Optimization.bOverrideCaptureEveryFrame)
	{
		SetComponentTickInterval(1.0f / Optimization.RefreshRate);
	}
	SetupRenderTarget();
}

void UFPS_SceneCaptureComponent2D::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (RenderTarget.IsValid())
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	}
}

void UFPS_SceneCaptureComponent2D::SetupRenderTarget()
{
	if (!RenderTarget.IsValid())
	{
		RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), RenderTargetSize.Width, RenderTargetSize.Height, ETextureRenderTargetFormat::RTF_RGBA16f);
		TextureTarget = RenderTarget.Get();
		StartingSceneCaptureRot = GetRelativeRotation();
	}
}

void UFPS_SceneCaptureComponent2D::HandleReticleZoom()
{
	if (MagnificationSettings.bIsFirstFocalPlane)
	{
		for (const FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
		{
			if (IsValid(Reticle.DynamicReticleMaterial))
			{
				Reticle.DynamicReticleMaterial->SetScalarParameterValue(FName("ReticleSize"), (MagnificationSettings.CurrentMagnification / MagnificationSettings.DecreaseReticleScaleAmount) * Reticle.ReticleSize);
			}
		}
	}
}

void UFPS_SceneCaptureComponent2D::StartCapture()
{
	if (!OwningMesh.IsValid())
	{
		SetupRenderTarget();
	}

	if (OwningMesh.IsValid() && Optimization.bClearScopeWithMaterial && ReticleIndex < ReticleSettings.ReticleMaterials.Num() && ReticleSettings.ReticleMaterials[ReticleIndex].DynamicReticleMaterial)
	{
		OwningMesh->SetMaterial(ReticleSettings.ReticleMaterialIndex, ReticleSettings.ReticleMaterials[ReticleIndex].DynamicReticleMaterial);
	}
	if (!Optimization.bOverrideCaptureEveryFrame && !bCaptureEveryFrame)
	{
		CaptureScene();
		SetComponentTickInterval(1.0f / Optimization.RefreshRate);
	}
	
	bCaptureEveryFrame = true;
	bCaptureOnMovement = true;
	SetComponentTickEnabled(true);
}

void UFPS_SceneCaptureComponent2D::StopCapture()
{
	if (!Optimization.bOverrideCaptureEveryFrame && !Optimization.bDisableWhenNotAiming)
	{
		SetComponentTickInterval(1.0f / Optimization.NotAimingRefreshRate);
	}
	else if (OwningMesh.IsValid())
	{
		if (Optimization.bClearScopeWithColor)
		{
			UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), TextureTarget, Optimization.ClearedColor);
		}
		else if (Optimization.bClearScopeWithMaterial && Optimization.ClearedScopeMaterial)
		{
			OwningMesh->SetMaterial(ReticleSettings.ReticleMaterialIndex, Optimization.ClearedScopeMaterial);
		}
	}

	SetComponentTickEnabled(false);
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
}

void UFPS_SceneCaptureComponent2D::SetOwningMesh(UMeshComponent* Mesh)
{
	OwningMesh = Mesh;
	for (FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
	{
		if (Reticle.ReticleMaterial)
		{
			Reticle.DynamicReticleMaterial = UMaterialInstanceDynamic::Create(Reticle.ReticleMaterial, this);
			Reticle.DynamicReticleMaterial->GetScalarParameterValue(FName("ReticleSize"), Reticle.ReticleSize);
			Reticle.DynamicReticleMaterial->SetTextureParameterValue(RenderTargetMaterialVarName, RenderTarget.Get());
		}
	}
	SetReticle(ReticleIndex);
	if (MagnificationSettings.Magnifications.Num())
	{
		MagnificationSettings.CurrentMagnification = MagnificationSettings.Magnifications[0];
	}
	HandleReticleZoom();
	FOVAngle = FOVDivider / MagnificationSettings.CurrentMagnification;
}

void UFPS_SceneCaptureComponent2D::ZoomIn()
{
	if (++MagnificationSettings.MagnificationIndex < MagnificationSettings.Magnifications.Num())
	{
		MagnificationSettings.CurrentMagnification = MagnificationSettings.Magnifications[MagnificationSettings.MagnificationIndex];

		HandleReticleZoom();
		
		for (const FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
		{
			if (IsValid(Reticle.DynamicReticleMaterial))
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
		
		FOVAngle = FOVDivider / MagnificationSettings.CurrentMagnification;
	}
	else
	{
		MagnificationSettings.MagnificationIndex = MagnificationSettings.Magnifications.Num() - 1;
	}
}

void UFPS_SceneCaptureComponent2D::ZoomOut()
{
	if (MagnificationSettings.MagnificationIndex - 1 >= 0)
	{
		--MagnificationSettings.MagnificationIndex;
		MagnificationSettings.CurrentMagnification = MagnificationSettings.Magnifications[MagnificationSettings.MagnificationIndex];

		HandleReticleZoom();
		
		for (const FReticleMaterial& Reticle : ReticleSettings.ReticleMaterials)
		{
			if (IsValid(Reticle.DynamicReticleMaterial))
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
		
		FOVAngle = FOVDivider / MagnificationSettings.CurrentMagnification;
	}
	else
	{
		MagnificationSettings.MagnificationIndex = 0;
	}
}

float UFPS_SceneCaptureComponent2D::GetMagnification() const
{
	return MagnificationSettings.CurrentMagnification;
}

float UFPS_SceneCaptureComponent2D::GetMagnificationSensitivity() const
{
	return 1.0f / MagnificationSettings.CurrentMagnification;
}

void UFPS_SceneCaptureComponent2D::ReturnToZeroElevation()
{
	SightZero.Elevation = 0.0f;
	FRotator CurrentRot = GetRelativeRotation();
	CurrentRot.Pitch = StartingSceneCaptureRot.Pitch;
	SetRelativeRotation(CurrentRot);
}

void UFPS_SceneCaptureComponent2D::ReturnToZeroWindage()
{
	SightZero.Windage = 0.0f;
	FRotator CurrentRot = GetRelativeRotation();
	CurrentRot.Yaw = StartingSceneCaptureRot.Yaw;
	SetRelativeRotation(CurrentRot);
}

void UFPS_SceneCaptureComponent2D::SetNewZeroElevation()
{
	SightZero.Elevation = 0.0f;
	StartingSceneCaptureRot.Pitch = GetRelativeRotation().Pitch;
}

void UFPS_SceneCaptureComponent2D::SetNewZeroWindage()
{
	SightZero.Windage = 0.0f;
	StartingSceneCaptureRot.Yaw = GetRelativeRotation().Yaw;
}

FVector2D UFPS_SceneCaptureComponent2D::PointOfImpactUp(uint8 Clicks)
{
	FVector2D AdjustmentMade = FVector2D(SightZero.Elevation, 0.0f);
	if (Clicks > 0)
	{
		if (TurretAdjustmentType == EScopeAdjustment::MRAD)
		{
			OnElevationChanged.Broadcast(SightZero.Elevation, SightZero.Elevation + MilliradianClick);
			SightZero.Elevation += MilliradianClick * Clicks;
			FRotator CurrentRot = GetRelativeRotation();
			CurrentRot.Pitch -= MilliradianAdjustment * Clicks;
			SetRelativeRotation(CurrentRot);
		}
		else
		{
			OnElevationChanged.Broadcast(SightZero.Elevation, SightZero.Elevation + MOAClick);
			SightZero.Elevation += MOAClick * Clicks;
			FRotator CurrentRot = GetRelativeRotation();
			CurrentRot.Pitch -= MOAAdjustment * Clicks;
			SetRelativeRotation(CurrentRot);
		}
		AdjustmentMade.Y = SightZero.Elevation;
	}
	return AdjustmentMade;
}

FVector2D UFPS_SceneCaptureComponent2D::PointOfImpactDown(uint8 Clicks)
{
	FVector2D AdjustmentMade = FVector2D(SightZero.Elevation, 0.0f);
	if (Clicks > 0)
	{
		if (TurretAdjustmentType == EScopeAdjustment::MRAD)
		{
			OnElevationChanged.Broadcast(SightZero.Elevation, SightZero.Elevation - MilliradianClick);
			SightZero.Elevation -= MilliradianClick * Clicks;
			FRotator CurrentRot = GetRelativeRotation();
			CurrentRot.Pitch += MilliradianAdjustment * Clicks;
			SetRelativeRotation(CurrentRot);
		}
		else
		{
			OnElevationChanged.Broadcast(SightZero.Elevation, SightZero.Elevation - MOAClick);
			SightZero.Elevation -= MOAClick * Clicks;
			FRotator CurrentRot = GetRelativeRotation();
			CurrentRot.Pitch += MOAAdjustment * Clicks;
			SetRelativeRotation(CurrentRot);
		}
		AdjustmentMade.Y = SightZero.Elevation;
	}
	return AdjustmentMade;
}

FVector2D UFPS_SceneCaptureComponent2D::PointOfImpactLeft(uint8 Clicks)
{
	FVector2D AdjustmentMade = FVector2D(SightZero.Windage, 0.0f);
	if (Clicks > 0)
	{
		if (TurretAdjustmentType == EScopeAdjustment::MRAD)
		{
			OnWindageChanged.Broadcast(SightZero.Windage, SightZero.Windage - MilliradianClick);
			SightZero.Windage -= MilliradianClick * Clicks;
			FRotator CurrentRot = GetRelativeRotation();
			CurrentRot.Yaw += MilliradianAdjustment * Clicks;
			SetRelativeRotation(CurrentRot);
		}
		else
		{
			OnWindageChanged.Broadcast(SightZero.Windage, SightZero.Windage - MOAClick);
			SightZero.Windage -= MOAClick * Clicks;
			FRotator CurrentRot = GetRelativeRotation();
			CurrentRot.Yaw += MOAAdjustment * Clicks;
			SetRelativeRotation(CurrentRot);
		}
		AdjustmentMade.Y = SightZero.Windage;
	}
	return AdjustmentMade;
}

FVector2D UFPS_SceneCaptureComponent2D::PointOfImpactRight(uint8 Clicks)
{
	FVector2D AdjustmentMade = FVector2D(SightZero.Windage, 0.0f);
	if (Clicks > 0)
	{
		if (TurretAdjustmentType == EScopeAdjustment::MRAD)
		{
			OnWindageChanged.Broadcast(SightZero.Windage, SightZero.Windage + MilliradianClick);
			SightZero.Windage += MilliradianClick * Clicks;
			FRotator CurrentRot = GetRelativeRotation();
			CurrentRot.Yaw -= MilliradianAdjustment * Clicks;
			SetRelativeRotation(CurrentRot);
		}
		else
		{
			OnWindageChanged.Broadcast(SightZero.Windage, SightZero.Windage + MOAClick);
			SightZero.Windage += MOAClick * Clicks;
			FRotator CurrentRot = GetRelativeRotation();
			CurrentRot.Yaw -= MOAAdjustment * Clicks;
			SetRelativeRotation(CurrentRot);
		}
		AdjustmentMade.Y = SightZero.Windage;
	}
	return AdjustmentMade;
}

void UFPS_SceneCaptureComponent2D::CycleReticle()
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

void UFPS_SceneCaptureComponent2D::SetReticle(uint8 Index)
{
	if (OwningMesh.IsValid() && Index < ReticleSettings.ReticleMaterials.Num() && ReticleSettings.ReticleMaterials[Index].DynamicReticleMaterial)
	{
		ReticleIndex = Index;
		OwningMesh->SetMaterial(ReticleSettings.ReticleMaterialIndex, ReticleSettings.ReticleMaterials[ReticleIndex].DynamicReticleMaterial);
	}
}

void UFPS_SceneCaptureComponent2D::SetReticleSettings(FReticleSettings& INReticleSettings)
{
	ReticleSettings = INReticleSettings;
}
