// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "Interfaces/FPSTemplate_AimInterface.h"
#include "FPSTemplate_SightBase.generated.h"

class UMaterialInstance;

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
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	EScopeAdjustment TurretAdjustmentType;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Animation")
	int32 AnimationIndex;
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
	
	uint8 ReticleIndex;
	uint8 ReticleBrightnessIndex;
	FSightZero SightZero;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Zero")
	float MilliradianAdjustment;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Zero")
	float MOAAdjustment;

	const float MilliradianClick = 0.1f;
	const float MOAClick = 0.25f;
	
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Zero")
	void OnElevationChanged(float OldAdjustment, float NewAdjustment);
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Zero")
	void OnWindageChanged(float OldAdjustment, float NewAdjustment);
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Zero")
	void OnReturnToZeroElevation();
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Zero")
	void OnReturnToZeroWindage();
	
public:
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Sockets")
	FName& GetAimSocket() { return AimSocket; }

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Magnification")
	virtual float GetMagnification() const { return 1.0f; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Magnification")
	virtual void ZoomIn() {};
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Magnification")
	virtual void ZoomOut() {};

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
	virtual void ReturnToZeroElevation() {}
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void ReturnToZeroWindage() {}
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void SetNewZeroElevation() {}
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void SetNewZeroWindage() {}
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void PointOfImpactUp() {}
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void PointOfImpactDown() {}
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void PointOfImpactLeft() {}
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void PointOfImpactRight() {}
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Zero")
	FSightZero GetSightZero() const { return SightZero; }
	
	// Aim Interface defaults
	virtual FTransform GetDefaultSightSocketTransform() override { return GetSightSocketTransform(); }
	virtual FTransform GetSightSocketTransform() override { return GetAimSocketTransform(); }
	virtual int32 GetAnimationIndex() const override { return AnimationIndex; }
	virtual float GetAimInterpolationMultiplier() override { return AimInterpolationMultiplier; }
	virtual float GetRotationLagInterpolationMultiplier() override { return RotationLagInterpolationMultiplier; }
	virtual float GetMovementLagInterpolationMultiplier() override { return MovementLagInterpolationMultiplier; }
	virtual AFPSTemplate_SightBase* GetActiveSight() override { return this; }
	virtual void DisableRenderTargets(bool Disable) override { DisableRenderTarget(Disable); }
	virtual FRotator GetHeadRotation() const override { return HeadAimRotation; }
	virtual FName GetGripSocketName() const override { return GripSocket; }
	virtual float GetCurrentMagnification() const override { return GetMagnification(); }
	virtual FAimCameraSettings& GetCameraSettings() override { return CameraSettings; }
};
