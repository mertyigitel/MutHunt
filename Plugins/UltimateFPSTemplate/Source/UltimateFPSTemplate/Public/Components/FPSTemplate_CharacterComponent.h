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
	
	// Sets to use third person mode by default (replacement first person poses with third person poses for local client)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bIsThirdPersonDefault;
	// Will set the owner automatically when you call EquipFirearm or SetAimingActor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bAutoSetOwnerOnEquipFirearmAndAimingActor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bUseParentSocketForAiming;
	// The socket that the camera is to be attached to
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FName CameraSocket;
	// The camera sockets parent bone such as the *head* bone in the example character
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FName CameraSocketParentBone;
	// Default is for mannequin. If you find adjusting the Camera Distance in the Camera Settings (of firearm and sights) going the wrong direction, change it
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	TEnumAsByte<EAxis::Type> RightHandAxis;
	// Set this to the max speed your character component allows for movement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	float MovementComponentSprintSpeed;
	// The max angle you can look up (90 = straight up)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	float MaxLookUpAngle;
	// The max angle you can look down (90 = stright down)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	float MaxLookDownAngle;
	// Default lean left/right angle
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	float DefaultLeanAngle;
	// How much to lean per incremental lean (each time to incremental lean how far do you lean)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	float IncrementalLeanAmount;
	// How fast you will lean left/right
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	float DefaultLeanSpeed;
	// Ignore, this may be removed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	float MaxCrouchDepth;
	// Ignore this may be removed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	float CrouchDepthIncrement;
	// Used for firearm attaching ensural to prevent attaching issues, can be ignored
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	int32 MaxFirearmAttachmentAttempts;
	// Used for firearm attaching ensural to prevent attaching issues, can be ignored
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	float FirearmReAttachmentAttemptInterval;
	uint8 AttachmentAttempt;
	// How far you can look left/right in freelook
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FreeLook")
	float MaxLookLeftRight;
	// How far you can look up/down in freelook
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FreeLook")
	float MaxLookUpDown;
	
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Animation")
	bool bForceIntoSprintPose;

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FirearmCollision")
	bool bUseFirearmCollision;
	//UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FirearmCollision")
	float FirearmCollisionDistanceCheck;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | FirearmCollision")
	TEnumAsByte<ECollisionChannel> FirearmCollisionChannel;
	bool bFirearmCollisionHitting;

	bool bInThirdPerson;

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
	void OnRep_IsAiming();

	//UPROPERTY(ReplicatedUsing = OnRep_CurrentFirearm)
	AFPSTemplateFirearm* CurrentFirearm;
	//UFUNCTION()
	//void OnRep_CurrentFirearm();

	UPROPERTY(ReplicatedUsing = OnRep_AimingActor)
	AActor* AimingActor;
	UFUNCTION()
	void OnRep_AimingActor();

	UPROPERTY(ReplicatedUsing = OnRep_Lean)
	ELeaning CurrentLean;
	UFUNCTION()
	void OnRep_Lean();
	UPROPERTY(ReplicatedUsing = OnRep_Lean)
	float LeanAngle;
	bool bLeanLeftDown;
	bool bLeanRightDown;
	UPROPERTY(ReplicatedUsing = OnRep_IncrementalLeanAngle)
	float IncrementalLeanAngle;
	UFUNCTION()
	void OnRep_IncrementalLeanAngle();
	bool bIsIncrementalLeaning;
	
	UPROPERTY(Replicated)
	float LeanSpeed;

	UPROPERTY(ReplicatedUsing = OnRep_CrouchHeight)
	float CrouchHeight;
	UFUNCTION()
	void OnRep_CrouchHeight();
	FVector StartMeshLocation;

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
	void Server_SetLeanIncremental(float LeanAmount);
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetLeanSpeed(float NewLeanSpeed);
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
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_CrouchHeight(float NewHeight);
	
public:
	bool HasAuthority() const { return GetOwner() ? GetOwner()->HasAuthority() : false; }
	/**
	 * This function is required to be called by any character it is applied to as it handles everything
	 * @param CameraComponent Plug in your Camera here
	 * @param bAutoAttach If this is true it will auto attach the camera to the set socket in the Character Component settings
	 * @param FirstPersonMesh This is your first person mesh. If your using True First Person then plug in your full body mesh here, if using just Arms then plug that in here
	 * @param ThirdPersonMesh This is for your full body mesh.
	*/
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | CharacterComponent")
	void Init(UCameraComponent* CameraComponent, bool bAutoAttach, USkeletalMeshComponent* FirstPersonMesh, USkeletalMeshComponent* ThirdPersonMesh);
	/**
	 * Returns the current in use mesh
	 * @return This will return the first person mesh if you are locally controlled, and the third person mesh for remote clients
	*/
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
	// Will return the current lean angle of the character
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	float GetLeanAngle()const { return LeanAngle; }
	/**
	 * Set the lean angle of the character to a given value.
	 * Will do nothing if NewLeanAngle is beyond the maximum allowed lean angle
	*/
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void SetLeanAngle(float NewLeanAngle);
	// Resets the characters lean angle to straight
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void ResetLeanAngle();
	// Modifies the characters lean speed
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void SetLeanSpeed(float NewLeanSpeed);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	float GetLeanSpeed() const { return LeanSpeed; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void ResetLeanSpeed();
	// Performs an incremental lean to the amount of Incremental Lean Angle in the given direction
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void IncrementalLean(ELeaning LeanDirection);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	float GetIncrementalLeanAngle() const { return IncrementalLeanAngle; }
	// Returns true if you are currently incremental leaning
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	bool IsIncrementalLeaning() const { return bIsIncrementalLeaning; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	float GetMovementComponentSprintSpeed() const { return MovementComponentSprintSpeed; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsAiming() { return bIsAiming; }
	// Simply returns the owning characters forward vector, just a simple wrapper around the function on AActor
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	FVector GetActorForwardVector() const;
	// Simply returns the owning characters right vector, just a simple wrapper around the function on AActor
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	FVector GetActorRightVector() const;
	// Makes the owning character ragdoll
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void RagdollCharacter();
	// Makes the owning character ragdoll with force applied at the ImpactLocation
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void RagdollCharacterWithForce(const FVector ImpactLocation, const float ImpactForce = 0.0f);
	/**
	 * Sets the system to utilize third person poses for local player if ThirdPerson is true
	 * @param bThirdPerson If true, change the system to use third person poses for local player.
	 * If false, change the system to use first person poses for local player
	*/
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	void SetThirdPersonView(bool bThirdPerson);
	// Returns true if the system is set to use third person poses
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Character")
	bool IsInThirdPerson() const { return bInThirdPerson; }

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void StartAiming();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void StopAiming();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetSprinting(bool Sprinting);
	// Forces you into the sprinting pose to be used with the pose editor
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetForceIntoSprintPose(bool bForce) { bForceIntoSprintPose = bForce; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Procedural")
	bool GetIsSprinting() const { return bIsSprinting; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Procedural")
	bool GetForceIntoSprintPose() const { return bForceIntoSprintPose; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Procedural")
	bool GetIsAiming() const { return bIsAiming; }
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
	// Whether or not to use the firearm collision system. True = use, False = dont use
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetFirearmCollision(bool EnableFirearmCollision) { bUseFirearmCollision = EnableFirearmCollision; }
	/**
	 * Will set the passed in actor as the aiming actor.
	 * Will auto attach the passed in actor to a socket on the characters In Use Mesh at the given socket returned by GetGripSocketName function in Aim Interface
	*/
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetAimingActor(AActor* Actor);
	/**
	 * Sets whether or not the system will use Left Hand IK at all
	 * @param bUse If true it will enable left hand ik, if false it will disable it
	*/
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Procedural")
	void SetUseLeftHandIK(bool bUse);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Procedural")
	bool GetUseLeftHandIK() const { return bUseLeftHandIK; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Procedural")
	EAxis::Type GetRightHandAxis() const { return RightHandAxis; }

	float GetDefaultFOV()const { return DefaultCameraFOV; }
	// Play camera shake to the clients camera
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void PlayCameraShake(TSubclassOf<UCameraShakeBase> CameraShake, float Scale = 1.0f)const;
	// Gets the currently held firearm if it exists, returns nullptr if it doesnt
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	AFPSTemplateFirearm* GetFirearm() const { return CurrentFirearm; }
	// Returns the currently held actor (if its a firearm it will return the firearm)
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	AActor* GetAimingActor() const { return CurrentFirearm ? Cast<AActor>(CurrentFirearm) : AimingActor; }
	/**
	 * Will set the passed in firearm as the current firearm/aiming actor.
	 * Will auto attach the passed in firearm to a socket on the characters In Use Mesh at the given Grip Socket
 	*/
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void EquipFirearm(AFPSTemplateFirearm* Firearm);
	// When aiming it takes the current optics magnification sensitivity and returns a multiplier to be used in Turn/Look up inputs
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	float GetMagnificationSensitivity() const;
	/**
	 * When aiming it takes the current optics magnification sensitivity and returns a multiplier to be used in Turn/Look up inputs.
	 * This only takes affect after the optics magnification exceeds StartAtMagnification
	*/
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	float GetMagnificationSensitivityStartValue(float StartAtMagnification = 4.0f) const;
	// Will attach an actor to your in use mesh at a given socket name
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void AttachItem(AActor* Actor, const FName SocketName);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void ClearCurrentFirearm();
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void SetHighPortPose(bool bSkipNone);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void SetLowPortPose(bool bSkipNone);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void StopLowPortPose();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void StopHighPortPose();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | HighLowPort")
	void StopHighAndLowPortPose();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | HighLowPort")
	EPortPose GetPortPose() const { return PortPose; }

	float LookUpDownOffset;
	// Whether or not to freelook
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | FreeLook")
	void SetFreeLook(bool FreeLook);
	// Returns true if free looking
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

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Crouch")
	void CrouchIncrement(bool Down);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Crouch")
	void GoToCrouch();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Crouch")
	void GoToStanding();

	AFPSTemplate_SightBase* GetCurrentSight() const;
};
