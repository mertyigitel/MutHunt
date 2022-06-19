// Fill out your copyright notice in the Description page of Project Settings.


#include "SoldierCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MutHunt/Weapon/Weapon.h"
#include "Net/UnrealNetwork.h"
//#include "MutHunt/Components/SoldierCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/FPSTemplate_CharacterComponent.h"
#include "MutHunt/Components/SoldierCharacterComponent.h"
#include "Components/WidgetComponent.h"
#include <Kismet/KismetMathLibrary.h>

ASoldierCharacter::ASoldierCharacter(const FObjectInitializer& ObjectInitializer) : Super::AFPSTemplateCharacter(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = true;
	//Camera->SetupAttachment(GetMesh(), FName("Camera"));
	
	/*Combat = CreateDefaultSubobject<USoldierCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);*/

	SoldierCharacterComponent = CreateDefaultSubobject<USoldierCharacterComponent>(TEXT("CharacterComponent"));
	SoldierCharacterComponent->SetIsReplicated(true);

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ASoldierCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	/*if (IsLocallyControlled())
	{
		ClientMesh->HideBoneByName(FName("neck_01"), PBO_None);
		GetMesh()->SetVisibility(false);
	}
	else
	{
		ClientMesh->DestroyComponent();
	}*/

	if (SoldierCharacterComponent)
		SoldierCharacterComponent->Init(Camera, true, GetMesh(), GetMesh());
}

void ASoldierCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

void ASoldierCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASoldierCharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASoldierCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASoldierCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ASoldierCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ASoldierCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ASoldierCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASoldierCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ASoldierCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ASoldierCharacter::AimButtonReleased);
}

void ASoldierCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASoldierCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ASoldierCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (SoldierCharacterComponent)
	{
		SoldierCharacterComponent->Character = this;
	}
}

void ASoldierCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ASoldierCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ASoldierCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ASoldierCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ASoldierCharacter::EquipButtonPressed()
{
	if (SoldierCharacterComponent)
	{
		if (HasAuthority())
		{
			SoldierCharacterComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void ASoldierCharacter::ServerEquipButtonPressed_Implementation()
{
	if (SoldierCharacterComponent)
	{
		SoldierCharacterComponent->EquipWeapon(OverlappingWeapon);
	}
}

void ASoldierCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		if (!GetCharacterMovement()->IsFalling())
			Crouch();
	}
}

void ASoldierCharacter::AimButtonPressed()
{
	if (SoldierCharacterComponent)
	{
		SoldierCharacterComponent->SetAiming(true);
	}
	//SoldierCharacterComponent->StartAiming();
}

void ASoldierCharacter::AimButtonReleased()
{
	if (SoldierCharacterComponent)
	{
		SoldierCharacterComponent->SetAiming(false);
	}
	//SoldierCharacterComponent->StopAiming();
}

void ASoldierCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ASoldierCharacter::AimOffset(float DeltaTime)
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir) // Running, or jumping
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
}

void ASoldierCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 60.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -60.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 5.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ASoldierCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

//void ASoldierCharacter::ServerEquipButtonPressed_Implementation()
//{
//	if (Combat)
//	{
//		Combat->EquipWeapon(OverlappingWeapon);
//	}
//}

void ASoldierCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ASoldierCharacter::IsWeaponEquipped()
{
	return (SoldierCharacterComponent && SoldierCharacterComponent->EquippedWeapon);
}

bool ASoldierCharacter::IsAiming()
{
	return (SoldierCharacterComponent && SoldierCharacterComponent->bIsAiming);
}

//AWeapon* ASoldierCharacter::GetEquippedWeapon()
//{
//	if (Combat == nullptr) return nullptr;
//
//	return Combat->EquippedWeapon;
//}