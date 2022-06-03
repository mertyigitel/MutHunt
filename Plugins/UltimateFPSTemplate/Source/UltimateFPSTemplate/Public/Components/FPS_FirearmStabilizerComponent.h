// Copyright 2022, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FPSTemplateDataTypes.h"
#include "FPS_FirearmStabilizerComponent.generated.h"

class AFPSTemplateFirearm;
class UFPSTemplate_CharacterComponent;
class UCurveVector;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFirearmStabilized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFirearmUnStabilized);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTIMATEFPSTEMPLATE_API UFPS_FirearmStabilizerComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UFPS_FirearmStabilizerComponent();

protected:
	// What Trace Channel to use for detection
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Stabilizer")
	TEnumAsByte<ETraceTypeQuery> CollisionChannel;
	// Whether or not to draw debug lines and what type of debugging
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Stabilizer")
	TEnumAsByte<EDrawDebugTrace::Type> DebugType;
	// Only attempt to stabilize with a key press, does not automatically stabilize
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Stabilizer")
	bool bOnlyWithKeyPress;
	// Only run the stabilization logic on the owner. I recommend leaving to true
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Stabilizer")
	bool bOnlyRunOnOwner;
	// Dimensions of the stabilizer box
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Stabilizer")
	FVector Size;
	// How many times per second to check for a surface to stabilize on
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Stabilizer")
	float UpdateRate;
	// Curve to play upon stabilized
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Stabilizer")
	FCurveData StabilizedCurveData;
	// Curve to play upon unstabilized
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Stabilizer")
	FCurveData UnStabilizedCurveData;

	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Stabilizer")
	AFPSTemplateFirearm* Firearm;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Stabilizer")
	UFPSTemplate_CharacterComponent* CharacterComponent;

	bool bIsStabilized;
	
protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	bool bIsLive;
	FTimerHandle EditorDebugHandle;
	void DrawDebugEditor();
#endif

	void StabilizeFirearm();
	
	UPROPERTY(BlueprintAssignable, Category = "FPSTemplate | Stabilizer")
	FFirearmStabilized FirearmStabilized;
	UPROPERTY(BlueprintAssignable, Category = "FPSTemplate | Stabilizer")
	FFirearmUnStabilized FirearmUnStabilized;
	
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Gets called on Begin Play by default, sets Firearm and CharacterComponent reference.
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Stabilizer")
	void CacheEssentials();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Stabilizer")
	void Enable();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Stabilizer")
	void Disable();
	// True if firearm resting area found
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Stabilizer")
	bool FindStabilizingSurface();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Stabilizer")
	void AttemptStabilizeFirearm();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Stabilizer")
	void UnStabilizeFirearm();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Stabilizer")
	bool IsStabilized() const { return bIsStabilized; }
};
