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
#include "../Components/SoldierCharacterComponent.h"

ASoldierCharacter::ASoldierCharacter(const FObjectInitializer& ObjectInitializer) : Super::AFPSTemplateCharacter(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	/*GetMesh()->SetTickGroup(TG_PostUpdateWork);
	GetMesh()->bVisibleInReflectionCaptures = true;
	GetMesh()->bCastHiddenShadow = true;

	ClientMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ClientMesh"));
	ClientMesh->SetCastShadow(false);
	ClientMesh->bCastHiddenShadow = false;
	ClientMesh->bVisibleInReflectionCaptures = false;
	ClientMesh->SetTickGroup(TG_PostUpdateWork);
	ClientMesh->SetupAttachment(GetMesh());*/

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = true;
	//Camera->SetupAttachment(GetMesh(), FName("Camera"));
	
	/*Combat = CreateDefaultSubobject<USoldierCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);*/

	CharacterComponent = CreateDefaultSubobject<USoldierCharacterComponent>(TEXT("CharacterComponent"));
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
}

void ASoldierCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ThisClass::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASoldierCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASoldierCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ASoldierCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ASoldierCharacter::LookUp);

	//PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ASoldierCharacter::EquipButtonPressed);
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
	/*if (Combat)
	{
		Combat->Character = this;
	}*/
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

//void ASoldierCharacter::EquipButtonPressed()
//{
//	if (Combat)
//	{
//		if (HasAuthority())
//		{
//			Combat->EquipWeapon(OverlappingWeapon);
//		}
//		else
//		{
//			ServerEquipButtonPressed();
//		}
//	}
//}

void ASoldierCharacter::AimButtonPressed()
{
	/*if (Combat)
	{
		Combat->SetAiming(true);
	}*/
	CharacterComponent->StartAiming();
}

void ASoldierCharacter::AimButtonReleased()
{
	/*if (Combat)
	{
		Combat->SetAiming(false);
	}*/
	CharacterComponent->StopAiming();
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

//bool ASoldierCharacter::IsWeaponEquipped()
//{
//	return (Combat && Combat->EquippedWeapon);
//}

//bool ASoldierCharacter::IsAiming()
//{
//	return (Combat && Combat->bAiming);
//}

//AWeapon* ASoldierCharacter::GetEquippedWeapon()
//{
//	if (Combat == nullptr) return nullptr;
//
//	return Combat->EquippedWeapon;
//}

void ASoldierCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}