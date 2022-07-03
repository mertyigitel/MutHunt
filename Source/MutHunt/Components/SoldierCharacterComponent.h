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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire();

	UFUNCTION(Server, Reliable)
	void ServerStopFire();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopFire();

private:
	class ASoldierCharacter* Character;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeedCrouched;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeedCrouched;

	bool bFireButtonPressed;
};
