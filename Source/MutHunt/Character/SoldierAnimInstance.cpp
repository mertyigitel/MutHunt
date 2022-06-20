// Fill out your copyright notice in the Description page of Project Settings.


#include "SoldierAnimInstance.h"
#include "SoldierCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include <Kismet/KismetMathLibrary.h>

void USoldierAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	SoldierCharacter = Cast<ASoldierCharacter>(TryGetPawnOwner());
}

void USoldierAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (SoldierCharacter == nullptr)
		SoldierCharacter = Cast<ASoldierCharacter>(TryGetPawnOwner());

	if (SoldierCharacter == nullptr) return;

	FVector CharVelocity = SoldierCharacter->GetVelocity();
	SpeedForward = FVector::DotProduct(CharVelocity, SoldierCharacter->GetActorForwardVector());
	SpeedSide = FVector::DotProduct(CharVelocity, SoldierCharacter->GetActorRightVector());

	bIsInAir = SoldierCharacter->GetMovementComponent()->IsFalling();
	bIsAccelerating = SoldierCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
	bWeaponEquipped = SoldierCharacter->IsWeaponEquipped();
	bIsCrouched = SoldierCharacter->bIsCrouched;
	//EquippedWeapon = SoldierCharacter->GetEquippedWeapon();
	bIsAiming = SoldierCharacter->IsAiming();
	AO_Yaw = SoldierCharacter->GetAO_Yaw();
	TurningInPlace = SoldierCharacter->GetTurningInPlace();
}
