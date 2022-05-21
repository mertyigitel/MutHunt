//Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "Templates/SubclassOf.h"
#include "Engine/NetSerialization.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Particles/WorldPSCPool.h"
#include "Particles/ParticleSystem.h"
#include "Curves/CurveVector.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"
#include "FPSTemplateDataTypes.generated.h"

UENUM(BlueprintType)
enum class EMagazineState : uint8
{
	InUse		UMETA(DisplayName, "InUse"),
	NotInUse	UMETA(DisplayName, "NotInUse"),
	OnGround	UMETA(DisplayName, "OnGround")
};

UENUM(BlueprintType)
enum class ELightLaser : uint8
{
	Light	UMETA(DisplayName, "Light"),
	Laser	UMETA(DisplayName, "Laser"),
	Both	UMETA(DisplayName, "Both")
};

UENUM(BlueprintType)
enum class EFirearmFireMode : uint8
{
	Safe			UMETA(DisplayName, "Safe"),
	Semi			UMETA(DisplayName, "Semi"),
	Burst			UMETA(DisplayName, "Burst"),
	FullAuto		UMETA(DisplayName, "FullAuto"),
	BoltAction		UMETA(DisplayName, "BoltAction")
};

UENUM(BlueprintType)
enum class ELeaning : uint8
{
	None	UMETA(DisplayName, "None"),
	Left	UMETA(DisplayName, "Left"),
	Right	UMETA(DisplayName, "Right")
};

UENUM(BlueprintType)
enum class EPortPose : uint8
{
	None	UMETA(DisplayName, "None"),
	High	UMETA(DisplayName, "High"),
	Low		UMETA(DisplayName, "Low")
};

UENUM(BlueprintType)
enum class EPartType : uint8
{
	Sight			UMETA(DisplayName, "Sight"),
	Magnifier		UMETA(DisplayName, "Magnifier"),
	LightLaser		UMETA(DisplayName, "LightLaser"),
	Handguard		UMETA(DisplayName, "Handguard"),
	Barrel			UMETA(DisplayName, "Barrel"),
	MuzzleDevice	UMETA(DisplayName, "MuzzleDevice"),
	Stock			UMETA(DisplayName, "Stock"),
	PistolGrip		UMETA(DisplayName, "PistolGrip"),
	ForwardGrip		UMETA(DisplayName, "ForwardGrip"),
	Other			UMETA(DisplayName, "Other")
};

UENUM(BlueprintType)
enum class EFirearmInHand : uint8
{
	None		UMETA(DisplayName, "None"),
	M4			UMETA(DisplayName, "M4"),
	Test		UMETA(DisplayName, "Test"),
};

UENUM(BlueprintType)
enum class EMeasurementType : uint8
{
	Metric		UMETA(DisplayName, "Metric"),
	Imperial	UMETA(DisplayName, "Imperial")
};

UENUM(BlueprintType)
enum class EElevationWindage : uint8
{
	Elevation	UMETA(DisplayName, "Elevation"),
	Windage		UMETA(DisplayName, "Windage"),
	Both		UMETA(DisplayName, "Both")
};

UENUM(BlueprintType)
enum class EScopeAdjustment : uint8
{
	MRAD	UMETA(DisplayName, "MRAD"),
	MOA		UMETA(DisplayName, "MOA")
};

class UMaterialInstance;
class UMaterialInstanceDynamic;
class UFXSystemAsset;
class USoundBase;

USTRUCT(BlueprintType)
struct FHoleMaterial
{
	GENERATED_BODY()
	UMaterialInstanceDynamic* MaterialInstance = nullptr;
	TArray<FVector> HoleLocations;
	uint8 HoleIndex = 0;
	uint8 MaxHoleCount = 8;
};

USTRUCT(BlueprintType)
struct FHoleMaterialSetting
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate")
	int32 MaxHoleCount = 8;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate")
	int32 MaterialIndex = 0;
};

USTRUCT(BlueprintType)
struct FAimCameraSettings
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float CameraFOVZoom;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float CameraFOVZoomSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float CameraDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	bool bUsedFixedCameraDistance;
};

USTRUCT(BlueprintType)
struct FFirearmPartStats
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float Weight = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float ErgonomicsChangePercentage = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float RecoilChangePercentage = 0.0f;

	FFirearmPartStats operator+ (const FFirearmPartStats& Stats) const
	{
		FFirearmPartStats NewStats;
		NewStats.Weight = this->Weight + Stats.Weight;
		NewStats.ErgonomicsChangePercentage = this->ErgonomicsChangePercentage + Stats.ErgonomicsChangePercentage;
		NewStats.RecoilChangePercentage = this->RecoilChangePercentage + Stats.RecoilChangePercentage;

		return NewStats;
	}
	
	FFirearmPartStats& operator+= (const FFirearmPartStats& Stats)
	{
		this->Weight += Stats.Weight;
		this->ErgonomicsChangePercentage += Stats.ErgonomicsChangePercentage;
		this->RecoilChangePercentage += Stats.RecoilChangePercentage;

		return *this;
	}
};

USTRUCT(BlueprintType)
struct FFirearmStats
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float Weight = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float Ergonomics = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float RecoilMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FSightData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "FPSTemplate")
	class AFPSTemplate_SightBase* Sight = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "FPSTemplate")
	FName Socket = NAME_None;

	bool operator==(const FSightData& SightData) const
	{
		return this->Sight == SightData.Sight;
	}
};

USTRUCT(BlueprintType)
struct FFirearmPartData
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	TSubclassOf<class AFPSTemplate_PartBase> PartClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	class UTexture2D* PartImage = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	FString PartName;
};

USTRUCT(BlueprintType)
struct FReticleMaterial
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | ReticleMaterial")
	UMaterialInstance* ReticleMaterial = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | ReticleMaterial")
	UMaterialInstanceDynamic* DynamicReticleMaterial = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | ReticleMaterial")
	float ReticleSize = 1.0f;
};

USTRUCT(BlueprintType)
struct FReticleBrightness
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | ReticleBrightness")
	TArray<float> ReticleBrightnessSettings = { 1.0f };
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | ReticleBrightness")
	int32 ReticleDefaultBrightnessIndex = 0;
};

USTRUCT(BlueprintType)
struct FReticleSettings
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | ReticleSettings")
	int32 ReticleMaterialIndex = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | ReticleSettings")
	TArray<FReticleMaterial> ReticleMaterials;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | ReticleSettings")
	FReticleBrightness ReticleBrightness;
};

USTRUCT(BlueprintType)
struct FSightZero
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float Elevation = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float Windage = 0.0f;
};

USTRUCT(BlueprintType)
struct FSightMagnification
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | MagnificationSettings")
	TArray<float> Magnifications = { 1.0f };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | MagnificationSettings")
	bool bIsFirstFocalPlane = true;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | MagnificationSettings")
	float DecreaseReticleScaleAmount = 8.0f;

	uint8 MagnificationIndex = 0;
	float CurrentMagnification = 1.0f;
};

USTRUCT(BlueprintType)
struct FSightOptimization
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization")
	float ScopeRefreshRate = 60.0f;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization")
	bool bDisableWhenNotAiming = true;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization", meta = (EditCondition = "!bDisableWhenNotAiming"))
	float NotAimingRefreshRate = 5.0f;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization", meta = (EditCondition = "bDisableWhenNotAiming"))
	bool bClearScopeWithColor = true;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization", meta = (EditCondition = "bDisableWhenNotAiming && bClearScopeWithColor"))
	FLinearColor ScopeClearedColor = FLinearColor::Black;;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization", meta = (EditCondition = "bDisableWhenNotAiming && !bClearScopeWithColor"))
	bool bClearScopeWithMaterial = false;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization", meta = (EditCondition = "bDisableWhenNotAiming && !bClearScopeWithColor && bClearScopeWithMaterial"))
	UMaterialInterface* ClearedScopeMaterial = nullptr;
	
	UMaterialInterface* OriginalScopeMaterial = nullptr;
};

USTRUCT(BlueprintType)
struct FSoundSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	USoundBase* Sound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	float VolumeMultiplier = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	float PitchMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FEffectImpactSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	UFXSystemAsset* ImpactEffect = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	UMaterialInstance* ImpactDecal = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	EPSCPoolMethod PoolingMethod = EPSCPoolMethod::AutoRelease;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	bool bUseParticlesRotation = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	FVector DecalSize = FVector(2.5f, 2.5f, 2.5f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	float DecalLifeTime = 8.0f;
};

USTRUCT(BlueprintType)
struct FImpactEffects
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	FEffectImpactSettings EffectSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	FSoundSettings SoundSettings;
};

USTRUCT(BlueprintType)
struct FFirearmPartList
{
	GENERATED_BODY()
	UPROPERTY()
	FString ComponentName;
	UPROPERTY()
	TSubclassOf<class AFPSTemplateFirearm> ParentFirearm;
	UPROPERTY()
	TSubclassOf<class AFPSTemplate_PartBase> Parent;
	UPROPERTY()
	TSubclassOf<class AFPSTemplate_PartBase> Part;
	UPROPERTY()
	float PartOffset = 0.0f;
	
	bool bHasBeenCreated = false;
};

USTRUCT(BlueprintType)
struct FFirearm
{
	GENERATED_BODY()
	UPROPERTY()
	TSubclassOf<class AFPSTemplateFirearm> FirearmClass;
	UPROPERTY()
	TArray<FFirearmPartList> PartsList;
};

USTRUCT(BlueprintType)
struct FProjectileTransform
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "FPSTemplate | Projectile")
	FVector Location;
	UPROPERTY(BlueprintReadWrite, Category = "FPSTemplate | Projectile")
	FRotator Rotation;

	FProjectileTransform() {Location = FVector_NetQuantize100(); Rotation = FRotator();}
	FProjectileTransform(const FVector& INLocation, const FRotator& INRotation)
	{
		Location = FVector_NetQuantize100(INLocation);
		//Rotation = FVector_NetQuantize100(INRotation.Pitch, INRotation.Yaw, INRotation.Roll);
		Rotation = INRotation;
	}
	FProjectileTransform(const FTransform& INTransform)
	{
		//Location = FVector_NetQuantize100(INTransform.GetLocation());
		Location = INTransform.GetLocation();
		Rotation = INTransform.Rotator();
		//Rotation = FVector_NetQuantize100(INTransform.Rotator().Pitch, INTransform.Rotator().Yaw, INTransform.Rotator().Roll);
	}
	
	static FTransform GetTransformFromProjectile(const FProjectileTransform& ProjectileTransform)
	{
		//FVector_NetQuantize100 Rot = ProjectileTransform.Rotation;
		FVector Loc = ProjectileTransform.Location;
		//return FTransform(FRotator(ProjectileTransform.Rotation.X, ProjectileTransform.Rotation.Y, ProjectileTransform.Rotation.Z).Quaternion(), ProjectileTransform.Location, FVector::OneVector);
		return FTransform(ProjectileTransform.Rotation, ProjectileTransform.Location, FVector::OneVector);
	}
	FTransform GetTransformFromProjectile() const
	{
		return FTransform(Rotation, Location, FVector::OneVector);
		//return FTransform(FRotator(Rotation.X, Rotation.Y, Rotation.Z).Quaternion(), Location, FVector::OneVector);
	}
};

USTRUCT(BlueprintType)
struct FMinMax
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	float Min = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	float Max = 0.0f;

	FMinMax() {}
	FMinMax(float Minimum, float Maximum)
	{
		Min = Minimum;
		Max = Maximum;
	}
};

USTRUCT(BlueprintType)
struct FSwayMultipliers
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float MovementRollMultiplier = 1.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float MovementPitchMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FPoseSettings
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float AimInterpolationMultiplier = 1.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float PortPoseInterpolationSpeed = 25.0f;
};

USTRUCT(BlueprintType)
struct FCurveAndShakeSettings
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	UCurveVector* ShakeCurve = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float ShakeCurveDuration = 3.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	UCurveVector* MovementSwayCurve = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	bool PerformShakeAfterPortPose = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float ShakeCurveSpeed = 8.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	bool ControlMovementSwayByStats = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	bool ScaleCameraShakeByStats = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	TSubclassOf<UCameraShakeBase> FireCameraShake;
};

USTRUCT(BlueprintType)
struct FRecoilData
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	UCurveVector* RecoilLocationCurve;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	FMinMax RecoilLocationRandomness = FMinMax(1.0f, 2.0f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	UCurveVector* RecoilRotationCurve;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	FMinMax RecoilPitchRandomness = FMinMax(1.0f, 2.0f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	FMinMax RecoilRollRandomness = FMinMax(-3.0f, 3.0f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	FMinMax RecoilYawRandomness = FMinMax(-8.0f, 8.0f);

	FVector GetFixedLocationVector(const float Time)
	{
		if (RecoilLocationCurve)
		{
			FVector Loc = RecoilLocationCurve->GetVectorValue(Time);
		}
		return FVector::ZeroVector;
	}

	FVector GetFixedRotationVector(const float Time)
	{
		if (RecoilRotationCurve)
		{
			FVector Rot = RecoilRotationCurve->GetVectorValue(Time);
		}
		return FVector::ZeroVector;
	}
};