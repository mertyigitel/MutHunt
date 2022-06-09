// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/FPSTemplate_CharacterComponent.h"
#include "SoldierCharacterComponent.generated.h"

UCLASS()
class MUTHUNT_API USoldierCharacterComponent : public UFPSTemplate_CharacterComponent
{
	GENERATED_BODY()
	
public:
	USoldierCharacterComponent();
	friend class ASoldierCharacter;

	void EquipWeapon(class AWeapon* WeaponToEquip);
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_EquippedWeapon();
private:
	class ASoldierCharacter* Character;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
};
