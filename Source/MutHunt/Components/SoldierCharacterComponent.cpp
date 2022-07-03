// Fill out your copyright notice in the Description page of Project Settings.

#include "SoldierCharacterComponent.h"
#include "MutHunt/Weapon/Weapon.h"
#include "MutHunt/Character/SoldierCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

USoldierCharacterComponent::USoldierCharacterComponent() : 
	Super::UFPSTemplate_CharacterComponent(),
	BaseWalkSpeed(400.f),
	AimWalkSpeed(200.f),
	BaseWalkSpeedCrouched(200.f),
	AimWalkSpeedCrouched(100.f)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USoldierCharacterComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BaseWalkSpeedCrouched;
	}
}

void USoldierCharacterComponent::SetAiming(bool bAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) { return; }

	if (bAiming)
	{
		StartAiming();
		Character->GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = AimWalkSpeedCrouched;
	}
	else
	{
		StopAiming();
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BaseWalkSpeedCrouched;
	}

	if (!Character->HasAuthority())
	{
		ServerSetAiming(bAiming);
	}
}

void USoldierCharacterComponent::ServerSetAiming_Implementation(bool bAiming)
{
	if (bAiming)
	{
		StartAiming();
		Character->GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = AimWalkSpeedCrouched;
	}
	else
	{
		StopAiming();
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BaseWalkSpeedCrouched;
	}
}

void USoldierCharacterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USoldierCharacterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USoldierCharacterComponent, EquippedWeapon);
	//DOREPLIFETIME(USoldierCharacterComponent, bAiming);
}

void USoldierCharacterComponent::EquipWeapon(class AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) { return; }

	EquipFirearm(WeaponToEquip);
	EquippedWeapon = Cast<AWeapon>(CurrentFirearm);
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->ShowPickupWidget(false);
	EquippedWeapon->UpdateCanFire();
}

void USoldierCharacterComponent::OnRep_EquippedWeapon()
{

}

void USoldierCharacterComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (EquippedWeapon == nullptr) { return; }

	if (bFireButtonPressed)
	{
		EquippedWeapon->Fire(); // UltimateFPSTemplate multiplayer logic is done in blueprint
		ServerFire();
	}
	else
	{
		EquippedWeapon->StopFire(); // UltimateFPSTemplate multiplayer logic is done in blueprint
		ServerStopFire();
	}
}

void USoldierCharacterComponent::ServerFire_Implementation()
{
	MulticastFire();
}

void USoldierCharacterComponent::MulticastFire_Implementation()
{
	if (EquippedWeapon == nullptr) { return; }

	EquippedWeapon->Fire_Implementation();
}

void USoldierCharacterComponent::ServerStopFire_Implementation()
{
	MulticastStopFire();
}

void USoldierCharacterComponent::MulticastStopFire_Implementation()
{
	if (EquippedWeapon == nullptr) { return; }

	EquippedWeapon->StopFire_Implementation();
}
