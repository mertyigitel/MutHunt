// Fill out your copyright notice in the Description page of Project Settings.


#include "SoldierAnimInstance.h"
#include "SoldierCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include <Kismet/KismetMathLibrary.h>
#include <Net/UnrealNetwork.h>

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
	bRotateRootBone = SoldierCharacter->ShouldRotateRootBone();
	bIsSimProxy = SoldierCharacter->IsSimProxy();

	if (SoldierCharacter->GetLocalRole() > ROLE_SimulatedProxy)
	{
		AimOffsetYaw(DeltaTime);
	}
}

void USoldierAnimInstance::AimOffsetYaw(float DeltaTime)
{
	SoldierYawLastTick = SoldierYaw;
	SoldierYaw = SoldierCharacter->GetActorRotation().Yaw;
	YawChangeOverFrame = SoldierYaw - SoldierYawLastTick;

	if ((SpeedForward == 0.f && SpeedSide == 0.f) || !bIsInAir) // standing still, not jumping
	{
		AO_Yaw = UKismetMathLibrary::NormalizeAxis(YawChangeOverFrame + AO_Yaw);
	}

	if (FMath::Abs(SpeedForward) > 0.f || FMath::Abs(SpeedSide) > 0.f || bIsInAir) // Running, or jumping
	{
		AO_Yaw = FMath::FInterpTo(AO_Yaw, 0.f, DeltaTime, 4.f);
	}

	//*********************************************** DEPRECATED **************************************************
	//SoldierYawLastTick = SoldierYaw;
	//SoldierYaw = SoldierCharacter->GetActorRotation().Yaw;
	//YawChangeOverFrame = SoldierYaw - SoldierYawLastTick;
	//
	//if ((SpeedForward == 0.f && SpeedSide == 0.f) || !bIsInAir) // standing still, not jumping
	//{
	//	bRotateRootBone = true;
	//	RootYawOffset = UKismetMathLibrary::NormalizeAxis(YawChangeOverFrame + RootYawOffset);
	//	SoldierCharacter->bUseControllerRotationYaw = true;
	//}
	//
	//if (FMath::Abs(SpeedForward) > 0.f || FMath::Abs(SpeedSide) > 0.f || bIsInAir) // Running, or jumping
	//{
	//	bRotateRootBone = false;
	//	RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.f, DeltaTime, 4.f);
	//	SoldierCharacter->bUseControllerRotationYaw = true;
	//}
	//*********************************************** DEPRECATED **************************************************
}
