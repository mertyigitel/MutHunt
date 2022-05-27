// Fill out your copyright notice in the Description page of Project Settings.

#include "SoldierCharacterComponent.h"
#include "MutHunt/Weapon/Weapon.h"
#include "MutHunt/Character/SoldierCharacter.h"
#include "Net/UnrealNetwork.h"

USoldierCharacterComponent::USoldierCharacterComponent() : Super::UFPSTemplate_CharacterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USoldierCharacterComponent::BeginPlay()
{
	Super::BeginPlay();
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
}

void USoldierCharacterComponent::OnRep_EquippedWeapon()
{

}