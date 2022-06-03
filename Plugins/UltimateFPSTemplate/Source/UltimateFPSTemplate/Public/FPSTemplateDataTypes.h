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
#include "Components/FPSTemplate_PartComponent.h"
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
	Off		UMETA(DisplayName, "Off"),
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
	Manual			UMETA(DisplayName, "Manual")
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

UENUM(BlueprintType)
enum class EPose : uint8
{
	ShortStockPose	UMETA(DisplayName, "ShortStockPose"),
	BasePoseOffset	UMETA(DisplayName, "BasePoseOffset"),
	SprintPose		UMETA(DisplayName, "SprintPose"),
	HighPortPose	UMETA(DisplayName, "HighPortPose"),
	LowPortPose		UMETA(DisplayName, "LowPortPose")
};

// Forward Declarations
class UMaterialInstance;
class UMaterialInstanceDynamic;
class UFXSystemAsset;
class USoundBase;
class UFPSTemplate_PartComponent;
class AFPSTemplate_SightBase;
class AFPSTemplate_PartBase;
class UTexture2D;
class AFPSTemplateFirearm;

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
	// How much should the camera zoom when aiming with this optic
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float CameraFOVZoom;
	// How fast should the camera zoom when aiming with this optic
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float CameraFOVZoomSpeed;
	// How close/far should the camera be from this optic when aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float CameraDistance;
	// This will lock the distance from optic to the camera when aiming. Useful for magnified optics with a simulated eyebox
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float MuzzleVelocityChangePercentage = 0.0f;

	FFirearmPartStats operator+ (const FFirearmPartStats& Stats) const
	{
		FFirearmPartStats NewStats;
		NewStats.Weight = this->Weight + Stats.Weight;
		NewStats.ErgonomicsChangePercentage = this->ErgonomicsChangePercentage + Stats.ErgonomicsChangePercentage;
		NewStats.RecoilChangePercentage = this->RecoilChangePercentage + Stats.RecoilChangePercentage;
		NewStats.MuzzleVelocityChangePercentage = this->MuzzleVelocityChangePercentage + Stats.MuzzleVelocityChangePercentage;
		
		return NewStats;
	}
	
	FFirearmPartStats& operator+= (const FFirearmPartStats& Stats)
	{
		this->Weight += Stats.Weight;
		this->ErgonomicsChangePercentage += Stats.ErgonomicsChangePercentage;
		this->RecoilChangePercentage += Stats.RecoilChangePercentage;
		this->MuzzleVelocityChangePercentage += Stats.MuzzleVelocityChangePercentage;
		
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
	// This is used to alter how fast the projectile comes out of the muzzle
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	float MuzzleVelocityMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FSightData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "FPSTemplate")
	AFPSTemplate_SightBase* Sight = nullptr;

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
	TSubclassOf<AFPSTemplate_PartBase> PartClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	UTexture2D* PartImage = nullptr;
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
	// Set this to the material index (element) that the reticle will be on
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | ReticleSettings")
	int32 ReticleMaterialIndex = 1;
	// Ignore for now, this is for a future feature
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | ReticleSettings")
	float Radius = 1.0f;
	// Reticle materials you wish to use and cycle through (such as red and green dots)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | ReticleSettings")
	TArray<FReticleMaterial> ReticleMaterials;
	// Reticle Brightness settings to make your reticle brighter/darker
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
	// Use this instead of setting Capture Every Frame
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization")
	bool bOverrideCaptureEveryFrame = false;
	// The refresh rate of the optic (60 = 60 times per second)
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization", meta = (EditCondition = "!bOverrideCaptureEveryFrame"))
	float RefreshRate = 60.0f;
	// Disable the scene capture component when not aiming down sights
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization", meta = (EditCondition = "!bOverrideCaptureEveryFrame"))
	bool bDisableWhenNotAiming = true;
	// Continue running scene capture component at a set refresh rate (5 = 5 times per second)
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization", meta = (EditCondition = "!bDisableWhenNotAiming && !bOverrideCaptureEveryFrame"))
	float NotAimingRefreshRate = 5.0f;
	// When not aiming clear the scope with a color
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization", meta = (EditCondition = "bDisableWhenNotAiming"))
	bool bClearScopeWithColor = true;
	// Color to clear the scope with
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization", meta = (EditCondition = "bDisableWhenNotAiming && bClearScopeWithColor"))
	FLinearColor ClearedColor = FLinearColor::Black;
	// When not aiming clear the scope with a material
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Optimization", meta = (EditCondition = "bDisableWhenNotAiming && !bClearScopeWithColor"))
	bool bClearScopeWithMaterial = false;
	// Material to clear the scope with
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPSTemplate")
	float DecalFadeScreenSize = 0.001f;
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
	TSubclassOf<AFPSTemplateFirearm> ParentFirearm;
	UPROPERTY()
	TSubclassOf<AFPSTemplate_PartBase> Parent;
	UPROPERTY()
	TSubclassOf<AFPSTemplate_PartBase> Part;
	UPROPERTY()
	float PartOffset = 0.0f;
	
	bool bHasBeenCreated = false;
};

USTRUCT(BlueprintType)
struct FFirearm
{
	GENERATED_BODY()
	UPROPERTY()
	TSubclassOf<AFPSTemplateFirearm> FirearmClass;
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
		Location = INTransform.GetLocation();
		Rotation = INTransform.Rotator();
	}
	
	static FTransform GetTransformFromProjectile(const FProjectileTransform& ProjectileTransform)
	{
		return FTransform(ProjectileTransform.Rotation, ProjectileTransform.Location, FVector::OneVector);
	}
	FTransform GetTransformFromProjectile() const
	{
		return FTransform(Rotation, Location, FVector::OneVector);
	}
	
	operator FTransform() const { return GetTransformFromProjectile(FTransform(Rotation, Location)); }
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
struct FCurveData
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	UCurveVector* LocationCurve = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	UCurveVector* RotationCurve = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float CurveDuration = 3.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate")
	float CurveSpeed = 8.0f;
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
	UCurveVector* RecoilLocationCurve = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	FMinMax RecoilLocationRandomness = FMinMax(1.0f, 2.0f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	UCurveVector* RecoilRotationCurve = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	FMinMax RecoilPitchRandomness = FMinMax(1.0f, 2.0f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	FMinMax RecoilRollRandomness = FMinMax(-3.0f, 3.0f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate")
	FMinMax RecoilYawRandomness = FMinMax(-8.0f, 8.0f);

	FVector GetFixedLocationVector(const float Time) const
	{
		if (RecoilLocationCurve)
		{
			return RecoilLocationCurve->GetVectorValue(Time);
		}
		return FVector::ZeroVector;
	}

	FVector GetFixedRotationVector(const float Time) const
	{
		if (RecoilRotationCurve)
		{
			return RecoilRotationCurve->GetVectorValue(Time);
		}
		return FVector::ZeroVector;
	}
};


USTRUCT(BlueprintType)
struct FRenderTargetSize
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SKG")
	int32 Width = 512;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SKG")
	int32 Height = 512;
};

// This section is to aid in server performance with replication.
UENUM(BlueprintType)
enum EPartIndex
{
	Barrel, Handguard, Stock, ForwardGrip, Muzzle
};

USTRUCT(BlueprintType)
struct FFirearmPartComponentsRep
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | PartComponents")
	TArray<UFPSTemplate_PartComponent*> IndividualComponents = { nullptr, nullptr, nullptr, nullptr, nullptr };

	UFPSTemplate_PartComponent* GetPartComponent(EPartIndex PartIndex)
	{
		if (IndividualComponents.Num() >= PartIndex)
		{
			return IndividualComponents[PartIndex];
		}
		return nullptr;
	}

	AFPSTemplate_PartBase* GetPart(EPartIndex PartIndex)
	{
		if (IndividualComponents.Num() >= PartIndex && IndividualComponents[PartIndex])
		{
			return IndividualComponents[PartIndex]->GetPart();
		}
		return nullptr;
	}

	template<class T>
	T* GetPart(EPartIndex PartIndex)
	{
		if (IndividualComponents.Num() >= PartIndex && IndividualComponents[PartIndex])
		{
			return IndividualComponents[PartIndex]->GetPart<T>();
		}
		return nullptr;
	}

	void SetPart(UFPSTemplate_PartComponent* PartComponent, EPartIndex PartIndex)
	{
		if (IndividualComponents.Num() >= PartIndex)
		{
			IndividualComponents[PartIndex] = PartComponent;
		}
	}
};

UENUM(BlueprintType)
enum EPartOwnerIndex
{
	Part, Sight, LightLaser, Magnifier, RenderTarget
};

USTRUCT(BlueprintType)
struct FFirearmPartComponentsOwner
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<UFPSTemplate_PartComponent*> PartComponents;

	FFirearmPartComponentsOwner()
	{
		
	}
};

USTRUCT(BlueprintType)
struct FFirearmPartComponentsOwnerRep
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | PartComponents")
	TArray<FFirearmPartComponentsOwner> PartComponents = { FFirearmPartComponentsOwner(), FFirearmPartComponentsOwner(), FFirearmPartComponentsOwner(), FFirearmPartComponentsOwner(), FFirearmPartComponentsOwner() };

	TArray<UFPSTemplate_PartComponent*> GetPartComponents(EPartOwnerIndex PartIndex)
	{
		if (PartComponents.Num() >= PartIndex)
		{
			return PartComponents[PartIndex].PartComponents;
		}
		return TArray<UFPSTemplate_PartComponent*>();
	}

	void AddPartComponent(UFPSTemplate_PartComponent* PartComponent, EPartOwnerIndex PartIndex)
	{
		if (PartComponents.Num() >= PartIndex)
		{
			PartComponents[PartIndex].PartComponents.AddUnique(PartComponent);
		}
	}

	void SetPartComponents(TArray<UFPSTemplate_PartComponent*> INPartComponent, EPartOwnerIndex PartIndex)
	{
		if (PartComponents.Num() >= PartIndex)
		{
			PartComponents[PartIndex].PartComponents = INPartComponent;
		}
	}

	void ClearPartComponents(EPartOwnerIndex PartIndex)
	{
		if (PartComponents.Num() >= PartIndex)
		{
			PartComponents[PartIndex].PartComponents.Empty();
		}
	}
};