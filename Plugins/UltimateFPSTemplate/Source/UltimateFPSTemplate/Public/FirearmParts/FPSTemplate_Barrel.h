// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "FPSTemplate_Barrel.generated.h"

class AFPSTemplateProjectile;
class AFPSTemplate_Muzzle;
class AFPSTemplateFirearm;
class UNiagaraSystem;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_Barrel : public AFPSTemplate_PartBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplate_Barrel();

protected:
	// Socket for the muzzle (end of barrel where projectile gets launched)
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Sockets")
	FName MuzzleSocket;
	// Random niagara systems for this muzzle, to be used for shooting effects like muzzle flash
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	TArray<UNiagaraSystem*> FireNiagaraSystems;
	// Whether or not to use the muzzle temperature system as used in the example m4 barrel
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature")
	bool bUseMuzzleTemperature;
	// How much to increase the temperature by per shot
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float TemperatureIncreasePerShot;
	// How much to decrease the temperature by on tick (how quick to cool barrel)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float TemperatureDecreasePerTick;
	// Min value for temperature to affect material
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float MinTemperatureEffect;
	// Max temperature for full effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float MaxTemperatureEffect;
	// Rate at which the tick runs to cool barrel
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float TemperatureCooldownTickRate;

	UPROPERTY(Replicated)
	AFPSTemplate_Muzzle* MuzzleDevice;

	TWeakObjectPtr<UMaterialInstanceDynamic> MuzzleMaterial;
	float MuzzleTemperature;
	float MuzzleTemperatureNormalized;

	void NormalizeTemperature();
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Effects")
	void MuzzleTemperatureChanged(float NewMuzzleTemperature);
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

public:
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Ammo")
	FTransform GetMuzzleSocketTransform() const;

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	bool DoesMuzzleSocketExist() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	UNiagaraSystem* GetFireNiagaraSystem();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	AFPSTemplate_Muzzle* GetMuzzleDevice();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	AActor* GetMuzzleDeviceActor();

	virtual void CacheParts() override;

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Effects")
	void FiredShot();
};