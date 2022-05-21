// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"

#include "FirearmParts/BaseClasses/FPSTemplate_SightBase.h"
#include "FPSTemplate_SightMagnifiedRTBase.generated.h"

class UMaterialInterface;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_SightMagnifiedRTBase : public AFPSTemplate_SightBase
{
	GENERATED_BODY()

public:
	AFPSTemplate_SightMagnifiedRTBase();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	class USceneComponent* RootSceneComponent;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	class USceneCaptureComponent2D* SceneCapture;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	FSightOptimization ScopeOptimization;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	FSightMagnification MagnificationSettings;
	
	FRotator StartingSceneCaptureRot;

protected:
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void Tick(float DeltaSeconds) override;
	
	void SetFOVAngle() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Events")
	void OpticZoomedIn(float OldMagnification);

	void HandleReticleZoom();
	void HandleRenderTarget();

public:
	virtual float GetMagnification() const override { return MagnificationSettings.CurrentMagnification; }
	virtual void ZoomIn() override;
	virtual void ZoomOut() override;
	virtual void ReturnToZeroElevation() override;
	virtual void ReturnToZeroWindage() override;
	virtual void SetNewZeroElevation() override;
	virtual void SetNewZeroWindage() override;
	virtual void PointOfImpactUp() override;
	virtual void PointOfImpactDown() override;
	virtual void PointOfImpactLeft() override;
	virtual void PointOfImpactRight() override;
	virtual void DisableRenderTarget(bool Disable) override;
	virtual void ZoomOptic(bool bZoom) override;
};
