// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FPSTemplateDataTypes.h"
#include "Actors/FPSTemplateFirearm.h"
#include "FPSTemplate_CharacterComponent.generated.h"

class UFPSTemplateAnimInstance;
class USkeletalMeshComponent;
class UCameraComponent;
class AFPSTemplateFirearm;
class AFPSTemplate_RangeFinder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFreeLookEnd, FRotator, NewControlRotation);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTIMATEFPSTEMPLATE_API UFPSTemplate_CharacterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFPSTemplate_CharacterComponent();

protected:
	TWeakObjectPtr<UFPSTemplateAnimInstance> AnimationInstance;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Readables")
	UCameraComponent* FPCameraComponent;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Readables")
	USkeletalMeshComponent* FPMesh;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Readables")
	USkeletalMeshComponent* TPMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bUseParentSocketForAiming;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FName CameraSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FName CameraSocketParentBone;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float MovementComponentSprintSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float MaxLookUpAngle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float MaxLookDownAngle;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	float DefaultLeanAngle;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	int32 MaxFirearmAttachmentAttempts;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	float FirearmReAttachmentAttemptInterval;
	uint8 AttachmentAttempt;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FreeLook")
	float MaxLookLeftRight;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FreeLook")
	float MaxLookUpDown;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FirearmCollision")
	bool bUseFirearmCollision;
	//UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FirearmCollision")
	float FirearmCollisionDistanceCheck;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FirearmCollision")
	TEnumAsByte<ECollisionChannel> FirearmCollisionChannel;
	bool bFirearmCollisionHitting;

	bool bHighPort;
	bool bLowPort;
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetPortPose(EPortPose NewPortPose);
	UPROPERTY(ReplicatedUsing = OnRep_PortPose)
	EPortPose PortPose;
	UFUNCTION()
	void OnRep_PortPose() const;
	
	UPROPERTY(Replicated)
	bool bIsSprinting;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetSprinting(bool IsSprinting);
	
	UPROPERTY(ReplicatedUsing = OnRep_IsAiming)
	bool bIsAiming;
	UFUNCTION()
	void OnRep_IsAiming() const;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentFirearm)
	AFPSTemplateFirearm* CurrentFirearm;
	UFUNCTION()
	void OnRep_CurrentFirearm();

	UPROPERTY(ReplicatedUsing = OnRep_AimingActor)
	AActor* AimingActor;
	UFUNCTION()
	void OnRep_AimingActor();

	UPROPERTY(ReplicatedUsing = OnRep_Lean)
	ELeaning CurrentLean;
	UFUNCTION()
	void OnRep_Lean() const;
	UPROPERTY(ReplicatedUsing = OnRep_Lean)
	float LeanAngle;
	bool bLeanLeftDown;
	bool bLeanRightDown;

	UPROPERTY(ReplicatedUsing = OnRep_LeftHandFollowIK)
	bool bLeftHandFollowIK;
	UFUNCTION()
	void OnRep_LeftHandFollowIK() const;
	UPROPERTY(ReplicatedUsing = OnRep_UseLeftHandIK)
	bool bUseLeftHandIK;
	UFUNCTION()
	void OnRep_UseLeftHandIK() const;
	
	TWeakObjectPtr<APawn> OwningPawn;
	bool bIsInitialized;
	bool bLocallyControlled;
	float DefaultCameraFOV;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetAiming(bool IsAiming);
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetLean(ELeaning Lean);
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetLeanAngle(float NewLeanAngle);
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetLeftHandFollowIK(bool bFollow);
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetUseLeftHandIK(bool bUse);

	UPROPERTY(ReplicatedUsing = OnRep_FreeLook)
	bool bFreeLook;
	UFUNCTION()
	void OnRep_FreeLook();

	UPROPERTY(Replicated)
	float ControlYaw;

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_ClearCurrentFirearm();
	
public:
	bool HasAuthority() const { return GetOwner() ? GetOwner()->HasAuthority() : false; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | CharacterComponent")
	void Init(UCameraComponent* CameraComponent, bool bAutoAttach, USkeletalMeshComponent* FirstPersonMesh, USkeletalMeshComponent* ThirdPersonMesh);

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	USkeletalMeshComponent* GetInUseMesh() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	UFPSTemplateAnimInstance* GetAnimationInstance() const  { return AnimationInstance.Get(); }
	
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	UPawnMovementComponent* GetMovementComponent() const;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsLocallyControlled() { return bLocallyControlled; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	FRotator GetBaseAimRotation()const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	FRotator GetControlRotation()const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	const FName& GetCameraSocket()const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	UCameraComponent* GetCameraComponent() const { return FPCameraComponent; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	float GetLeanAngle()const { return LeanAngle; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void SetLeanAngle(float NewLeanAngle);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void ResetLeanAngle();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	float GetMovementComponentSprintSpeed() const { return MovementComponentSprintSpeed; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsAiming() { return bIsAiming; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	FVector GetActorForwardVector() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	FVector GetActorRightVector() const;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void RagdollCharacter();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void RagdollCharacterWithForce(const FVector ImpactLocation, const float ImpactForce = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void StartAiming();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void StopAiming();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetSprinting(bool Sprinting);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Procedural")
	bool GetIsSprinting()const { return bIsSprinting; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void LeanLeft();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void LeanRight();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void StopLeanLeft();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void StopLeanRight();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	bool IsUsingFirearmCollision() const { return bUseFirearmCollision; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetFirearmCollision(bool EnableFirearmCollision) { bUseFirearmCollision = EnableFirearmCollision; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetAimingActor(AActor* Actor) { AimingActor = Actor; OnRep_AimingActor(); }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetLeftHandFollowIK(bool bFollow);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetUseLeftHandIK(bool bUse);

	float GetDefaultFOV()const { return DefaultCameraFOV; }

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void PlayCameraShake(TSubclassOf<UCameraShakeBase> CameraShake, float Scale = 1.0f)const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	AFPSTemplateFirearm* GetFirearm() const { return CurrentFirearm; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	AActor* GetAimingActor() const { return CurrentFirearm ? Cast<AActor>(CurrentFirearm) : AimingActor; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void EquipFirearm(AFPSTemplateFirearm* Firearm);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	float GetMagnificationSensitivity() const;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	float GetMagnificationSensitivityStartValue(float StartAtMagnification = 4.0f) const;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void AttachItem(AActor* Actor, const FName SocketName);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void ClearCurrentFirearm();
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void SetHighPortPose();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void SetLowPortPose();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void StopLowPortPose();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void StopHighPortPose();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void StopHighAndLowPortPose();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | HighLowPort")
	EPortPose GetPortPose() const { return PortPose; }

	float LookUpDownOffset;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | FreeLook")
	void SetFreeLook(bool FreeLook);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | FreeLook")
	bool GetFreeLook() const { return bFreeLook; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | FreeLook", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool ValidTurn(float AxisValue);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | FreeLook", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool ValidLookUp(float AxisValue);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | FreeLook")
	void SetControlRotation(FRotator NewControlRotation);
	UPROPERTY(BlueprintAssignable, Category = "FPSTemplate | FreeLook")
	FOnFreeLookEnd FreeLookEnd;
};
