// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPSTemplateDataTypes.h"
#include "FPSTemplate_AimInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UFPSTemplate_AimInterface : public UInterface
{
	GENERATED_BODY()
};


class AFPSTemplate_SightBase;

class ULTIMATEFPSTEMPLATE_API IFPSTemplate_AimInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	FAimCameraSettings Temp;
	virtual FTransform GetDefaultSightSocketTransform() { return FTransform(); }
	virtual FTransform GetSightSocketTransform() { return FTransform(); }
	virtual bool IsFirearm() const { return false; }
	virtual int32 GetAnimationIndex() const = 0;
	virtual float GetAimInterpolationMultiplier() = 0;
	virtual float GetRotationLagInterpolationMultiplier() = 0;
	virtual float GetMovementLagInterpolationMultiplier() = 0;
	virtual AFPSTemplate_SightBase* GetActiveSight() = 0;
	virtual void DisableRenderTargets(bool Disable) {}
	virtual FName GetGripSocketName() const = 0;
	virtual FRotator GetHeadRotation() const { return FRotator(45.0f, 0.0f, 0.0f); }
	virtual float GetCurrentMagnification() const { return 1.0f; }
	virtual FAimCameraSettings& GetCameraSettings() { return Temp; }
	virtual FSwayMultipliers GetSwayMultipliers() const { return FSwayMultipliers(); }
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | AimInterface")
	virtual void ZoomOptic(bool bZoom) {}
};
