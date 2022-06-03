//Copyright 2021, Dakota Dawe, All rights reserved
#pragma once

#include "CoreMinimal.h"
#include "FPSTemplateDataTypes.h"
#include "Animation/SkeletalMeshActor.h"
#include "FirearmParts/BaseClasses/FPSTemplate_LightLaserBase.h"
#include "Interfaces/FPSTemplate_AimInterface.h"
#include "Interfaces/FPSTemplate_AttachmentInterface.h"
#include "Components/FPS_FirearmStabilizerComponent.h"
#include "FPSTemplateFirearm.generated.h"

class UFPSTemplate_CharacterComponent;
class AFPSTemplate_Muzzle;
class UFPSTemplate_PartComponent;
class UAnimationAsset;
class UAnimSequence;
class UAnimMontage;
class UCurveVector;
class UNiagaraSystem;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplateFirearm : public ASkeletalMeshActor, public IFPSTemplate_AimInterface, public IFPSTemplate_AttachmentInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplateFirearm(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterComponent)
	UFPSTemplate_CharacterComponent* CharacterComponent;
	UFUNCTION()
	void OnRep_CharacterComponent();

	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate")
	USkeletalMeshComponent* FirearmMesh;
	// Used to blend between different state machines in the anim graph using an integer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	int32 FirearmAnimationIndex;
	// Used to blend between different state machines in the anim graph using gameplay tags
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FGameplayTag FirearmAnimationGameplayTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	FFirearmStats DefaultFirearmStats;
	// Array of fire modes that you can cycle between. This is replicated but not implemented into anything by default aside from example m4
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	TArray<EFirearmFireMode> FireModes;
	// Fire rate in Rounds Per Minute. This is not implemented into anything by default aside from example firearms
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	float FireRateRPM;
	// Burst counter for 3/4 round burst. This is not implemented into anything by default aside from example m4
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	int32 BurstCount;
	// Random niagara systems for this muzzle, to be used for shooting effects like muzzle flash
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	TArray<UNiagaraSystem*> FireNiagaraSystems;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	FAimCameraSettings CameraSettings;
	// Whether or not to use left hand ik with this firearm
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bUseLeftHandIK;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TEnumAsByte<ECollisionChannel> PoseCollision;
	// Whether or not to spawn Default Part from the part components when you construct a firearm from a string
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bSpawnDefaultPartsFromPreset;
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float CameraFOVZoom;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	float CameraFOVZoomSpeed;*/

	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Fire")
	float TimerAutoFireRate;

	// Socket for the firearm to attach to on your character
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName FirearmGripSocket;
	// The offset for the grip so you can control how the firearm is positioned in your hand on a per firearm basis
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FTransform GripSocketOffset;
	// Muzzle socket for the firearm in cases of not using attachments for barrel/muzzle devices
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName MuzzleSocket;
	// Default aim socket such as a mesh with iron sights
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName AimSocket;
	// Where to aim when point aiming, uses sockets on the firearm
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	TArray<FName> PointAimSockets;
	// Where the left hand will go if using left hand IK
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Sockets")
	FName LeftHandIKSocket;

	// How much faster should you point aim compared to normal aiming down sights
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	float PointAimADSInterpolationMultiplier;
	// Make un aiming the same speed as aiming down sights
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	bool bUnAimMultiplierSameAsADS;
	// How much faster should you un aim compared to normal aiming down sights
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming", meta = (EditCondition = "!bUnAimMultiplierSameAsADS"))
	float UnAimInterpolationMultiplier;
	// When aiming for remote clients (you looking at someone else), adjust where you want the firearm to go (Example: To the right shoulder)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FVector ShoulderStockOffset;
	// When aiming for remote clients (you looking at someone else), how much should the head lean
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Aiming")
	FRotator HeadAimRotation;
	
	UPROPERTY(ReplicatedUsing = OnRep_FireMode, BlueprintReadOnly, Category = "FPSTemplate | Firearm")
	EFirearmFireMode FireMode;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Firearm")
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

	// How quickly during firearm collision should you interpolate to the short stock pose
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Poses")
	float ShortStockInterpSpeed;
	// If you dont know what short stocking is, google it
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
	float RotationLagMultiplier;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	float MovementLagMultiplier;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	FPoseSettings PoseSettings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	FCurveAndShakeSettings CurveAndShakeSettings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Animation")
	FRecoilData RecoilData;

	UPROPERTY(Replicated)
	FFirearmStats FirearmStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Animation")
	float DefaultSwayMultiplier;
	float SwayMultiplier;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentSightComponent)
	UFPSTemplate_PartComponent* CurrentSightComponent;
	UFUNCTION()
	void OnRep_CurrentSightComponent();
	int32 SightComponentIndex;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentSightComponent)
	int32 PointAimIndex;
	UFUNCTION()
	void OnRep_PointAimIndex();

	
	UPROPERTY(Replicated)
	FFirearmPartComponentsOwnerRep FirearmPartComponentsOwner;
	// Replicates To All
	UPROPERTY(Replicated)
	FFirearmPartComponentsRep FirearmPartComponents;
	UFUNCTION(BlueprintImplementableEvent)
	void OnPartComponentAdded(UFPSTemplate_PartComponent* PartComponent);
	
	TWeakObjectPtr<UFPS_FirearmStabilizerComponent> StabilizerComponent;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void OnRep_Owner() override;
	virtual void OnRep_AttachmentReplication() override;
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | AssetID")
	FPrimaryAssetType AssetType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | AssetID")
	FName AssetName;
	
	virtual void FixPoseTransforms(FTransform& FirstPerson, FTransform& ThirdPerson);

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
	bool bShouldSpawnDefaultsFromPreset;

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetHidden(bool Hide);
	
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_AttachToSocket(const FName& Socket);

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetPointAiming(int32 Index);
	
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
	virtual void CycleFireMode(bool bReverse = false);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	bool IsFirearmDisabled() const { return !bCanFire; }
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

	void SetIsLoadedFromPreset() { bShouldSpawnDefaultsFromPreset = bSpawnDefaultPartsFromPreset; }
	
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	bool GetShouldSpawnDefaultOnPreset() const { return bShouldSpawnDefaultsFromPreset; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	AFPSTemplate_SightBase* GetCurrentSight() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	FVector& GetStockOffset() { return ShoulderStockOffset; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	TArray<UFPSTemplate_PartComponent*> GetAllPartComponents();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	UFPSTemplate_PartComponent* GetGripComponent();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	TArray<UFPSTemplate_PartComponent*> GetPartComponents(EPartOwnerIndex Part) { return FirearmPartComponentsOwner.GetPartComponents(Part); }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Attachments")
	UFPSTemplate_PartComponent* GetPartComponent(EPartIndex Part) { return FirearmPartComponents.GetPartComponent(Part); }
	
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
	FTransform GetMuzzleSocketTransform();
	// MOA = Minute of angle. 1 MOA = 1 inch of shift at 100 yards
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
	FProjectileTransform GetMuzzleProjectileSocketTransform(float RangeMeters, float MOA = 1.0f);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
    TArray<FProjectileTransform> GetMultipleMuzzleProjectileSocketTransforms(float RangeMeters, float InchSpreadAt25Yards = 40.0f, uint8 ShotCount = 4);
	
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
	float GetStockLengthOfPull();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	float GetSwayMultiplier() const { return SwayMultiplier; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void SetSwayMultiplier(float NewMultiplier) { NewMultiplier >= 0.0f ? SwayMultiplier = NewMultiplier : SwayMultiplier = DefaultSwayMultiplier; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Animation")
	void ResetSwayMultiplier() { SwayMultiplier = DefaultSwayMultiplier; }
	
	virtual FSwayMultipliers GetSwayMultipliers_Implementation() override { return SwayMultipliers; }
	virtual FTransform GetSightSocketTransform_Implementation() override;
	virtual FTransform GetDefaultSightSocketTransform_Implementation() override;
	virtual bool IsFirearm_Implementation() const override { return true; }
	virtual int32 GetAnimationIndex_Implementation() const override { return FirearmAnimationIndex; }
	virtual FGameplayTag GetAnimationGameplayTag_Implementation() const override { return FirearmAnimationGameplayTag; }
	virtual float GetAimInterpolationMultiplier_Implementation() override;
	virtual float GetUnAimInterpolationMultiplier_Implementation() override;
	virtual float GetRotationLagInterpolationMultiplier_Implementation() override;
	virtual float GetMovementLagInterpolationMultiplier_Implementation() override;
	virtual AFPSTemplate_SightBase* GetActiveSight_Implementation() override { return GetCurrentSight(); }
	virtual void DisableRenderTargets_Implementation(bool Disable) override { DisableAllRenderTargets(Disable); }
	virtual FName GetGripSocketName_Implementation() const override { return GetFirearmGripSocket(); }
	virtual FRotator GetHeadRotation_Implementation() const override { return HeadAimRotation; }
	virtual void GetCameraSettings_Implementation(FAimCameraSettings& OutCameraSettings) override;
	virtual void ZoomOptic_Implementation(bool bZoom) override;
	virtual float GetCurrentMagnification_Implementation() const override { return GetOpticMagnification(); }

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Stats")
	void UpdateFirearmStats();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Stats")
	FFirearmStats& GetFirearmStats() { return FirearmStats; }

	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	UAnimSequence* GripAnimation;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	int32 GetFirearmIndex() const { return FirearmAnimationIndex; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	USkeletalMeshComponent* GetMesh() const { return FirearmMesh; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	UFPSTemplate_CharacterComponent* GetCharacterComponent();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	EFirearmFireMode GetFireMode() const { return FireMode; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsSuppressed();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	UNiagaraSystem* GetFireNiagaraSystem();

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void SetPointAimIndex(int32 Index);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void CyclePointAim();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	bool IsPointAiming() const { return PointAimIndex > INDEX_NONE; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void CycleMagnifier();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void UseLightLaser(ELightLaser Toggle, bool bSync = false);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void CycleLightLaserModes();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	void CycleLaserColor();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	AFPSTemplate_Muzzle* GetMuzzleDevice();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	AActor* GetMuzzleActor();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	TArray<AFPSTemplate_PartBase*> GetFirearmParts();
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void IncreaseReticleBrightness(bool Increase);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Sight")
	float GetOpticMagnification() const;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void PointOfImpactUp(bool Up, uint8 Clicks = 1);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void PointOfImpactLeft(bool Left, uint8 Clicks = 1);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void ReturnSightToZero(EElevationWindage Dial);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void SetSightZero(EElevationWindage Dial);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Sight")
	FSightZero GetSightZero();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void CycleReticle();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Sight")
	void DisableAllRenderTargets(bool Disable);

	virtual void AddPartCache_Implementation(UFPSTemplate_PartComponent* PartComponent) override;
	virtual void SetSight_Implementation(UFPSTemplate_PartComponent* SightComponent) override;
	virtual void PartsChanged_Implementation() override;

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
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Pose")
	void SetShortStockInterpSpeed(float Speed);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Pose")
	void SetShortStockPose(const FTransform& Transform, bool bFirstPerson);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Pose")
	void SetBasePoseOffset(const FTransform& Transform, bool bFirstPerson);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Pose")
	void SetSprintPose(const FTransform& Transform, bool bFirstPerson);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Pose")
	void SetHighPortPose(const FTransform& Transform, bool bFirstPerson);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Pose")
	void SetLowPortPose(const FTransform& Transform, bool bFirstPerson);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Pose")
	float GetShortStockInterpSpeed() const { return ShortStockInterpSpeed; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Pose")
	float GetPortPoseInterpolationSpeed() const { return PoseSettings.PortPoseInterpolationSpeed; }

	bool UseLeftHandIK() const { return bUseLeftHandIK; }

	const FName& GetAimSocket() const { return AimSocket; }

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Init")
	void SetCharacterComponent(UFPSTemplate_CharacterComponent* INCharacterComponent);
	
	UAnimSequence* GetGripAnimation();

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Sockets")
	const FName& GetFirearmGripSocket() const { return FirearmGripSocket; }

	FRecoilData& GetRecoilData() { return RecoilData; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FPSTemplate | Implementable")
	void Equip();
	void Equip_Implementation() {}
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FPSTemplate | Implementable")
	void UnEquip();
	void UnEquip_Implementation() {}

	void SetupStabilizerComponent();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Stabilizer")
	UFPS_FirearmStabilizerComponent* GetStabilizerComponent();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Stabilizer")
	bool IsStabilized() const;

	void ActivateCurrentSight(bool bActivate) const;
};