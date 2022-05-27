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

	/*FVector Velocity = SoldierCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();*/

	// Offset Yaw for Strafing
	FRotator AimRotation = SoldierCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(SoldierCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	/*DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 10.f);
	CharacterDirection = DeltaRotation.Yaw;*/
	CharacterDirection = DeltaRot.Yaw;

	CalculateMovementDirection();

	bIsInAir = SoldierCharacter->GetMovementComponent()->IsFalling();
	bIsAccelerating = SoldierCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
	bWeaponEquipped = SoldierCharacter->IsWeaponEquipped();
	//EquippedWeapon = SoldierCharacter->GetEquippedWeapon();
	//bIsAiming = SoldierCharacter->IsAiming();


}

void USoldierAnimInstance::CalculateMovementDirection()
{
	if (CharacterDirection >= -22.5f && CharacterDirection < 22.5f)
	{
		MovementDirection = EMovementDirection::EMD_Forward;
	}
	else if (CharacterDirection >= 22.5f && CharacterDirection < 67.f)
	{
		MovementDirection = EMovementDirection::EMD_ForwardRight;
	}
	else if (CharacterDirection >= 67.5f && CharacterDirection < 112.5f)
	{
		MovementDirection = EMovementDirection::EMD_Right;
	}
	else if (CharacterDirection >= 112.5f && CharacterDirection < 157.5f)
	{
		MovementDirection = EMovementDirection::EMD_BackRight;
	}
	else if ((CharacterDirection >= 157.5f && CharacterDirection <= 180.f) || (CharacterDirection >= -180.f && CharacterDirection < -157.5f))
	{
		MovementDirection = EMovementDirection::EMD_Back;
	}
	else if (CharacterDirection >= -157.5f && CharacterDirection < -112.5f)
	{
		MovementDirection = EMovementDirection::EMD_BackLeft;
	}
	else if (CharacterDirection >= -112.5f && CharacterDirection < -67.5f)
	{
		MovementDirection = EMovementDirection::EMD_Left;
	}
	else if (CharacterDirection >= -67.5f && CharacterDirection < -22.5f)
	{
		MovementDirection = EMovementDirection::EMD_ForwardLeft;
	}
}
