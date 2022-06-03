// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "FPSTemplate_Muzzle.generated.h"

class UNiagaraSystem;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_Muzzle : public AFPSTemplate_PartBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplate_Muzzle();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bIsSuppressor;
	// Random niagara systems for this muzzle, to be used for shooting effects like muzzle flash
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	TArray<UNiagaraSystem*> FireNiagaraSystems;
	// Whether or not to use the muzzle temperature system as used in the example Surefire muzzle devices
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature")
	bool bUseMuzzleTemperature;
	// How much to increase the temperature by per shot
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float TemperatureIncreasePerShot;
	// How much to decrease the temperature by on tick (how quick to cool muzzle device)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float TemperatureDecreasePerTick;
	// Min value for temperature to affect material
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float MinTemperatureEffect;
	// Max temperature for full effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float MaxTemperatureEffect;
	// Rate at which the tick runs to cool muzzle
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float TemperatureCooldownTickRate;
	// Socket for the muzzle (end of muzzle where projectile gets launched)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName MuzzleSocket;

	UPROPERTY(Replicated)
	AFPSTemplate_Muzzle* MuzzleAttachment;

	TWeakObjectPtr<UMaterialInstanceDynamic> MuzzleMaterial;
	float MuzzleTemperature;
	float MuzzleTemperatureNormalized;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void NormalizeTemperature();

	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Effects")
	void MuzzleTemperatureChanged(float NewMuzzleTemperature);
	
public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Ammo")
	FTransform GetMuzzleSocketTransform();

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default")
	bool DoesMuzzleSocketExist();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default")
	bool IsSuppressor() const;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Ammo")
	AFPSTemplate_Muzzle* GetMuzzleAttachment() { return IsValid(MuzzleAttachment) ? MuzzleAttachment : this; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	UNiagaraSystem* GetFireNiagaraSystem();

	virtual void CacheParts() override;

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Effects")
	void FiredShot();
};
