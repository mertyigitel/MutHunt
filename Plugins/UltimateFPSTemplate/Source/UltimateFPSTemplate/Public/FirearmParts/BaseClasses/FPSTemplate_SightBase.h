// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/FPS_SceneCaptureComponent2D.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "Interfaces/FPSTemplate_AimInterface.h"
#include "FPSTemplate_SightBase.generated.h"

class UMaterialInstance;
class AFPSTemplate_MagnifierBase;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_SightBase : public AFPSTemplate_PartBase, public IFPSTemplate_AimInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplate_SightBase();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	FReticleSettings ReticleSettings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FAimCameraSettings CameraSettings;
	// Used to blend between different state machines in the anim graph using an integer
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Animation")
	int32 AnimationIndex;
	// Used to blend between different state machines in the anim graph using gameplay tags
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	FGameplayTag AnimationGameplayTag;
	// How much faster to ADS with this optic
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Animation")
	float AimInterpolationMultiplier;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	float RotationLagInterpolationMultiplier;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	float MovementLagInterpolationMultiplier;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Animation")
	FName GripSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	FRotator HeadAimRotation;
	// Whether or not this optic can have its zero adjusted (point of impact shifts)
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Zero")
	bool bCanBeZeroed;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Zero", meta = (EditCondition = "bCanBeZeroed"))
	bool bStartWithRandomZero;
	// What is the max amount of clicks for the random elevation zero
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Zero", meta = (EditCondition = "bCanBeZeroed"))
	uint8 RandomMaxElevationStartClicks;
	// What is the max amount of clicks for the random windage zero
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Zero", meta = (EditCondition = "bCanBeZeroed"))
	uint8 RandomMaxWindageStartClicks;
	
	uint8 ReticleIndex;
	uint8 ReticleBrightnessIndex;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Zero")
	float MilliradianAdjustment;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Zero")
	float MOAAdjustment;

	const float MilliradianClick = 0.1f;
	const float MOAClick = 0.25f;
	float RedDotZeroAmount;
	float CurrentDotElevation, CurrentDotWindage, StartingDotElevation, StartingDotWindage;
	FSightZero DotZero;
	FName ReticleZeroName = FName("ReticleZero");

	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Default")
	AFPSTemplate_MagnifierBase* Magnifier;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	UMaterialParameterCollection* MPC
	;
	UMaterialParameterCollectionInstance* MPCInstance;
	
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;

	void SetRandomZero();
	UFPS_SceneCaptureComponent2D* SceneCapture;

	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Zero")
	void OnElevationChanged(float OldAdjustment, float NewAdjustment, int32 Clicks);
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Zero")
	void OnWindageChanged(float OldAdjustment, float NewAdjustment, int32 Clicks);
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Zero")
	void OnReturnToZeroElevation();
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Zero")
	void OnReturnToZeroWindage();
	
public:
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Sockets")
	FName& GetAimSocket() { return AimSocket; }

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Magnification")
	virtual float GetMagnification() const;

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	virtual void CycleReticle();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	virtual void SetReticle(uint8 Index);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	void IncreaseBrightness();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	void DecreaseBrightness();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	void SetReticleBrightness(uint8 Index);

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void ReturnToZeroElevation();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void ReturnToZeroWindage();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void SetNewZeroElevation();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void SetNewZeroWindage();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void PointOfImpactUp(uint8 Clicks = 1);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void PointOfImpactDown(uint8 Clicks = 1);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void PointOfImpactLeft(uint8 Clicks = 1);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void PointOfImpactRight(uint8 Clicks = 1);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Zero")
	FSightZero GetSightZero() const;
	
	// Aim Interface defaults
	virtual void SetMagnifier_Implementation(AFPSTemplate_MagnifierBase* INMagnifier) override { Magnifier = INMagnifier; }
	virtual AFPSTemplate_MagnifierBase* GetMagnifier_Implementation() override { return Magnifier; }
	virtual FTransform GetDefaultSightSocketTransform_Implementation() override { return IFPSTemplate_AimInterface::Execute_GetSightSocketTransform(this); }
	virtual FTransform GetSightSocketTransform_Implementation() override;
	virtual FTransform GetAimSocketTransform() override { return IFPSTemplate_AimInterface::Execute_GetSightSocketTransform(this); }
	virtual int32 GetAnimationIndex_Implementation() const override { return AnimationIndex; }
	virtual FGameplayTag GetAnimationGameplayTag_Implementation() const override { return AnimationGameplayTag; }
	virtual float GetAimInterpolationMultiplier_Implementation() override { return AimInterpolationMultiplier; }
	virtual float GetRotationLagInterpolationMultiplier_Implementation() override { return RotationLagInterpolationMultiplier; }
	virtual float GetMovementLagInterpolationMultiplier_Implementation() override { return MovementLagInterpolationMultiplier; }
	virtual AFPSTemplate_SightBase* GetActiveSight_Implementation() override { return this; }
	virtual void DisableRenderTargets_Implementation(bool Disable) override { DisableRenderTarget(Disable); }
	virtual FRotator GetHeadRotation_Implementation() const override { return HeadAimRotation; }
	virtual FName GetGripSocketName_Implementation() const override { return GripSocket; }
	virtual float GetCurrentMagnification_Implementation() const override { return GetMagnification(); }
	virtual void GetCameraSettings_Implementation(FAimCameraSettings& OutCameraSettings) override;
	virtual void ZoomOptic_Implementation(bool bZoom) override;

	virtual void DisableRenderTarget(bool Disable) override;
};
