// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "FPSTemplate_LightLaserBase.generated.h"

class USpotLightComponent;
class UDecalComponent;
class UStaticMeshComponent;
class UMaterialInstance;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_LightLaserBase : public AFPSTemplate_PartBase
{
	GENERATED_BODY()
	
public:
	AFPSTemplate_LightLaserBase();

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "FPSTemplate | Default")
	USceneComponent* Root;
	UPROPERTY(VisibleDefaultsOnly, Category = "FPSTemplate | Default")
	UStaticMeshComponent* LaserMesh;
	UPROPERTY(VisibleDefaultsOnly, Category = "FPSTemplate | Default")
	USpotLightComponent* SpotLightComponent;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	UDecalComponent* LaserDecalComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	bool bIsLight;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	bool bIsLaser;
	// What collision channel to use for the light. For things like glass you will want to set it to ignore
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TEnumAsByte<ECollisionChannel> CollisionChannel;
	// Similar to Tarkov how you can cycle through different modes of the light laser. This is OPTIONAL to be used with CycleThroughModes()
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TArray<ELightLaser> CycleModes;
	// Light power intensity modes. Allows for changing the strength of the flashlight at runtime
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TArray<float> LightPowerIntensityLevels;
	// Max distance the lazer can reach
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default", meta = (EditCondition = "bIsLaser"))
	float MaxLaserDistance;
	// How much to scale the lazer dot by per distance
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default", meta = (EditCondition = "bIsLaser"))
	float LazerScaleMultiplier;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default", meta = (EditCondition = "bIsLaser"))
	TArray<UMaterialInstance*> LaserMaterials;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default", meta = (EditCondition = "bIsLaser"))
	TArray<UMaterialInstance*> LaserDotMaterials;
	
	// Socket for the laser to be projected from (use for collision/line tracing)
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Sockets")
	FName LaserSocket;

	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | LightLaser")
	uint8 CycleModesIndex;

	UPROPERTY(ReplicatedUsing = OnRep_LightPowerIntensityIndex)
	uint8 LightPowerIntensityIndex;
	UFUNCTION()
	void OnRep_LightPowerIntensityIndex();
	
	UPROPERTY(ReplicatedUsing = OnRep_LightOn)
	bool bLightOn;
	UFUNCTION()
	void OnRep_LightOn();
	UPROPERTY(ReplicatedUsing = OnRep_LaserOn)
    bool bLaserOn;
	UFUNCTION()
	void OnRep_LaserOn();

	UPROPERTY(ReplicatedUsing = OnRep_LaserColorIndex)
	uint8 LaserColorIndex;
	UFUNCTION()
	void OnRep_LaserColorIndex();

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_ToggleLight();
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_ToggleLaser();
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_CycleLaserColor(uint8 Index);
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_ToggleLightAndLaser(bool bSync);
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_CyclePowerModes(uint8 Index);

	virtual void PostInitProperties() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void SetupPartMesh() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Laser")
	void OnLaserScaleChanged(float DistanceForLaserHit);
	
public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle")
	void ToggleLight();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle")
	void ToggleLaser();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle")
	void ToggleLightAndLaser(bool bSync);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle")
	void CyclePowerModes();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle")
	void SetPowerMode(uint8 Index);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle")
	void CycleThroughModes();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle")
	void CycleLaserColor();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle")
	void SetLaserColor(uint8 Index);

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsLaserOn() { return bLaserOn; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Toggle", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsLightOn() { return bLightOn; }
};
