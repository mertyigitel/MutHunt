// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPSTemplateDataTypes.h"
#include "GameplayTagContainer.h"
#include "FPSTemplate_AimInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFPSTemplate_AimInterface : public UInterface
{
	GENERATED_BODY()
};


class AFPSTemplate_SightBase;
class AFPSTemplate_MagnifierBase;

class ULTIMATEFPSTEMPLATE_API IFPSTemplate_AimInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	FTransform GetDefaultSightSocketTransform();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	FTransform GetSightSocketTransform();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	bool IsFirearm() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	int32 GetAnimationIndex() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	FGameplayTag GetAnimationGameplayTag() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	float GetAimInterpolationMultiplier();
	virtual float GetAimInterpolationMultiplier_Implementation() { return 1.0f; }
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	float GetUnAimInterpolationMultiplier();
	virtual float GetUnAimInterpolationMultiplier_Implementation() { return 1.0f; }
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	float GetRotationLagInterpolationMultiplier();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	float GetMovementLagInterpolationMultiplier();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	AFPSTemplate_SightBase* GetActiveSight();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	void DisableRenderTargets(bool Disable);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	void SetMagnifier(AFPSTemplate_MagnifierBase* INMagnifier);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	AFPSTemplate_MagnifierBase* GetMagnifier();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	FName GetGripSocketName() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	FRotator GetHeadRotation() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	float GetCurrentMagnification() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	void GetCameraSettings(FAimCameraSettings& OutCameraSettings);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	FSwayMultipliers GetSwayMultipliers();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AimInterface")
	void ZoomOptic(bool bZoom);
};
