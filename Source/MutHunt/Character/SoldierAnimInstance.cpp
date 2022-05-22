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
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 10.f);
	CharacterDirection = DeltaRotation.Yaw;

	bIsInAir = SoldierCharacter->GetMovementComponent()->IsFalling();
	bIsAccelerating = SoldierCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
	//bWeaponEquipped = SoldierCharacter->IsWeaponEquipped();
	//EquippedWeapon = SoldierCharacter->GetEquippedWeapon();
	//bIsAiming = SoldierCharacter->IsAiming();

	//if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && SoldierCharacter->GetMesh())
	//{
	//	LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), RTS_World);
	//	FVector OutPosition;
	//	FRotator OutRotation;
	//	SoldierCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
	//	LeftHandTransform.SetLocation(OutPosition);
	//	LeftHandTransform.SetRotation(FQuat(OutRotation));
	//}

	//// True FPS implementations
	//if (bWeaponEquipped)
	//{
	//	IKProperties = EquippedWeapon->GetIKProperties();
	//	RHandToSightsTransform = EquippedWeapon->GetSightsWorldTransform().GetRelativeTransform(Mesh->GetSocketTransform(FName("hand_r")));
	//}
	//CameraTransform = FTransform(SoldierCharacter->GetBaseAimRotation(), SoldierCharacter->Camera->GetComponentLocation());
	//const FTransform& RootOffset = Mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * Mesh->GetSocketTransform(FName("ik_hand_root"));
	//RelativeCameraTransform = CameraTransform.GetRelativeTransform(RootOffset);

	//if(bAiming)
	//	ADSWeight = FMath::FInterpTo(ADSWeight, 1.f, DeltaTime, 15.f);
	//else
	//	ADSWeight = FMath::FInterpTo(ADSWeight, 0.f, DeltaTime, 15.f);
}
