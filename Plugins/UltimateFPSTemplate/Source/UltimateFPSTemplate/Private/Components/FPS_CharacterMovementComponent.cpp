// Copyright 2022, Dakota Dawe, All rights reserved


#include "Components/FPS_CharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

UFPS_CharacterMovementComponent::UFPS_CharacterMovementComponent()
{
	MaxAcceleration = 1365.0f;
	BrakingFriction = 1.0f;
	MaxWalkSpeed = 150.0f;
	
	SprintSpeed = 350.0f;
	bCanOnlySprintForwards = false;
	SprintSidewaysSpeed = 300.0f;
	SprintBackwardsSpeed = 250.0f;
	MoveDirectionTolerance = 0.5f;
	DefaultMaxWalkSpeed = 150.0f;

	bCanSprintWhileCrouched = false;
	bCrouchCanOnlySprintForwards = false;
	CrouchSprintSpeed = 125.0f;
	CrouchSprintSidewaysSpeed = 100.0f;
	CrouchSprintBackwardsSpeed = 80.0f;

	MaxWalkSpeedCrouched = 50.0f;
}

void UFPS_CharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	DefaultMaxWalkSpeed = MaxWalkSpeed;
}

bool UFPS_CharacterMovementComponent::IsMovingForwards() const
{
	if (CharacterOwner)
	{
		FVector Forward = CharacterOwner->GetActorForwardVector();
		FVector MoveDirection = Velocity.GetSafeNormal();

		Forward.Z = 0.0f;
		MoveDirection.Z = 0.0f;
		return FVector::DotProduct(Forward, MoveDirection) > MoveDirectionTolerance;
	}
	return false;
}

bool UFPS_CharacterMovementComponent::IsMovingBackwards() const
{
	if (CharacterOwner)
	{
		FVector Forward = CharacterOwner->GetActorForwardVector();
		FVector MoveDirection = Velocity.GetSafeNormal();

		Forward.Z = 0.0f;
		MoveDirection.Z = 0.0f;
		return FVector::DotProduct(Forward, MoveDirection) < -MoveDirectionTolerance;
	}
	return false;
}

bool UFPS_CharacterMovementComponent::IsMovingSideways() const
{
	if (CharacterOwner)
	{
		FVector Right = CharacterOwner->GetActorRightVector();
		FVector MoveDirection = Velocity.GetSafeNormal();

		Right.Z = 0.0f;
		MoveDirection.Z = 0.0f;
		const float DotProduct = FVector::DotProduct(Right, MoveDirection);
		return DotProduct > MoveDirectionTolerance || DotProduct < -MoveDirectionTolerance;
	}
	return false;
}

void UFPS_CharacterMovementComponent::SetSprinting(bool bSprint)
{
    bWantsToSprint = bSprint;
}

float UFPS_CharacterMovementComponent::HandleSprintSpeed() const
{
	float MaxSpeed = DefaultMaxWalkSpeed;

	if (IsCrouching() && !bCanSprintWhileCrouched)
	{
		return MaxWalkSpeedCrouched;
	}

	if (IsCrouching())
	{
		MaxSpeed = MaxWalkSpeedCrouched;
		if (IsMovingForwards())
		{
			MaxSpeed = CrouchSprintSpeed;
			if (bCrouchCanOnlySprintForwards && IsMovingSideways())
			{
				MaxSpeed = MaxWalkSpeedCrouched;
			}
		}
		if (!bCrouchCanOnlySprintForwards)
		{
			if (bCrouchSprintSideways && IsMovingSideways())
			{
				MaxSpeed = CrouchSprintSidewaysSpeed;
			}
			else if (bCrouchSprintBackwards && IsMovingBackwards())
			{
				MaxSpeed = CrouchSprintBackwardsSpeed;
			}
		}
		return MaxSpeed;
	}
	
	if (IsMovingForwards())
	{
		IsCrouching() ? MaxSpeed = CrouchSprintSpeed : MaxSpeed = SprintSpeed;
		MaxSpeed = SprintSpeed;
		if (bCanOnlySprintForwards && IsMovingSideways())
		{
			MaxSpeed = DefaultMaxWalkSpeed;
		}
	}
	if (!bCanOnlySprintForwards)
	{
		if (bSprintSideways && IsMovingSideways())
		{
			MaxSpeed = SprintSidewaysSpeed;
		}
		else if (bSprintBackwards && IsMovingBackwards())
		{
			MaxSpeed = SprintBackwardsSpeed;
		}
	}
	return MaxSpeed;
}

float UFPS_CharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	if (bWantsToSprint)
	{
		MaxSpeed = HandleSprintSpeed();
	}
	return MaxSpeed;
}

void UFPS_CharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	
	bWantsToSprint = (Flags&FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* UFPS_CharacterMovementComponent::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		UFPS_CharacterMovementComponent* MutableThis = const_cast<UFPS_CharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_FPSTemplate(*this);
	}
	return ClientPredictionData;
}

void FSavedMove_FPSTemplate::Clear()
{
	Super::Clear();
	bSavedWantsToSprint = false;
}

uint8 FSavedMove_FPSTemplate::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (bSavedWantsToSprint)
	{
		Result |= FLAG_Custom_0;
	}
	return Result;
}

bool FSavedMove_FPSTemplate::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	if (bSavedWantsToSprint != ((FSavedMove_FPSTemplate*)&NewMove)->bSavedWantsToSprint)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_FPSTemplate::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if (const UFPS_CharacterMovementComponent* CustomMovementComponent = Cast<UFPS_CharacterMovementComponent>(C->GetCharacterMovement()))
	{
		bSavedWantsToSprint = CustomMovementComponent->bWantsToSprint;
	}
}

void FSavedMove_FPSTemplate::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	if (UFPS_CharacterMovementComponent* CustomMovementComponent = Cast<UFPS_CharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		CustomMovementComponent->bWantsToSprint = bSavedWantsToSprint;
	}
}

FSavedMovePtr FNetworkPredictionData_Client_FPSTemplate::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_FPSTemplate);
}