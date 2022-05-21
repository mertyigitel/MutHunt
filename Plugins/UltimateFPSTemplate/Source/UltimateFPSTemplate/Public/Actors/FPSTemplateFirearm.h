//Copyright 2021, Dakota Dawe, All rights reserved
#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "FPSTemplateDataTypes.h"
#include "FirearmParts/BaseClasses/FPSTemplate_LightLaserBase.h"
#include "Interfaces/FPSTemplate_AimInterface.h"
#include "FPSTemplateFirearm.generated.h"

class UFPSTemplate_CharacterComponent;
class AFPSTemplate_Muzzle;
class UFPSTemplate_PartComponent;
class UAnimationAsset;
class UAnimSequence;
class UAnimMontage;
class UCurveVector;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplateFirearm : public AActor, public IFPSTemplate_AimInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplateFirearm();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterComponent)
	UFPSTemplate_CharacterComponent* CharacterComponent;
	UFUNCTION()
	void OnRep_CharacterComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPSTemplate")
	USkeletalMeshComponent* FirearmMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	int32 FirearmIndex;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	FFirearmStats DefaultFirearmStats;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	TArray<EFirearmFireMode> FireModes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	float FireRateRPM;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	int32 BurstCount;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FAimCameraSettings CameraSettings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bUseLeftHandIK;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TEnumAsByte<ECollisionChannel> PoseCollision;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float CameraFOVZoom;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float CameraFOVZoomSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Fire")
	float TimerAutoFireRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName FirearmGripSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FTransform GripSocketOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName MuzzleSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName AimSocket;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName LeftHandIKSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FVector ShoulderStockOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FRotator HeadAimRotation;
	
	UPROPERTY(ReplicatedUsing = OnRep_FireMode, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	EFirearmFireMode FireMode;
	int32 FireModeIndex;
	FTimerHandle TFAHandle;
	uint8 BurstFireCount;
	UFUNCTION()
	void OnRep_FireMode();
	
	UPROPERTY(ReplicatedUsing = OnRep_FirearmHidden, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	bool bFirearmHidden;
	UFUNCTION()
	void OnRep_FirearmHidden() const;

	UPROPERTY(ReplicatedUsing = OnRep_AttachedToSocket)
	FName AttachedToSocket;
	UFUNCTION()
	void OnRep_AttachedToSocket();

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	float ShortStockInterpSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform FirstPersonShortStockPose;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform FirstPersonBasePoseOffset;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform FirstPersonSprintPose;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform FirstPersonHighPortPose;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform FirstPersonLowPortPose;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform ThirdPersonShortStockPose;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform ThirdPersonBasePoseOffset;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform ThirdPersonSprintPose;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform ThirdPersonHighPortPose;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	FTransform ThirdPersonLowPortPose;

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	UCurveVector* ADSCurve;*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	FSwayMultipliers SwayMultipliers;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	FPoseSettings PoseSettings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	FCurveAndShakeSettings CurveAndShakeSettings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	FRecoilData RecoilData;

	UPROPERTY(Replicated)
	FFirearmStats FirearmStats;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FPSTemplate | PartComponents")
	TArray<UFPSTemplate_PartComponent*> PartComponents;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FPSTemplate | PartComponents")
	TArray<UFPSTemplate_PartComponent*> SightComponents;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Animation")
	float DefaultAimSwayMultiplier;
	float AimSwayMultiplier;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentSightComponent)
	UFPSTemplate_PartComponent* CurrentSightComponent;
	UFUNCTION()
	void OnRep_CurrentSightComponent();
	int32 SightComponentIndex;
	
	UPROPERTY(Replicated)
	UFPSTemplate_PartComponent* BarrelComponent;
	UPROPERTY(Replicated)
	UFPSTemplate_PartComponent* HandguardComponent;
	UPROPERTY(Replicated)
	UFPSTemplate_PartComponent* StockComponent;
	UPROPERTY(Replicated)
	TArray<UFPSTemplate_PartComponent*> MagnifierComponents;
	UPROPERTY(Replicated)
	TArray<UFPSTemplate_PartComponent*> LightsLaserComponents;
	UPROPERTY(Replicated)
	TArray<UFPSTemplate_PartComponent*> ComponentsWithRenderTargets;
	// FOR COMPLETE FIREARMS (NO PART BUILDS)
	UPROPERTY(Replicated)
	UFPSTemplate_PartComponent* ForwardGripComponent;
	UPROPERTY(Replicated)
	UFPSTemplate_PartComponent* MuzzleComponent;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void OnRep_Owner() override;
	virtual void OnRep_AttachmentReplication() override;
	void FixPoseTransforms();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CycleSights(UFPSTemplate_PartComponent* SightComponent);

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void PerformProceduralRecoil(float Multiplier = 1.0f, bool PlayCameraShake = true);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetFireMode(EFirearmFireMode NewFireMode);

	//UFPSTemplate_PartComponent* GetSightComponent();

	void HandleSightComponents();

	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Events")
	void OnFireModeChanged();
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Events")
	void OnPartsChanged();
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Events")
	void OnAttachedToSocket(const FName& Socket);

	bool bCanFire;
	bool bCanReFire;
	float TimeSinceLastShot;

	bool bCanCycleSights;

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetHidden(bool Hide);
	
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_AttachToSocket(const FName& Socket);
	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FPSTemplate | Implementable")
	void Fire();
	virtual void Fire_Implementation() { UE_LOG(LogTemp, Warning, TEXT("Fire Implementation Example")); }
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FPSTemplate | Implementable")
	void StopFire();
	virtual void StopFire_Implementation() { UE_LOG(LogTemp, Warning, TEXT("StopFire Implementation Example")); }
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FPSTemplate | Implementable")
	void Reload();
	virtual void Reload_Implementation() { UE_LOG(LogTemp, Warning, TEXT("Reload Implementation Example")); }
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FPSTemplate | Implementable")
	void ReloadDropMagazine();
	virtual void ReloadDropMagazine_Implementation() { UE_LOG(LogTemp, Warning, TEXT("ReloadDropMagazine Implementation Example")); }
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	virtual void CycleFireMode();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	bool IsFirearmDisabled() const { return bCanFire; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	FHitResult MuzzleTrace(float Distance, ECollisionChannel CollisionChannel, bool& bMadeBlockingHit, bool bDrawDebugLine);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	TArray<FHitResult> MuzzleTraceMulti(float Distance, ECollisionChannel CollisionChannel, bool& bMadeBlockingHit, bool bDrawDebugLine);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void HideFirearm(bool Hide);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void AttachToSocket(const FName Socket);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	void DestroyAllParts();

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	AFPSTemplate_SightBase* GetCurrentSight() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	FVector& GetStockOffset() { return ShoulderStockOffset; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	TArray<UFPSTemplate_PartComponent*> GetPartComponents() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	UFPSTemplate_PartComponent* GetGripComponent() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	TArray<UFPSTemplate_PartComponent*> GetLightLaserComponents() const { return LightsLaserComponents; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	UFPSTemplate_PartComponent* GetBarrelComponent() const { return BarrelComponent; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	UFPSTemplate_PartComponent* GetHandguardComponent() const { return HandguardComponent; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	UFPSTemplate_PartComponent* GetStockComponent() const { return StockComponent; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	TArray<UFPSTemplate_PartComponent*> GetMagnifierComponents() const { return MagnifierComponents; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	UFPSTemplate_PartComponent* GetMuzzleComponent() const { return MuzzleComponent; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	UFPSTemplate_PartComponent* GetForwardGripComponent() const { return ForwardGripComponent; }

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
	FProjectileTransform GetMuzzleSocketProjectileTransform() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
	FTransform GetMuzzleSocketTransform();
	// MOA = Minute of angle. 1 MOA = 1 inch of shift at 100 yards
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
	FProjectileTransform GetMuzzleProjectileSocketTransform(float RangeMeters, float MOA = 1.0f);
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void EnableSightCycling() { bCanCycleSights = true; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void DisableSightCycling() { bCanCycleSights = false; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	FTransform GetGripOffset() const { return GripSocketOffset; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	virtual FTransform GetLeftHandIKTransform();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void CycleSights();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void RefreshCurrentSight();
	/*UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	UCurveVector* GetADSCurve() const { return ADSCurve; }*/
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	UCurveVector* GetMovementSwayCurve() const { return CurveAndShakeSettings.MovementSwayCurve; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	bool GetPerformShakeAfterPortPose() const { return CurveAndShakeSettings.PerformShakeAfterPortPose; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	float GetShakeCurveSpeed() const { return CurveAndShakeSettings.ShakeCurveSpeed; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	UCurveVector* GetShakeCurve() const { return CurveAndShakeSettings.ShakeCurve; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	float GetShakeCurveDuration() const { return CurveAndShakeSettings.ShakeCurveDuration; }
	bool UseStatsForMovementSway() const { return CurveAndShakeSettings.ControlMovementSwayByStats; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	float GetStockLengthOfPull() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	float GetAimSwayMultiplier() const { return AimSwayMultiplier; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void SetAimSwayMultiplier(float NewMultiplier) { NewMultiplier >= 0.0f ? AimSwayMultiplier = NewMultiplier : AimSwayMultiplier = DefaultAimSwayMultiplier; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void ResetAimSwayMultiplier() { AimSwayMultiplier = DefaultAimSwayMultiplier; }
	virtual FSwayMultipliers GetSwayMultipliers() const override { return SwayMultipliers; }
	
	virtual FTransform GetSightSocketTransform() override;
	virtual FTransform GetDefaultSightSocketTransform() override;
	virtual bool IsFirearm() const override { return true; }
	virtual int32 GetAnimationIndex() const override { return FirearmIndex; }
	virtual float GetAimInterpolationMultiplier() override;
	virtual float GetRotationLagInterpolationMultiplier() override;
	virtual float GetMovementLagInterpolationMultiplier() override;
	virtual AFPSTemplate_SightBase* GetActiveSight() override { return GetCurrentSight(); }
	virtual void DisableRenderTargets(bool Disable) override { DisableAllRenderTargets(Disable); }
	virtual FName GetGripSocketName() const override { return GetFirearmGripSocket(); }
	virtual FRotator GetHeadRotation() const override { return HeadAimRotation; }

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Stats")
	void UpdateFirearmStats();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Stats")
	FFirearmStats& GetFirearmStats() { return FirearmStats; }

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	UAnimSequence* GripAnimation;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	int32 GetFirearmIndex() const { return FirearmIndex; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	USkeletalMeshComponent* GetMesh() const { return FirearmMesh; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	UFPSTemplate_CharacterComponent* GetCharacterComponent();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	EFirearmFireMode GetFireMode() const { return FireMode; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsSuppressed();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	virtual FAimCameraSettings& GetCameraSettings() override;

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void CycleMagnifier();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void UseLightLaser(ELightLaser Toggle);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void CycleLaserColor();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	AFPSTemplate_Muzzle* GetMuzzleDevice();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	AActor* GetMuzzleActor();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	TArray<AFPSTemplate_PartBase*> GetFirearmParts() const;
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void IncreaseReticleBrightness(bool Increase);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Sight")
	float GetOpticMagnification() const;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void PointOfImpactUp(bool Up);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void PointOfImpactLeft(bool Left);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void ReturnSightToZero(EElevationWindage Dial);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void SetSightZero(EElevationWindage Dial);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Sight")
	FSightZero GetSightZero() const;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void CycleReticle();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void DisableAllRenderTargets(bool Disable);
	
	void PartsChanged();

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Pose")
	FTransform& GetSprintPose();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Pose")
	FTransform& GetHighPortPose();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Pose")
	FTransform& GetLowPortPose();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Pose")
	FTransform& GetBasePoseOffset();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Pose")
	FTransform& GetShortStockPose();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Pose")
	float GetShortStockInterpSpeed() const { return ShortStockInterpSpeed; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Pose")
	float GetPortPoseInterpolationSpeed() const { return PoseSettings.PortPoseInterpolationSpeed; }
	virtual void ZoomOptic(bool bZoom) override;
	virtual float GetCurrentMagnification() const override { return GetOpticMagnification(); }

	bool UseLeftHandIK() const { return bUseLeftHandIK; }

	const FName& GetAimSocket() const { return AimSocket; }

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Init")
	void SetCharacterComponent(UFPSTemplate_CharacterComponent* INCharacterComponent);

	void AddPartCache(UFPSTemplate_PartComponent* PartComponent);
	
	class UAnimSequence* GetGripAnimation()const;

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Sockets")
	const FName& GetFirearmGripSocket() const { return FirearmGripSocket; }

	FRecoilData& GetRecoilData() { return RecoilData; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FPSTemplate | Implementable")
	void Equip();
	void Equip_Implementation() {}
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FPSTemplate | Implementable")
	void UnEquip();
	void UnEquip_Implementation() {}
};