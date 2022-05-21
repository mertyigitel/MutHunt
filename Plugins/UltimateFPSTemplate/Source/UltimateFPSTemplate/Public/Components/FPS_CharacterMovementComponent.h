// Copyright 2022, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FPS_CharacterMovementComponent.generated.h"

class FSavedMove_FPSTemplate;
class FNetworkPredictionData_Client;

UCLASS()
class ULTIMATEFPSTEMPLATE_API UFPS_CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	UFPS_CharacterMovementComponent();

	virtual void BeginPlay() override;
	
	friend FSavedMove_FPSTemplate;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual float GetMaxSpeed() const override;

	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Standing")
	float SprintSpeed;
	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Standing")
	float MoveDirectionTolerance;
	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Standing")
	bool bCanOnlySprintForwards;
	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Standing", meta = (EditCondition = "!bCanOnlySprintForwards"))
	bool bSprintSideways;
	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Standing", meta = (EditCondition = "!bCanOnlySprintForwards && bSprintSideways"))
	float SprintSidewaysSpeed;
	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Standing", meta = (EditCondition = "!bCanOnlySprintForwards"))
	bool bSprintBackwards;
	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Standing", meta = (EditCondition = "!bCanOnlySprintForwards && bSprintBackwards"))
	float SprintBackwardsSpeed;

	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Crouched")
	bool bCanSprintWhileCrouched;
	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Crouched", meta = (EditCondition = "bCanSprintWhileCrouched"))
	float CrouchSprintSpeed;
	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Crouched", meta = (EditCondition = "bCanSprintWhileCrouched"))
	bool bCrouchCanOnlySprintForwards;
	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Crouched", meta = (EditCondition = "bCanSprintWhileCrouched && !bCrouchCanOnlySprintForwards"))
	bool bCrouchSprintSideways;
	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Crouched", meta = (EditCondition = "bCanSprintWhileCrouched && !bCrouchCanOnlySprintForwards && bCrouchSprintSideways"))
	float CrouchSprintSidewaysSpeed;
	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Crouched", meta = (EditCondition = "bCanSprintWhileCrouched && !bCrouchCanOnlySprintForwards"))
	bool bCrouchSprintBackwards;
	UPROPERTY(EditAnywhere, Category = "Character Movement: FPSTemplate | Sprinting | Crouched", meta = (EditCondition = "bCanSprintWhileCrouched && !bCrouchCanOnlySprintForwards && bCrouchSprintBackwards"))
	float CrouchSprintBackwardsSpeed;
	
	float DefaultMaxWalkSpeed;
	uint8 bWantsToSprint : 1;

	bool IsMovingForwards() const;
	bool IsMovingBackwards() const;
	bool IsMovingSideways() const;
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | CharacterMovement")
	void SetSprinting(bool bSprint);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | CharacterMovement")
	bool GetIsSprinting() const { return bWantsToSprint; }

	float HandleSprintSpeed() const;
};

class FSavedMove_FPSTemplate : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;
	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;
	virtual void PrepMoveFor(ACharacter* Character) override;

	uint8 bSavedWantsToSprint : 1;
};

class FNetworkPredictionData_Client_FPSTemplate : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_FPSTemplate(const UCharacterMovementComponent& ClientMovement):Super(ClientMovement){}
	typedef FNetworkPredictionData_Client_Character Super;
	virtual FSavedMovePtr AllocateNewMove() override;
};