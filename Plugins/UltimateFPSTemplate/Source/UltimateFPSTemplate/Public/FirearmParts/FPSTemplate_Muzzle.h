// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "FPSTemplate_Muzzle.generated.h"


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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature")
	bool bUseMuzzleTemperature;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float TemperatureIncreasePerShot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float TemperatureDecreasePerTick;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float MinTemperatureEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float MaxTemperatureEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Temperature", meta = (EditCondition = "bUseMuzzleTemperature"))
	float TemperatureCooldownTickRate;
	
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

	virtual void CacheParts() override;

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Effects")
	void FiredShot();
};
