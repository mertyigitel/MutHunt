// Fill out your copyright notice in the Description page of Project Settings.


#include "SoldierCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MutHunt/Weapon/Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/FPSTemplate_CharacterComponent.h"
#include "MutHunt/Components/SoldierCharacterComponent.h"
#include "Components/WidgetComponent.h"
#include <Kismet/KismetMathLibrary.h>
#include "SoldierAnimInstance.h"

ASoldierCharacter::ASoldierCharacter(const FObjectInitializer& ObjectInitializer) : Super::AFPSTemplateCharacter(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = true;
	//Camera->SetupAttachment(GetMesh(), FName("Camera"));

	SoldierCharacterComponent = CreateDefaultSubobject<USoldierCharacterComponent>(TEXT("CharacterComponent"));
	SoldierCharacterComponent->SetIsReplicated(true);

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	bIsSimProxy = false;
}

void ASoldierCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (SoldierCharacterComponent)
		SoldierCharacterComponent->Init(Camera, true, GetMesh(), GetMesh());

	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		//TODO bu ayarlar daha sonra performans için kontrol edilmeli
		NetUpdateFrequency = 180.f;
		MinNetUpdateFrequency = 90.f;

		bIsSimProxy = true;
	}
}

void ASoldierCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ROLE_SimulatedProxy)
	{
		AimOffset(DeltaTime);
	}
	else
	{
		SimProxiesTurn(DeltaTime);
	}
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
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASoldierCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASoldierCharacter::FireButtonReleased);
}

void ASoldierCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASoldierCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ASoldierCharacter, TurningInPlace, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ASoldierCharacter, SoldierBaseAimYaw, COND_SimulatedOnly);
}

void ASoldierCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (SoldierCharacterComponent)
	{
		SoldierCharacterComponent->Character = this;
	}
}

void ASoldierCharacter::PostNetReceiveLocationAndRotation()
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		// Don't change transform if using relative position (it should be nearly the same anyway, or base may be slightly out of sync)
		if (!ReplicatedBasedMovement.HasRelativeLocation())
		{
			const FRepMovement& ConstRepMovement = GetReplicatedMovement();
			const FVector OldLocation = GetActorLocation();
			const FVector NewLocation = FRepMovement::RebaseOntoLocalOrigin(ConstRepMovement.Location, this);
			const FQuat OldRotation = GetActorQuat();

			GetCharacterMovement()->bNetworkSmoothingComplete = false;
			GetCharacterMovement()->bJustTeleported |= (OldLocation != NewLocation);
			//GetCharacterMovement()->SmoothCorrection(OldLocation, OldRotation, NewLocation, ConstRepMovement.Rotation.Quaternion());
			GetCharacterMovement()->SmoothCorrection(
				OldLocation,
				OldRotation,
				NewLocation,
				bShouldReplicateRotation ? ConstRepMovement.Rotation.Quaternion() : OldRotation);
			OnUpdateSimulatedPosition(OldLocation, OldRotation);
		}
		GetCharacterMovement()->bNetworkUpdateReceived = true;
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
}

void ASoldierCharacter::AimButtonReleased()
{
	if (SoldierCharacterComponent)
	{
		SoldierCharacterComponent->SetAiming(false);
	}
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

	USoldierAnimInstance* SoldierAnimInstance = Cast<USoldierAnimInstance>(GetMesh()->GetAnimInstance());
	if (SoldierAnimInstance)
	{
		AO_Yaw = SoldierAnimInstance->GetAO_Yaw();
	}

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir) // Running, or jumping
	{
		bRotateRootBone = false;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	SoldierBaseAimYaw = GetBaseAimRotation().Yaw;

	//*********************************************** DEPRECATED **************************************************
	// 
	////TODO AimOffset ve SimProxiesTurn animation blueprinte taþýnacak
	//FVector Velocity = GetVelocity();
	//Velocity.Z = 0.f;
	//float Speed = Velocity.Size();
	//bool bIsInAir = GetCharacterMovement()->IsFalling();
	//
	//if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	//{
	//	bRotateRootBone = true;
	//	FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
	//	FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
	//	AO_Yaw = DeltaAimRotation.Yaw;
	//	if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
	//	{
	//		InterpAO_Yaw = AO_Yaw;
	//	}
	//	bUseControllerRotationYaw = true;
	//	TurnInPlace(DeltaTime);
	//}
	//
	//if (Speed > 0.f || bIsInAir) // Running, or jumping
	//{
	//	bRotateRootBone = false;
	//	StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
	//	AO_Yaw = FMath::FInterpTo(AO_Yaw, 0.f, DeltaTime, 4.f);
	//	bUseControllerRotationYaw = true;
	//	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	//}
	//
	//SoldierBaseAimYaw = GetBaseAimRotation().Yaw;
	//*********************************************** DEPRECATED **************************************************
}

void ASoldierCharacter::SimProxiesTurn(float DeltaTime)
{
	bRotateRootBone = false;

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bUseControllerRotationYaw = false;
		bShouldReplicateRotation = false;

		FRotator CurrentAimRotation = FRotator(0.f, SoldierBaseAimYaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, GetActorRotation());
		AO_Yaw = DeltaAimRotation.Yaw;

		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), FRotator(0.f, SoldierBaseAimYaw, 0.f), DeltaTime, 4.f);
		if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
			SetActorRotation(NewRotation);
	}

	if (Speed > 0.f || bIsInAir) // Running, or jumping
	{
		AO_Yaw = FMath::FInterpTo(AO_Yaw, 0.f, DeltaTime, 4.f);

		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), FRotator(0.f, SoldierBaseAimYaw, 0.f), DeltaTime, 4.f);
		if (FMath::Abs(GetActorRotation().Yaw - SoldierBaseAimYaw) > 2.f)
			SetActorRotation(NewRotation);
		else
		{
			bShouldReplicateRotation = true;
			bUseControllerRotationYaw = true;
		}
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

void ASoldierCharacter::FireButtonPressed()
{
	if (SoldierCharacterComponent)
	{
		SoldierCharacterComponent->FireButtonPressed(true);
	}
}

void ASoldierCharacter::FireButtonReleased()
{
	if (SoldierCharacterComponent)
	{
		SoldierCharacterComponent->FireButtonPressed(false);
	}
}