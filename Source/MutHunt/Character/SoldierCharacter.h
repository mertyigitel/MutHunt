// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Actors/FPSTemplateCharacter.h"
#include "MutHunt/MutHuntTypes/TurningInPlace.h"
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
	virtual void PostNetReceiveLocationAndRotation() override;

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	virtual void Jump() override;
	void AimOffset(float DeltaTime);
	void SimProxiesTurn(float DeltaTime);
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class USoldierCharacterComponent* SoldierCharacterComponent;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	FRotator StartingAimRotation;

	UPROPERTY(Replicated)
	ETurningInPlace TurningInPlace;

	bool bRotateRootBone;
	bool bShouldReplicateRotation;
	bool bIsSimProxy;

	UPROPERTY(Replicated)
	float SoldierBaseAimYaw;

	void TurnInPlace(float DeltaTime);

	float SoldierYawLastTick;
	float SoldierYaw;
	float YawChangeOverFrame;

public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	//AWeapon* GetEquippedWeapon();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsSimProxy() const { return bIsSimProxy; }
};
