// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Actors/FPSTemplateCharacter.h"
#include "SoldierCharacter.generated.h"

UCLASS()
class MUTHUNT_API ASoldierCharacter : public AFPSTemplateCharacter
{
	GENERATED_BODY()

public:
	friend class USoldierAnimInstance;
	ASoldierCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	//void EquipButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	/*UPROPERTY(VisibleAnywhere)
	class USoldierCombatComponent* Combat;*/

	/*UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();*/

	/*UPROPERTY(VisibleAnywhere ,BlueprintReadWrite, Category = "TrueFPS", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* ClientMesh;*/

public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	/*bool IsWeaponEquipped();
	bool IsAiming();
	AWeapon* GetEquippedWeapon();*/
};
