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

	Speed = FMath::FInterpTo(Speed, CharacterVelocity, DeltaTime, 10.f);
	bIsInAir = SoldierCharacter->GetMovementComponent()->IsFalling();
	bIsAccelerating = SoldierCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;

	// Offset Yaw for Strafing
	FRotator AimRotation = SoldierCharacter->GetBaseAimRotation();
	FRotator MovementRotation;
	if (bIsAccelerating)
		MovementRotation = UKismetMathLibrary::MakeRotFromX(SoldierCharacter->GetVelocity());
	else
		MovementRotation = AimRotation;
	
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);

	if (CharacterVelocity > 0.f)
		CachedDirection = CharacterDirection;

	if (!bIsAccelerating && Speed > 0.f)
	{
		CharacterDirection = CachedDirection;
	}
	else
	{	
		if (Speed > 0.f && Speed <= 50.f)
		{
			DeltaRotation = DeltaRot;
			CharacterDirection = DeltaRotation.Yaw;
		}
		else
		{
			DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 5.f);
			CharacterDirection = DeltaRotation.Yaw;
		}
	}

	bWeaponEquipped = SoldierCharacter->IsWeaponEquipped();
	bIsCrouched = SoldierCharacter->bIsCrouched;
	//EquippedWeapon = SoldierCharacter->GetEquippedWeapon();
	bIsAiming = SoldierCharacter->IsAiming();


}
