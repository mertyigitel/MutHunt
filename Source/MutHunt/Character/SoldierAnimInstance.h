// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MutHunt/Weapon/Weapon.h"
#include "FPSTemplateAnimInstance.h"
#include "SoldierAnimInstance.generated.h"


UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	EMD_Forward UMETA(DisplayName = "Forward"),
	EMD_Back UMETA(DisplayName = "Back"),
	EMD_Right UMETA(DisplayName = "Right"),
	EMD_Left UMETA(DisplayName = "Left"),
	EMD_ForwardRight UMETA(DisplayName = "ForwardRight"),
	EMD_ForwardLeft UMETA(DisplayName = "ForwardLeft"),
	EMD_BackRight UMETA(DisplayName = "BackRight"),
	EMD_BackLeft UMETA(DisplayName = "BackLeft"),

	EMD_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class MUTHUNT_API USoldierAnimInstance : public UFPSTemplateAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime);

private:
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class ASoldierCharacter* SoldierCharacter;

	/*UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;*/

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating; // Not real physics acceleration, shows whether we give input to the character or not

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	class AWeapon* EquippedWeapon;

	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	EMovementDirection MovementDirection;

	/*UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;*/

protected:
	void CalculateMovementDirection();
};
