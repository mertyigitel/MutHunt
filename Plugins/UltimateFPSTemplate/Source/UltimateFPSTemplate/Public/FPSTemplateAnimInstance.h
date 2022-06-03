//Copyright 2021, Dakota Dawe, All rights reserved
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FPSTemplateDataTypes.h"
#include "GameplayTagContainer.h"

#include "FPSTemplateAnimInstance.generated.h"

class AFPSTemplateFirearm;
class UFPSTemplate_CharacterComponent;
class UAnimSequence;

UCLASS()
class ULTIMATEFPSTEMPLATE_API UFPSTemplateAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
		
public:
	UFPSTemplateAnimInstance();

	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Toggles")
	bool bUseProceduralSpine;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Toggles", meta = (EditCondition = "bUseProceduralSpine"))
	int32 SpineBoneCount;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Toggles")
	bool bUseLeftHandIK;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Toggles")
	bool bUseBasePoseCorrection;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Toggles", meta = (EditCondition = "bUseBasePoseCorrection"))
	float BasePoseCorrectionBlend;
	
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Default")
	UFPSTemplate_CharacterComponent* CharacterComponent;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Default")
	float CharacterDirection;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Default")
	float CharacterVelocity;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	int32 AnimationIndex;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	FGameplayTag AnimationGameplayTag;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	AActor* AimingActor;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	AFPSTemplateFirearm* Firearm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPSTemplate | Default")
	UAnimSequence* LeftHandPose;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bValidLeftHandPose;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	float AimInterpolationSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	float CycleSightsInterpolationSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	float RotationLagResetInterpolationSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	float MotionLagResetInterpolationSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FName RightHandBone;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Crouch")
	bool bEnableProceduralCrouch;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Crouch", meta = (EditCondition = "bEnableProceduralCrouch"))
	float CrouchInterpSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Crouch", meta = (EditCondition = "bEnableProceduralCrouch"))
	FName LeftFootBone;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Crouch", meta = (EditCondition = "bEnableProceduralCrouch"))
	FName RightFootBone;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FVector RelativeToHandLocation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FRotator RelativeToHandRotation;
	FTransform RelativeToHandTransform;
	FTransform FinalRelativeHand;
	bool bInterpRelativeToHand;

	bool bFirstRun;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FVector SightLocation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FRotator SightRotation;

	float SightDistance;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	float AimingAlpha;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	bool bIsAiming;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FRotator HeadAimingRotation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sway")
	bool bInvertRotationLag;

	void SetSightTransform();
	void SetRelativeToHand();
	void InterpRelativeToHand(float DeltaSeconds);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | LeftHandIK")
	FTransform LeftHandIKTransform;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | LeftHandIK")
	float LeftHandIKAlpha;
	void SetLeftHandIK();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Actions")
	float RotationAlpha;
	
	bool bInterpAiming;
	bool bInterpCameraZoom;
	void InterpCameraZoom(float DeltaSeconds);
	float DefaultCameraFOV;

	void InterpAimingAlpha(float DeltaSeconds);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Lag")
	FVector RotationLagLocation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Lag")
	FRotator RotationLagRotation;
	FTransform RotationLagTransform;
	FTransform UnmodifiedRotationLagTransform;
	FRotator OldRotation;
	void SetRotationLag(float DeltaSeconds);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Lag")
	FVector MovementLagLocation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Lag")
	FRotator MovementLagRotation;

	void SetMovementLag(float DeltaSeconds);
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bIsLocallyControlled;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FFirearmStats FirearmStats;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FRotator SpineRotation;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Recoil")
	FVector RecoilLocation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Recoil")
	FRotator RecoilRotation;
	FTransform RecoilTransform;
	FTransform FinalRecoilTransform;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Recoil")
	FVector BasePoseOffsetLocation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Recoil")
	FRotator BasePoseOffsetRotation;

	bool bInterpRecoil;
	float RecoilStartTime;
	float RecoilMultiplier;

	void RecoilInterpToZero(float DeltaSeconds);
	void RecoilInterpTo(float DeltaSeconds);

	ELeaning CurrentLean;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FRotator LeanRotation;
	bool bInterpLeaning;
	void InterpLeaning(float DeltaSeconds);
	
	float CurveTimer;
	float VelocityMultiplier;
	void HandleMovementSway(float DeltaSeconds);
	void HandleSprinting();
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FVector SwayLocation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FRotator SwayRotation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FVector SprintPoseLocation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FRotator SprintPoseRotation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float SprintAlpha;
	bool bSprinting;

	float SwayMultiplier;

	bool bCustomizingFirearm;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FTransform WeaponCustomizingTransform;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float WeaponCustomizingAlpha;

	bool bCanAim;

	bool bInterpPortPose;
	EPortPose PortPose;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float PortPoseAlpha;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Poses")
	FVector CurrentPoseLocation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Poses")
	FRotator CurrentPoseRotation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Poses")
	FVector ShortStockPoseLocation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Poses")
	FRotator ShortStockPoseRotation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Poses")
	float ShortStockPoseAlpha;
	bool bInCollisionPose;

	FTransform DefaultRelativeToHand;

	void InterpPortPose(float DeltaSeconds);
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FRotator FreeLookRotation;
	FRotator FreeLookStartRotation;
	bool bFreeLook;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FRotator ShakeRotation;
	bool bInterpShakeCurve;
	void InterpShakeCurve(float DeltaSeconds);
	float ShakeCurveStartTime;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float ShakeCurveAlpha;
	bool bCanPlayShakeCurve;
	
	FRotator SpineToInterpTo;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Poses")
	FVector CustomPoseLocation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Poses")
	FRotator CustomPoseRotation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Poses")
	float CustomPoseAlpha;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Curve")
	FVector CustomCurveLocation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Curve")
	FRotator CustomCurveRotation;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FPSTemplate | Curve")
	float CustomCurveAlpha;
	FCurveData CustomCurveData;
	float CustomCurveStartTime;
	bool bInterpCustomCurve;
	void InterpCustomCurve(float DeltaSeconds);

	// Returns true if you are the one controlling the actor of this AnimBP
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Extra")
	bool IsLocallyControlled();

	bool bInterpCrouch;
	float CrouchHeightInterpTo;
	float CurrentCrouchHeight;
	FVector MeshStartLocation;
	void InterpToNewCrouch(float DeltaSeconds);
	
public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	void SetIsAiming(bool IsAiming);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	void CycledSights();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	void PlayFirearmShakeCurve(bool ManuallyPlay = false);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	void PlayCustomCurve(FCurveData INCurveData);
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void EnterCustomPose(const FTransform& Pose);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void ClearCustomPose();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void SetLeaning(ELeaning Lean);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void SetAnimationIndex(int32 NewAnimationIndex) { AnimationIndex = NewAnimationIndex; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void SetAnimationGameplayTag(FGameplayTag GameplayTag) { AnimationGameplayTag = GameplayTag; }
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Actions")
	void SetIsReloading(bool IsReloading, float BlendAlpha = 0.35f);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Actions")
	void SetCanAim(bool CanAim);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Actions")
	void PerformRecoil(float Multiplier = 1.0f);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Actions")
	void ChangingFireMode(bool IsChanging);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Actions")
	void EnableLeftHandIK(bool Enable) { Enable ? LeftHandIKAlpha = 1.0f : LeftHandIKAlpha = 0.0f; }
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default")
	void StopMontages(float BlendOutTime);
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Extra")
	void SetCustomization(bool CustomizeFirearm);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Extra")
	void SetPortPose(EPortPose Pose);
	void SetPortPoseBlend(EPortPose Pose, float Alpha);
	bool HandleFirearmCollision(EPortPose Pose, float Alpha);

	void SetCharacterComponent(UFPSTemplate_CharacterComponent* INCharacterComponent) { CharacterComponent = INCharacterComponent;}

	void SetFreeLook(bool FreeLook);

	void InterpolateCrouch(FVector MeshStartingLocation, float NewCrouchHeight);
};
