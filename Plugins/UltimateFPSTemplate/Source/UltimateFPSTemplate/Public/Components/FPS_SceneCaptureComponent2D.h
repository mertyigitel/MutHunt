// Copyright 2022, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "FPSTemplateDataTypes.h"
#include "FPS_SceneCaptureComponent2D.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnElevationChanged, float, OldAdjustment, float, NewAdjustment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWindageChanged, float, OldAdjustment, float, NewAdjustment);

UCLASS(hidecategories=(Collision, Object, Physics, SceneComponent), ClassGroup=Rendering, editinlinenew, meta=(BlueprintSpawnableComponent))
class ULTIMATEFPSTEMPLATE_API UFPS_SceneCaptureComponent2D : public USceneCaptureComponent2D
{
	GENERATED_BODY()

public:
	UFPS_SceneCaptureComponent2D(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	FSightOptimization Optimization;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	FSightMagnification MagnificationSettings;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Zero")
	EScopeAdjustment TurretAdjustmentType;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Setup")
	FRenderTargetSize RenderTargetSize;
	// Material parameter name for the render target
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Setup")
	FName RenderTargetMaterialVarName;

	APawn* OwningPawn;
	TWeakObjectPtr<UMeshComponent> OwningMesh;
	uint8 ReticleIndex;
	uint8 ReticleBrightnessIndex;
	FSightZero SightZero;
	UPROPERTY()
	FReticleSettings ReticleSettings;

	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Zero")
	float MilliradianAdjustment;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Zero")
	float MOAAdjustment;

	const float MilliradianClick = 0.1f;
	const float MOAClick = 0.25f;

	UPROPERTY(BlueprintAssignable, Category = "FPSTemplate | Zero")
	FOnElevationChanged OnElevationChanged;
	UPROPERTY(BlueprintAssignable, Category = "FPSTemplate | Zero")
	FOnWindageChanged OnWindageChanged;

	TWeakObjectPtr<UTextureRenderTarget2D> RenderTarget;

	FRotator StartingSceneCaptureRot;
	
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetupRenderTarget();
	void SetupMesh();
	
	void HandleReticleZoom();
	
public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | SceneComponent")
	void StartCapture();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | SceneComponent")
	void StopCapture();

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | SceneComponent")
	void SetOwningPawn(APawn* Pawn) { OwningPawn = Pawn; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | SceneComponent")
	void SetOwningMesh(UMeshComponent* Mesh);

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | SceneComponent")
	void ZoomIn();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | SceneComponent")
	void ZoomOut();

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | SceneComponent")
	float GetMagnification() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | SceneComponent")
	float GetMagnificationSensitivity() const;

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void ReturnToZeroElevation();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void ReturnToZeroWindage();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void SetNewZeroElevation();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Zero")
	virtual void SetNewZeroWindage();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | SceneComponent")
	FSightZero GetSightZero() const { return SightZero; }
	//X is Old Adjustment, Y is New Adjustment
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | SceneComponent")
	FVector2D PointOfImpactUp(uint8 Clicks = 1);
	//X is Old Adjustment, Y is New Adjustment
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | SceneComponent")
	FVector2D PointOfImpactDown(uint8 Clicks = 1);
	//X is Old Adjustment, Y is New Adjustment
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | SceneComponent")
	FVector2D PointOfImpactLeft(uint8 Clicks = 1);
	//X is Old Adjustment, Y is New Adjustment
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | SceneComponent")
	FVector2D PointOfImpactRight(uint8 Clicks = 1);

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	virtual void CycleReticle();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Reticle")
	virtual void SetReticle(uint8 Index);

	void SetReticleSettings(FReticleSettings& INReticleSettings);
};
