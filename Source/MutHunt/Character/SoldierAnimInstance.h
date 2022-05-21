// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MutHunt/Weapon/Weapon.h"
#include "FPSTemplateAnimInstance.h"
#include "SoldierAnimInstance.generated.h"

/**
 * 
 */
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

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating; // Not real physics acceleration, shows whether we give input to the character or not

	UPROPERTY(BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	class AWeapon* EquippedWeapon;

	/*UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;*/

	/*UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadWrite, Category = "TrueFPS", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrueFPS", meta = (AllowPrivateAccess = "true"))
	FIKProperties IKProperties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrueFPS|IK Vars", meta = (AllowPrivateAccess = "true"))
	FTransform CameraTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrueFPS|IK Vars", meta = (AllowPrivateAccess = "true"))
	FTransform RelativeCameraTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrueFPS|IK Vars", meta = (AllowPrivateAccess = "true"))
	FTransform RHandToSightsTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrueFPS", meta = (AllowPrivateAccess = "true"))
	float ADSWeight = 0.f;*/
};
