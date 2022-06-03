//Copyright 2021, Dakota Dawe, All rights reserved

#include "Actors/FPSTemplateFirearm.h"
#include "FPSTemplateAnimInstance.h"
#include "Misc/FPSTemplateStatics.h"
#include "FirearmParts/BaseClasses/FPSTemplate_SightBase.h"
#include "FirearmParts/FPSTemplate_Barrel.h"
#include "FirearmParts/FPSTemplate_Handguard.h"
#include "FirearmParts/FPSTemplate_ForwardGrip.h"
#include "FirearmParts/FPSTemplate_Muzzle.h"
#include "FirearmParts/BaseClasses/FPSTemplate_LightLaserBase.h"
#include "Components/FPSTemplate_PartComponent.h"
#include "FirearmParts/BaseClasses/FPSTemplate_MagnifierBase.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "Components/FPSTemplate_CharacterComponent.h"
#include "Components/FPS_FirearmStabilizerComponent.h"

#include "DrawDebugHelpers.h"
#include "NiagaraSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#define DEFAULT_STATS_MULTIPLIER (GetFirearmStats().Ergonomics * (10.0f / (GetFirearmStats().Weight * 1.5f)))

// Sets default values
AFPSTemplateFirearm::AFPSTemplateFirearm(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	NetUpdateFrequency = 8.0f;

	/*FirearmMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	FirearmMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = FirearmMesh;*/
	PoseCollision = ECollisionChannel::ECC_GameTraceChannel2;

	bFirearmHidden = false;
	
	FirearmAnimationIndex = 0;
	PointAimIndex = -1;

	FirearmGripSocket = FName("cc_FirearmGrip");
	GripSocketOffset = FTransform();
	MuzzleSocket = FName("S_Muzzle");
	LeftHandIKSocket = FName("S_LeftHandIK");
	AimSocket = FName("S_Aim");

	CameraSettings.CameraFOVZoom = 10.0f;
	CameraSettings.CameraFOVZoomSpeed = 10.0f;
	CameraSettings.CameraDistance = 0.0f;
	CameraSettings.bUsedFixedCameraDistance = false;

	bSpawnDefaultPartsFromPreset = false;
	bShouldSpawnDefaultsFromPreset = true;

	bUseLeftHandIK = false;

	DefaultFirearmStats.Weight = 7.0f;
	DefaultFirearmStats.Ergonomics = 50.0f;
	DefaultFirearmStats.RecoilMultiplier = 1.0f;

	/*CameraFOVZoom = 15.0f;
	CameraFOVZoomSpeed = 10.0f;*/

	PointAimADSInterpolationMultiplier = 1.2f;
	bUnAimMultiplierSameAsADS = false;
	UnAimInterpolationMultiplier = 1.0f;
	ShoulderStockOffset = FVector(-8.0f, 0.0f, 2.0f);
	HeadAimRotation = FRotator(45.0f, 0.0f, 0.0f);

	bCanFire = true;
	bCanReFire = true;
	bCanCycleSights = true;
	
	FireModeIndex = 0;
	BurstFireCount = 0;
	BurstCount = 3;

	FireRateRPM = 800.0f;

	TimeSinceLastShot = 0.0f;

	FirstPersonShortStockPose = FTransform();
	ThirdPersonShortStockPose = FTransform();
	ShortStockInterpSpeed = 22.0f;
	FirstPersonBasePoseOffset = FTransform();
	FirstPersonSprintPose.SetLocation(FVector(-5.0f, 0.0f, -10.0f));
	FirstPersonSprintPose.SetRotation(FRotator(-45.0f, 0.0f, 20.0f).Quaternion());
	FirstPersonHighPortPose.SetLocation(FVector(-10.0f, 0.0f, -10.0f));
	FirstPersonHighPortPose.SetRotation(FRotator(80.0f, 45.0f, 0.0f).Quaternion());
	FirstPersonLowPortPose.SetLocation(FVector(-10.0f, 0.0f, -10.0f));
	FirstPersonLowPortPose.SetRotation(FRotator(80.0f, -45.0f, 0.0f).Quaternion());

	ThirdPersonBasePoseOffset = FTransform();
	ThirdPersonSprintPose.SetLocation(FVector(-5.0f, 0.0f, -10.0f));
	ThirdPersonSprintPose.SetRotation(FRotator(-45.0f, 0.0f, 20.0f).Quaternion());
	ThirdPersonHighPortPose.SetLocation(FVector(-10.0f, 0.0f, -3.0f));
	ThirdPersonHighPortPose.SetRotation(FRotator(80.0f, 45.0f, 0.0f).Quaternion());
	ThirdPersonLowPortPose.SetLocation(FVector(0.0f, 8.0f, 8.0f));
	ThirdPersonLowPortPose.SetRotation(FRotator(80.0f, -45.0f, 0.0f).Quaternion());

	DefaultSwayMultiplier = 2.0f;
	SwayMultiplier = DefaultSwayMultiplier;

	RotationLagMultiplier = 1.0f;
	MovementLagMultiplier = 1.0f;
	
	CharacterComponent = nullptr;

	AttachedToSocket = NAME_None;
}

void AFPSTemplateFirearm::OnRep_CharacterComponent()
{
	//CycleSights();
	if (IsValid(this) && AttachedToSocket != NAME_None)
	{
		AttachToSocket(AttachedToSocket);
	}
}

// Called when the game starts or when spawned
void AFPSTemplateFirearm::BeginPlay()
{
	Super::BeginPlay();

	FirearmMesh = GetSkeletalMeshComponent();
	
	if (const AActor* OwningActor = GetOwner())
	{
		CharacterComponent = Cast<UFPSTemplate_CharacterComponent>(OwningActor->GetComponentByClass(UFPSTemplate_CharacterComponent::StaticClass()));
	}
	
	if (HasAuthority())
	{
		TArray<UFPSTemplate_PartComponent*> TempPartComponents;
		GetComponents<UFPSTemplate_PartComponent>(TempPartComponents);
		FirearmPartComponentsOwner.SetPartComponents(TempPartComponents, EPartOwnerIndex::Part);
	}

	if (HasAuthority())
	{
		HandleSightComponents();
	}

	// Update initial Default Parts
	FTimerHandle TTemp;
	GetWorldTimerManager().SetTimer(TTemp, this, &AFPSTemplateFirearm::PartsChanged_Implementation, 0.1f, false);

	FixPoseTransforms(FirstPersonShortStockPose, ThirdPersonShortStockPose);
	FixPoseTransforms(FirstPersonBasePoseOffset, ThirdPersonBasePoseOffset);
	FixPoseTransforms(FirstPersonHighPortPose, ThirdPersonHighPortPose);
	FixPoseTransforms(FirstPersonLowPortPose, ThirdPersonLowPortPose);
	FixPoseTransforms(FirstPersonSprintPose, ThirdPersonSprintPose);
}

void AFPSTemplateFirearm::PostInitProperties()
{
	Super::PostInitProperties();
	SwayMultiplier = DefaultSwayMultiplier;
	
	FirearmStats = DefaultFirearmStats;
	if (FireModes.Num())
	{
		FireMode = FireModes[0];
	}
	TimerAutoFireRate = 60 / FireRateRPM;

	bool bValidMesh = false;
	if (GetSkeletalMeshComponent())
	{
		bValidMesh = true;
		GetSkeletalMeshComponent()->SetCollisionResponseToChannel(PoseCollision, ECR_Ignore);
		GetSkeletalMeshComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		GetSkeletalMeshComponent()->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		GetSkeletalMeshComponent()->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
	}
	{
		ensureMsgf(bValidMesh, TEXT("Firearm: %s Has an INVALID skeletal mesh component"), *GetName());
	}
}

void AFPSTemplateFirearm::OnRep_Owner()
{
	if (const AActor* OwningActor = GetOwner())
	{
		CharacterComponent = Cast<UFPSTemplate_CharacterComponent>(OwningActor->GetComponentByClass(UFPSTemplate_CharacterComponent::StaticClass()));
	}
	RefreshCurrentSight();
}

void AFPSTemplateFirearm::OnRep_AttachmentReplication()
{
	//Super::OnRep_AttachmentReplication();
}

FPrimaryAssetId AFPSTemplateFirearm::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, AssetName);
}

void AFPSTemplateFirearm::FixPoseTransforms(FTransform& FirstPerson, FTransform& ThirdPerson)
{
	FirstPerson = UFPSTemplateStatics::FixTransform(FirstPerson);
	ThirdPerson = UFPSTemplateStatics::FixTransform(ThirdPerson);
}

void AFPSTemplateFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSTemplateFirearm, CharacterComponent);
	DOREPLIFETIME(AFPSTemplateFirearm, bFirearmHidden);
	DOREPLIFETIME(AFPSTemplateFirearm, CurrentSightComponent);
	DOREPLIFETIME(AFPSTemplateFirearm, PointAimIndex);
	DOREPLIFETIME_CONDITION(AFPSTemplateFirearm, FirearmStats, COND_OwnerOnly); // SET THIS TO OWNER ONLY ????
	DOREPLIFETIME_CONDITION(AFPSTemplateFirearm, FireMode, COND_SkipOwner);
	DOREPLIFETIME(AFPSTemplateFirearm, AttachedToSocket);
	DOREPLIFETIME_CONDITION(AFPSTemplateFirearm, FirearmPartComponentsOwner, COND_OwnerOnly);
	DOREPLIFETIME(AFPSTemplateFirearm, FirearmPartComponents);
}

float AFPSTemplateFirearm::GetStockLengthOfPull()
{
	if (IsValid(FirearmPartComponents.GetPart(EPartIndex::Stock)))
	{
		if (const AFPSTemplate_PartBase* Part = FirearmPartComponents.GetPart(EPartIndex::Stock))
		{
			return Part->GetPartOffset() + Part->GetStockLengthOfPull();
		}
	}
	return 0.0f;
}

float AFPSTemplateFirearm::GetAimInterpolationMultiplier_Implementation()
{
	float SightAimMultiplier = 1.0f;
	if (PointAimIndex > INDEX_NONE)
	{
		SightAimMultiplier = PointAimADSInterpolationMultiplier;
	}
	else
	{
		AFPSTemplate_SightBase* Sight = GetCurrentSight();
		if (IsValid(Sight) && Sight->Implements<UFPSTemplate_AimInterface>())
		{
			SightAimMultiplier = IFPSTemplate_AimInterface::Execute_GetAimInterpolationMultiplier(Sight);
		}
	}
	return (DEFAULT_STATS_MULTIPLIER * PoseSettings.AimInterpolationMultiplier) * SightAimMultiplier;
}

float AFPSTemplateFirearm::GetUnAimInterpolationMultiplier_Implementation()
{
	if (bUnAimMultiplierSameAsADS)
	{
		return IFPSTemplate_AimInterface::Execute_GetAimInterpolationMultiplier(this);
	}
	return (DEFAULT_STATS_MULTIPLIER * PoseSettings.AimInterpolationMultiplier) * UnAimInterpolationMultiplier;
}

float AFPSTemplateFirearm::GetRotationLagInterpolationMultiplier_Implementation()
{
	return DEFAULT_STATS_MULTIPLIER * RotationLagMultiplier;
}

float AFPSTemplateFirearm::GetMovementLagInterpolationMultiplier_Implementation()
{
	return DEFAULT_STATS_MULTIPLIER * MovementLagMultiplier;
}

void AFPSTemplateFirearm::GetCameraSettings_Implementation(FAimCameraSettings& OutCameraSettings)
{
	if (PointAimIndex > INDEX_NONE)
	{
		OutCameraSettings = CameraSettings;
		return;
	}
	AFPSTemplate_SightBase* CurrentSight = GetCurrentSight();
	if (IsValid(CurrentSight))
	{
		if (CurrentSight->Implements<UFPSTemplate_AimInterface>())
		{
			Execute_GetCameraSettings(CurrentSight, OutCameraSettings);
			return;
		}
	}
	OutCameraSettings = CameraSettings;
}

void AFPSTemplateFirearm::ZoomOptic_Implementation(bool bZoom)
{
	AFPSTemplate_SightBase* Sight = GetCurrentSight();
	if (IsValid(Sight) && Sight->Implements<UFPSTemplate_AimInterface>())
	{
		Execute_ZoomOptic(Sight, bZoom);
	}
	else
	{
		RefreshCurrentSight();
	}
}

FTransform AFPSTemplateFirearm::GetSightSocketTransform_Implementation()
{
	if (PointAimIndex > INDEX_NONE && PointAimIndex < PointAimSockets.Num() && FirearmMesh->DoesSocketExist(PointAimSockets[PointAimIndex]))
	{
		return FirearmMesh->GetSocketTransform(PointAimSockets[PointAimIndex]);
	}
	
	if (CurrentSightComponent && IsValid(CurrentSightComponent->GetPart()))
	{
		return CurrentSightComponent->GetPart()->GetAimSocketTransform();
	}
	
	TArray<UFPSTemplate_PartComponent*> PartComponents = FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::Sight);
	if (PartComponents.Num() && IsValid(PartComponents[0]) && IsValid(PartComponents[0]->GetPart()))
	{
		return PartComponents[0]->GetPart()->GetAimSocketTransform();
	}
	/*if (SightComponents.Num() && SightComponents[0] && IsValid(SightComponents[0]->GetPart()))
	{
		return SightComponents[0]->GetPart()->GetAimSocketTransform();
	}*/
	return FirearmMesh->GetSocketTransform(AimSocket);
}

FTransform AFPSTemplateFirearm::GetDefaultSightSocketTransform_Implementation()
{
	return FirearmMesh->DoesSocketExist(AimSocket) ? FirearmMesh->GetSocketTransform(AimSocket) : Execute_GetSightSocketTransform(this);
}

void AFPSTemplateFirearm::UpdateFirearmStats()
{
	if (HasAuthority())
	{
		FirearmStats = DefaultFirearmStats;
		FFirearmPartStats PartStats;
		for (UFPSTemplate_PartComponent* PartComponent : FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::Part))
		{
			if (PartComponent && IsValid(PartComponent->GetPart()))
			{
				PartComponent->GetPart()->CacheParts();
				PartStats += PartComponent->GetPart()->GetPartStats();
			}
		}

		FirearmStats.Weight += PartStats.Weight;
		float PercentMultiplier = 1.0f + (PartStats.RecoilChangePercentage / 100.0f);
		PercentMultiplier = FMath::Clamp(PercentMultiplier, 0.1f, 100.0f);
		FirearmStats.RecoilMultiplier = DefaultFirearmStats.RecoilMultiplier;
		FirearmStats.RecoilMultiplier *= PercentMultiplier;

		PercentMultiplier = 1.0f + (PartStats.ErgonomicsChangePercentage / 100.0f);
		FirearmStats.Ergonomics *= PercentMultiplier;

		PercentMultiplier = 1.0f + (PartStats.MuzzleVelocityChangePercentage / 100.0f);
		PercentMultiplier = FMath::Clamp(PercentMultiplier, 0.1f, 100.0f);
		FirearmStats.MuzzleVelocityMultiplier = DefaultFirearmStats.MuzzleVelocityMultiplier;
		FirearmStats.MuzzleVelocityMultiplier *= PercentMultiplier;
	}
}

void AFPSTemplateFirearm::HandleSightComponents()
{
	FirearmPartComponentsOwner.ClearPartComponents(EPartOwnerIndex::Sight);
	for (UFPSTemplate_PartComponent* PartComponent : FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::Part))
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			if (PartComponent->GetPart()->IsAimable())
			{
				FirearmPartComponentsOwner.AddPartComponent(PartComponent, EPartOwnerIndex::Sight);
				//SightComponents.Add(PartComponent);
			}
			for (UFPSTemplate_PartComponent* PartPartComponent : PartComponent->GetPart()->GetSightComponents())
			{
				if (PartPartComponent && IsValid(PartPartComponent->GetPart()))
				{
					if (PartPartComponent->GetPart()->IsAimable())
					{
						FirearmPartComponentsOwner.AddPartComponent(PartPartComponent, EPartOwnerIndex::Sight);
						//SightComponents.Add(PartPartComponent);
					}
				}
			}
		}
	}
	if (!IsValid(CurrentSightComponent) || (IsValid(CurrentSightComponent) && IsValid(CurrentSightComponent->GetPart()) && !CurrentSightComponent->GetPart()->IsAimable()) || (IsValid(CurrentSightComponent) && !IsValid(CurrentSightComponent->GetPart())))
	{
		CycleSights();
	}
}

UFPSTemplate_CharacterComponent* AFPSTemplateFirearm::GetCharacterComponent()
{
	if (!CharacterComponent)
	{
		if (IsValid(GetOwner()))
		{
			UActorComponent* Component = GetOwner()->GetComponentByClass(UFPSTemplate_CharacterComponent::StaticClass());
			CharacterComponent = Cast<UFPSTemplate_CharacterComponent>(Component);
			OnRep_CharacterComponent();
		}
	}
	return CharacterComponent;
}

bool AFPSTemplateFirearm::IsSuppressed()
{
	const AFPSTemplate_Muzzle* Muzzle = GetMuzzleDevice();
	if (IsValid(Muzzle) && Muzzle->IsSuppressor())
	{
		return true;
	}
	return false;
}

UNiagaraSystem* AFPSTemplateFirearm::GetFireNiagaraSystem()
{
	AActor* MuzzleActor = GetMuzzleActor();
	if (IsValid(MuzzleActor))
	{
		AFPSTemplate_Muzzle* Muzzle = Cast<AFPSTemplate_Muzzle>(MuzzleActor);
		if (IsValid(Muzzle))
		{
			if (UNiagaraSystem* NiagaraSystem = Muzzle->GetFireNiagaraSystem())
			{
				return NiagaraSystem;
			}
		}
		
		const UFPSTemplate_PartComponent* BarrelComponent = GetPartComponent(EPartIndex::Barrel);
		if (IsValid(BarrelComponent))
		{
			AFPSTemplate_Barrel* Barrel = BarrelComponent->GetPart<AFPSTemplate_Barrel>();
			if (IsValid(Barrel))
			{
				if (UNiagaraSystem* NiagaraSystem = Barrel->GetFireNiagaraSystem())
				{
					return NiagaraSystem;
				}
			}
		}
	}
	
	const int32 RandomIndex = UFPSTemplateStatics::GetRandomIndexForArray(FireNiagaraSystems.Num());
	if (RandomIndex != INDEX_NONE)
	{
		return FireNiagaraSystems[RandomIndex];
	}
	return nullptr;
}

bool AFPSTemplateFirearm::Server_SetPointAiming_Validate(int32 Index)
{
	return true;
}

void AFPSTemplateFirearm::Server_SetPointAiming_Implementation(int32 Index)
{
	SetPointAimIndex(Index);
}

void AFPSTemplateFirearm::SetPointAimIndex(int32 Index)
{
	if (Index < PointAimSockets.Num())
	{
		PointAimIndex = Index;
		if (GetCharacterComponent() && CharacterComponent->IsAiming())
		{
			if (PointAimIndex > INDEX_NONE)
			{
				ActivateCurrentSight(false);
			}
			else
			{
				ActivateCurrentSight(true);
			}
		}
		OnRep_PointAimIndex();
	}
	else
	{
		PointAimIndex = INDEX_NONE;
	}
	
	if (!HasAuthority())
	{
		Server_SetPointAiming(PointAimIndex);
	}
}

void AFPSTemplateFirearm::CyclePointAim()
{
	if (++PointAimIndex > PointAimSockets.Num() - 1)
	{
		PointAimIndex = 0;
	}
	SetPointAimIndex(PointAimIndex);
}

void AFPSTemplateFirearm::CycleMagnifier()
{
	for (const UFPSTemplate_PartComponent* PartComponent : FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::Magnifier))
	{
		if (PartComponent)
		{
			AFPSTemplate_MagnifierBase* Magnifier = PartComponent->GetPart<AFPSTemplate_MagnifierBase>();
			if (IsValid(Magnifier))
			{
				Magnifier->Use();
			}
		}
	}
}

void AFPSTemplateFirearm::UseLightLaser(ELightLaser Toggle, bool bSync)
{
	for (UFPSTemplate_PartComponent* PartComponent : FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::LightLaser))
	{
		if (PartComponent)
		{
			AFPSTemplate_LightLaserBase* LightLaser = PartComponent->GetPart<AFPSTemplate_LightLaserBase>();
			if (IsValid(LightLaser))
			{
				switch (Toggle)
				{
					case ELightLaser::Light: LightLaser->ToggleLight(); break;
					case ELightLaser::Laser: LightLaser->ToggleLaser(); break;
					case ELightLaser::Both: LightLaser->ToggleLightAndLaser(bSync); break;
				}
			}
		}
	}
}

void AFPSTemplateFirearm::CycleLightLaserModes()
{
	for (UFPSTemplate_PartComponent* PartComponent : FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::LightLaser))
	{
		if (PartComponent)
		{
			AFPSTemplate_LightLaserBase* LightLaser = PartComponent->GetPart<AFPSTemplate_LightLaserBase>();
			if (IsValid(LightLaser))
			{
				LightLaser->CycleThroughModes();
			}
		}
	}
}

void AFPSTemplateFirearm::CycleLaserColor()
{
	for (const UFPSTemplate_PartComponent* PartComponent : FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::LightLaser))
	{
		if (PartComponent)
		{
			AFPSTemplate_LightLaserBase* LightLaser = PartComponent->GetPart<AFPSTemplate_LightLaserBase>();
			if (IsValid(LightLaser))
			{
				LightLaser->CycleLaserColor();
			}
		}
	}
}

void AFPSTemplateFirearm::CycleReticle()
{
	AFPSTemplate_SightBase* Sight = GetCurrentSight();
	if (IsValid(Sight))
	{
		Sight->CycleReticle();
	}
}

void AFPSTemplateFirearm::IncreaseReticleBrightness(bool Increase)
{
	AFPSTemplate_SightBase* Sight = GetCurrentSight();
	if (IsValid(Sight))
	{
		if (Increase)
		{
			Sight->IncreaseBrightness();
		}
		else
		{
			Sight->DecreaseBrightness();
		}
	}
	else
	{
		RefreshCurrentSight();
	}
}

float AFPSTemplateFirearm::GetOpticMagnification() const
{
	return PointAimIndex == INDEX_NONE && IsValid(GetCurrentSight()) ? GetCurrentSight()->GetMagnification() : 1.0f;
}

void AFPSTemplateFirearm::PointOfImpactUp(bool Up, uint8 Clicks)
{
	AFPSTemplate_SightBase* Sight = GetCurrentSight();
	if (IsValid(Sight))
	{
		if (Up)
		{
			Sight->PointOfImpactUp(Clicks);
		}
		else
		{
			Sight->PointOfImpactDown(Clicks);
		}
	}
	else
	{
		RefreshCurrentSight();
	}
}

void AFPSTemplateFirearm::PointOfImpactLeft(bool Left, uint8 Clicks)
{
	AFPSTemplate_SightBase* Sight = GetCurrentSight();
	if (IsValid(Sight))
	{
		if (Left)
		{
			Sight->PointOfImpactLeft(Clicks);
		}
		else
		{
			Sight->PointOfImpactRight(Clicks);
		}
	}
	else
	{
		RefreshCurrentSight();
	}
}

void AFPSTemplateFirearm::ReturnSightToZero(EElevationWindage Dial)
{
	AFPSTemplate_SightBase* Sight = GetCurrentSight();
	if (IsValid(Sight))
	{
		switch (Dial)
		{
			case EElevationWindage::Elevation : Sight->ReturnToZeroElevation(); break;
			case EElevationWindage::Windage : Sight->ReturnToZeroWindage(); break;
			case EElevationWindage::Both : Sight->ReturnToZeroElevation(); Sight->ReturnToZeroWindage(); break;
		}
	}
	else
	{
		RefreshCurrentSight();
	}
}

void AFPSTemplateFirearm::SetSightZero(const EElevationWindage Dial)
{
	AFPSTemplate_SightBase* Sight = GetCurrentSight();
	if (IsValid(Sight))
	{
		switch (Dial)
		{
		case EElevationWindage::Elevation : Sight->SetNewZeroElevation(); break;
		case EElevationWindage::Windage : Sight->SetNewZeroWindage(); break;
		case EElevationWindage::Both : Sight->SetNewZeroElevation(); Sight->SetNewZeroWindage(); break;
		}
	}
	else
	{
		RefreshCurrentSight();
	}
}

FSightZero AFPSTemplateFirearm::GetSightZero()
{
	const AFPSTemplate_SightBase* Sight = GetCurrentSight();
	if (IsValid(Sight))
	{
		return Sight->GetSightZero();
	}
	else
	{
		RefreshCurrentSight();
	}
	return FSightZero();
}

AFPSTemplate_Muzzle* AFPSTemplateFirearm::GetMuzzleDevice()
{
	AFPSTemplate_Muzzle* Muzzle = Cast<AFPSTemplate_Muzzle>(GetMuzzleActor());
	if (IsValid(Muzzle))
	{
		return Muzzle->GetMuzzleAttachment();
	}
	return Cast<AFPSTemplate_Muzzle>(GetMuzzleActor());
}

AActor* AFPSTemplateFirearm::GetMuzzleActor()
{
	AFPSTemplate_Muzzle* Muzzle = FirearmPartComponents.GetPart<AFPSTemplate_Muzzle>(EPartIndex::Muzzle);
	if (IsValid(Muzzle))
	{
		return Muzzle->GetMuzzleAttachment();
	}

	AFPSTemplate_Barrel* Barrel = FirearmPartComponents.GetPart<AFPSTemplate_Barrel>(EPartIndex::Barrel);
	if (IsValid(Barrel))
	{
		return Barrel->GetMuzzleDeviceActor();
	}
	return this;
}

FHitResult AFPSTemplateFirearm::MuzzleTrace(float Distance, ECollisionChannel CollisionChannel, bool& bMadeBlockingHit, bool bDrawDebugLine)
{
	FTransform MuzzleTransform = FProjectileTransform::GetTransformFromProjectile(GetMuzzleSocketTransform());
	FVector End = MuzzleTransform.GetLocation() + MuzzleTransform.Rotator().Vector() * Distance;
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	
	bMadeBlockingHit = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleTransform.GetLocation(), End, CollisionChannel, Params);
	if (bDrawDebugLine)
	{
		DrawDebugLine(GetWorld(), MuzzleTransform.GetLocation(), End, FColor::Red, false, 3.0f, 0, 2.0f);
	}
	return HitResult;
}

TArray<FHitResult> AFPSTemplateFirearm::MuzzleTraceMulti(float Distance, ECollisionChannel CollisionChannel, bool& bMadeBlockingHit, bool bDrawDebugLine)
{
	const FTransform MuzzleTransform = FProjectileTransform::GetTransformFromProjectile(GetMuzzleSocketTransform());
	const FVector End = MuzzleTransform.GetLocation() + MuzzleTransform.Rotator().Vector() * Distance;
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	
	bMadeBlockingHit = GetWorld()->LineTraceMultiByChannel(HitResults, MuzzleTransform.GetLocation(), End, CollisionChannel, Params);
	if (bDrawDebugLine)
	{
		DrawDebugLine(GetWorld(), MuzzleTransform.GetLocation(), End, FColor::Red, false, 3.0f, 0, 2.0f);
	}
	return HitResults;
}

void AFPSTemplateFirearm::DisableAllRenderTargets(bool Disable)
{
	for (UFPSTemplate_PartComponent* PartComponent : FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::RenderTarget))
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			/*if (PartComponent->GetPart()->GetPartType() == EPartType::Magnifier)
			{
				if (AFPSTemplate_MagnifierBase* Magnifier = Cast<AFPSTemplate_MagnifierBase>(PartComponent->GetPart()))
				{
					if (Magnifier->GetSightInfront() == GetCurrentSight())
					{
						PartComponent->GetPart()->DisableRenderTarget(false);
						continue;
					}
				}
			}*/
			
			if (!Disable && PartComponent != CurrentSightComponent)
			{
				PartComponent->GetPart()->DisableRenderTarget(true);
				continue;
			}

			PartComponent->GetPart()->DisableRenderTarget(Disable);
		}
	}
	//RefreshCurrentSight();
}

void AFPSTemplateFirearm::PartsChanged_Implementation()
{
	HandleSightComponents();
	UpdateFirearmStats();
	OnPartsChanged();
}

TArray<AFPSTemplate_PartBase*> AFPSTemplateFirearm::GetFirearmParts()
{
	TArray<AFPSTemplate_PartBase*> Parts;
	for (const UFPSTemplate_PartComponent* PartComponent : FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::Part))
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			Parts.Add(PartComponent->GetPart());
			for (const UFPSTemplate_PartComponent* PartPartComponent : PartComponent->GetPart()->GetPartComponents())
			{
				if (PartPartComponent && IsValid(PartPartComponent->GetPart()))
				{
					Parts.Add(PartPartComponent->GetPart());
				}
			}
		}
	}
	return Parts;
}

FTransform& AFPSTemplateFirearm::GetSprintPose()
{
	if (GetCharacterComponent())
	{
		if (!CharacterComponent->IsLocallyControlled() || CharacterComponent->IsInThirdPerson())
		{
			return ThirdPersonSprintPose; // THIRD PERSON HERE
		}
	}
	return FirstPersonSprintPose;
}

FTransform& AFPSTemplateFirearm::GetHighPortPose()
{
	if (GetCharacterComponent())
	{
		if (!CharacterComponent->IsLocallyControlled() || CharacterComponent->IsInThirdPerson())
		{
			return ThirdPersonHighPortPose;
		}
	}
	return FirstPersonHighPortPose;
}

FTransform& AFPSTemplateFirearm::GetLowPortPose()
{
	if (GetCharacterComponent())
	{
		if (!CharacterComponent->IsLocallyControlled() || CharacterComponent->IsInThirdPerson())
		{
			return ThirdPersonLowPortPose;
		}
	}
	return FirstPersonLowPortPose;
}

FTransform& AFPSTemplateFirearm::GetBasePoseOffset()
{
	if (GetCharacterComponent())
	{
		if (!CharacterComponent->IsLocallyControlled() || CharacterComponent->IsInThirdPerson())
		{
			return ThirdPersonBasePoseOffset;
		}
	}
	return FirstPersonBasePoseOffset;
}

FTransform& AFPSTemplateFirearm::GetShortStockPose()
{
	if (GetCharacterComponent())
	{
		if (!CharacterComponent->IsLocallyControlled() || CharacterComponent->IsInThirdPerson())
		{
			return ThirdPersonShortStockPose;
		}
	}
	return FirstPersonShortStockPose;
}

void AFPSTemplateFirearm::SetShortStockInterpSpeed(float Speed)
{
	ShortStockInterpSpeed = Speed;
}

void AFPSTemplateFirearm::SetShortStockPose(const FTransform& Transform, bool bFirstPerson)
{
	const FTransform FixedTransform = UFPSTemplateStatics::FixTransform(Transform);
	if (bFirstPerson)
	{
		FirstPersonShortStockPose = FixedTransform;
	}
	else
	{
		ThirdPersonShortStockPose = FixedTransform;
	}
}

void AFPSTemplateFirearm::SetBasePoseOffset(const FTransform& Transform, bool bFirstPerson)
{
	const FTransform FixedTransform = UFPSTemplateStatics::FixTransform(Transform);
	if (bFirstPerson)
	{
		FirstPersonBasePoseOffset = FixedTransform;
	}
	else
	{
		ThirdPersonBasePoseOffset = FixedTransform;
	}
}

void AFPSTemplateFirearm::SetHighPortPose(const FTransform& Transform, bool bFirstPerson)
{
	const FTransform FixedTransform = UFPSTemplateStatics::FixTransform(Transform);
	if (bFirstPerson)
	{
		FirstPersonHighPortPose = FixedTransform;
	}
	else
	{
		ThirdPersonHighPortPose = FixedTransform;
	}
}

void AFPSTemplateFirearm::SetLowPortPose(const FTransform& Transform, bool bFirstPerson)
{
	const FTransform FixedTransform = UFPSTemplateStatics::FixTransform(Transform);
	if (bFirstPerson)
	{
		FirstPersonLowPortPose = FixedTransform;
	}
	else
	{
		ThirdPersonLowPortPose = FixedTransform;
	}
}

void AFPSTemplateFirearm::SetSprintPose(const FTransform& Transform, bool bFirstPerson)
{
	const FTransform FixedTransform = UFPSTemplateStatics::FixTransform(Transform);
	if (bFirstPerson)
	{
		FirstPersonSprintPose = FixedTransform;
	}
	else
	{
		ThirdPersonSprintPose = FixedTransform;
	}
}

void AFPSTemplateFirearm::SetCharacterComponent(UFPSTemplate_CharacterComponent* INCharacterComponent)
{
	CharacterComponent = INCharacterComponent;
}

void AFPSTemplateFirearm::AddPartCache_Implementation(UFPSTemplate_PartComponent* PartComponent)
{
	if (PartComponent && IsValid(PartComponent->GetPart()))
	{
		OnPartComponentAdded(PartComponent);
		switch (PartComponent->GetPart()->GetPartType())
		{
		case EPartType::Barrel :
			{
				FirearmPartComponents.SetPart(PartComponent, EPartIndex::Barrel);
				break;
			}
		case EPartType::Handguard :
			{
				FirearmPartComponents.SetPart(PartComponent, EPartIndex::Handguard);
				break;
			}
		case EPartType::Stock :
			{
				FirearmPartComponents.SetPart(PartComponent, EPartIndex::Stock);
				break;
			}
		case EPartType::Magnifier :
			{
				FirearmPartComponentsOwner.AddPartComponent(PartComponent, EPartOwnerIndex::Magnifier);
				FirearmPartComponentsOwner.AddPartComponent(PartComponent, EPartOwnerIndex::RenderTarget);
				break;
			}
		case EPartType::LightLaser :
			{
				FirearmPartComponentsOwner.AddPartComponent(PartComponent, EPartOwnerIndex::LightLaser);
				break;
			}
		case EPartType::ForwardGrip :
			{
				FirearmPartComponents.SetPart(PartComponent, EPartIndex::ForwardGrip);
				break;
			}
		case EPartType::MuzzleDevice :
			{
				FirearmPartComponents.SetPart(PartComponent, EPartIndex::Muzzle);
				break;
			}
		default: if (PartComponent->GetPart()->HasRenderTarget())
			{
				FirearmPartComponentsOwner.AddPartComponent(PartComponent, EPartOwnerIndex::RenderTarget);
			}
		}
	}
}

UAnimSequence* AFPSTemplateFirearm::GetGripAnimation()
{
	const AFPSTemplate_ForwardGrip* ForwardGrip = FirearmPartComponents.GetPart<AFPSTemplate_ForwardGrip>(EPartIndex::ForwardGrip);
	if (IsValid(ForwardGrip))
	{
		return ForwardGrip->GetGripAnimation();
	}
	
	const AFPSTemplate_Handguard* Handguard = FirearmPartComponents.GetPart<AFPSTemplate_Handguard>(EPartIndex::Handguard);
	if (IsValid(Handguard))
	{
		return Handguard->GetGripAnimation();
	}
	return GripAnimation;
}

void AFPSTemplateFirearm::SetupStabilizerComponent()
{
	if (!StabilizerComponent.IsValid())
	{
		StabilizerComponent = Cast<UFPS_FirearmStabilizerComponent>(GetComponentByClass(UFPS_FirearmStabilizerComponent::StaticClass()));
	}
	if (StabilizerComponent.IsValid())
	{
		StabilizerComponent->CacheEssentials();
	}
}

UFPS_FirearmStabilizerComponent* AFPSTemplateFirearm::GetStabilizerComponent()
{
	SetupStabilizerComponent();
	return StabilizerComponent.Get();
}

bool AFPSTemplateFirearm::IsStabilized() const
{
	if (StabilizerComponent.IsValid())
	{
		return StabilizerComponent->IsStabilized();
	}
	return false;
}

void AFPSTemplateFirearm::OnRep_FireMode()
{
	if (GetCharacterComponent() && CharacterComponent->IsLocallyControlled())
	{
		return;
	}
	OnFireModeChanged();
}

void AFPSTemplateFirearm::OnRep_FirearmHidden() const
{
	FirearmMesh->SetHiddenInGame(bFirearmHidden, true);
}

bool AFPSTemplateFirearm::Server_SetFireMode_Validate(EFirearmFireMode NewFireMode)
{
	return true;
}

void AFPSTemplateFirearm::Server_SetFireMode_Implementation(EFirearmFireMode NewFireMode)
{
	FireMode = NewFireMode;
	OnRep_FireMode();
}

void AFPSTemplateFirearm::CycleFireMode(bool bReverse)
{
	if (FireModes.Num() > 1)
	{
		if (bReverse)
		{
			if (--FireModeIndex < 0)
			{
				FireModeIndex = FireModes.Num() - 1;
			}
		}
		else
		{
			if (++FireModeIndex > FireModes.Num() - 1)
			{
				FireModeIndex = 0;
			}
		}
		FireMode = FireModes[FireModeIndex];
		OnFireModeChanged();
		if (!HasAuthority())
		{
			Server_SetFireMode(FireMode);
		}
	}
}

void AFPSTemplateFirearm::PerformProceduralRecoil(float Multiplier, bool PlayCameraShake)
{
	if (GetCharacterComponent() && CharacterComponent->GetAnimationInstance())
	{
		CharacterComponent->GetAnimationInstance()->PerformRecoil(Multiplier);
		if (PlayCameraShake && CurveAndShakeSettings.FireCameraShake)
		{
			CharacterComponent->PlayCameraShake(CurveAndShakeSettings.FireCameraShake, Multiplier * 1.2f);
		}
	}
}

TArray<UFPSTemplate_PartComponent*> AFPSTemplateFirearm::GetAllPartComponents()
{
	TArray<UFPSTemplate_PartComponent*> PartComponentsList = FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::Part);
	for (const UFPSTemplate_PartComponent* Part : FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::Part))
	{
		if (IsValid(Part) && IsValid(Part->GetPart()))
		{
			PartComponentsList.Append(Part->GetPart()->GetPartComponents());
		}
	}
	//FirearmPartComponentsOwner.SetPartComponents(PartComponentsList, EPartOwnerIndex::Part);
	return PartComponentsList;
}

UFPSTemplate_PartComponent* AFPSTemplateFirearm::GetGripComponent()
{
	if (FirearmPartComponents.GetPartComponent(EPartIndex::ForwardGrip))
	{
		return FirearmPartComponents.GetPartComponent(EPartIndex::ForwardGrip);
	}
	if (FirearmPartComponents.GetPartComponent(EPartIndex::Handguard))
	{
		return FirearmPartComponents.GetPartComponent(EPartIndex::Handguard);
	}
	return nullptr;
}

FTransform AFPSTemplateFirearm::GetLeftHandIKTransform()
{
	const AFPSTemplate_ForwardGrip* ForwardGrip = FirearmPartComponents.GetPart<AFPSTemplate_ForwardGrip>(EPartIndex::ForwardGrip);
	if (IsValid(ForwardGrip))
	{
		return ForwardGrip->GetGripTransform();
	}

	const AFPSTemplate_Handguard* Handguard = FirearmPartComponents.GetPart<AFPSTemplate_Handguard>(EPartIndex::Handguard);
	if (IsValid(Handguard))
	{
		return Handguard->GetGripTransform();
	}
	return FirearmMesh->GetSocketTransform(LeftHandIKSocket);
}

/*FProjectileTransform AFPSTemplateFirearm::GetMuzzleSocketProjectileTransform() 
{
	const AFPSTemplate_Barrel* Barrel = FirearmPartComponents.GetPart<AFPSTemplate_Barrel>(EPartIndex::Barrel);
	if (IsValid(Barrel))
	{
		return Barrel->GetMuzzleSocketTransform();
	}
	return FirearmMesh->GetSocketTransform(MuzzleSocket);
}*/

FTransform AFPSTemplateFirearm::GetMuzzleSocketTransform()
{
	if (AActor* MuzzleActor = GetMuzzleActor())
	{
		AFPSTemplate_Muzzle* Muzzle = Cast<AFPSTemplate_Muzzle>(MuzzleActor);
		if (IsValid(Muzzle))
		{
			return Muzzle->GetMuzzleSocketTransform();
		}
		const AFPSTemplate_Barrel* Barrel = Cast<AFPSTemplate_Barrel>(MuzzleActor);
		if (IsValid(Barrel))
		{
			return Barrel->GetMuzzleSocketTransform();
		}
	}
	return FirearmMesh->GetSocketTransform(MuzzleSocket);
}

FProjectileTransform AFPSTemplateFirearm::GetMuzzleProjectileSocketTransform(float RangeMeters, float MOA)
{
	RangeMeters *= 100.0f;
	if (RangeMeters > 10000)
	{
		RangeMeters = 10000;
	}
	else if (RangeMeters < 2500)
	{
		RangeMeters = 2500;
	}

	const FTransform SightTransform = Execute_GetSightSocketTransform(this);
	FTransform MuzzleTransform = GetMuzzleSocketTransform();
	FRotator MuzzleRotation = UFPSTemplateStatics::GetEstimatedMuzzleToScopeZero(MuzzleTransform, SightTransform, RangeMeters);	
	MuzzleRotation = UFPSTemplateStatics::SetMuzzleMOA(MuzzleRotation, MOA);

	MuzzleTransform.SetRotation(MuzzleRotation.Quaternion());
	return MuzzleTransform;
}

TArray<FProjectileTransform> AFPSTemplateFirearm::GetMultipleMuzzleProjectileSocketTransforms(float RangeMeters, float InchSpreadAt25Yards,	uint8 ShotCount)
{
	TArray<FProjectileTransform> ProjectileTransforms;
	ProjectileTransforms.Reserve(ShotCount);
	for (uint8 i = 0; i < ShotCount; ++i)
	{
		ProjectileTransforms.Add(GetMuzzleProjectileSocketTransform(RangeMeters, InchSpreadAt25Yards));
	}
	return ProjectileTransforms;
}

void AFPSTemplateFirearm::OnRep_CurrentSightComponent()
{
	if (GetCharacterComponent() && CharacterComponent->GetAnimationInstance())
	{
		if (!CharacterComponent->IsLocallyControlled())
		{
			CharacterComponent->GetAnimationInstance()->CycledSights();
		}
	}
}

void AFPSTemplateFirearm::OnRep_PointAimIndex()
{
	OnRep_CurrentSightComponent();
}

bool AFPSTemplateFirearm::Server_CycleSights_Validate(UFPSTemplate_PartComponent* SightComponent)
{
	return true;
}

void AFPSTemplateFirearm::Server_CycleSights_Implementation(UFPSTemplate_PartComponent* SightComponent)
{
	if (SightComponent)
	{
		CurrentSightComponent = SightComponent;
		OnRep_CurrentSightComponent();
	}
}

void AFPSTemplateFirearm::ActivateCurrentSight(bool bActivate) const
{
	if (bActivate && PointAimIndex > INDEX_NONE)
	{
		if (IsValid(CurrentSightComponent) && IsValid(CurrentSightComponent->GetPart()))
		{
			CurrentSightComponent->GetPart()->DisableRenderTarget(true);
		}
		return;
	}
	if (IsValid(CurrentSightComponent) && IsValid(CurrentSightComponent->GetPart()))
	{
		CurrentSightComponent->GetPart()->DisableRenderTarget(!bActivate);
	}
}

void AFPSTemplateFirearm::CycleSights()
{
	if (!bCanCycleSights || PointAimIndex > INDEX_NONE)
	{
		return;
	}

	ActivateCurrentSight(false);
	
	bool FoundValidSight = false;
	TArray<UFPSTemplate_PartComponent*> PartComponents = FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::Sight);
	if (PartComponents.Num())
	{
		if (++SightComponentIndex >= PartComponents.Num())
		{
			SightComponentIndex = 0;
		}
		if (UFPSTemplate_PartComponent* SightComponent = PartComponents[SightComponentIndex])
		{
			const AFPSTemplate_SightBase* Sight = SightComponent->GetPart<AFPSTemplate_SightBase>();
			if (IsValid(Sight) && Sight->Implements<UFPSTemplate_AimInterface>())
			{
				CurrentSightComponent = SightComponent;
			}
			else
			{
				CurrentSightComponent = SightComponent;
			}
			FoundValidSight = true;
		}
	}
	if (FoundValidSight)
	{
		if (!HasAuthority())
		{
			Server_CycleSights(CurrentSightComponent);
		}
	}
	if (GetCharacterComponent())
	{
		if (UFPSTemplateAnimInstance* AnimInstance = CharacterComponent->GetAnimationInstance())
		{
			AnimInstance->CycledSights();
		}
	}

	if (GetCharacterComponent())
	{
		ActivateCurrentSight(GetCharacterComponent()->IsAiming());
	}
}

void AFPSTemplateFirearm::RefreshCurrentSight()
{
	if (!bCanCycleSights)
	{
		return;
	}
	
	if (CurrentSightComponent)
	{
		if (!HasAuthority())
		{
			Server_CycleSights(CurrentSightComponent);
		}
		if (GetCharacterComponent())
		{
			if (UFPSTemplateAnimInstance* AnimInstance = CharacterComponent->GetAnimationInstance())
			{
				AnimInstance->CycledSights();
			}
		}
	}
	else
	{
		CycleSights();
	}

	if (GetCharacterComponent())
	{
		ActivateCurrentSight(GetCharacterComponent()->IsAiming());
	}
}

void AFPSTemplateFirearm::SetSight_Implementation(UFPSTemplate_PartComponent* SightComponent)
{
	const TArray<UFPSTemplate_PartComponent*> PartComponents = FirearmPartComponentsOwner.GetPartComponents(EPartOwnerIndex::Sight);
	if (IsValid(SightComponent) && PartComponents.Contains(SightComponent))
	{
		CurrentSightComponent = SightComponent;
		RefreshCurrentSight();
	}
}

bool AFPSTemplateFirearm::Server_SetHidden_Validate(bool Hide)
{
	return true;
}

void AFPSTemplateFirearm::Server_SetHidden_Implementation(bool Hide)
{
	HideFirearm(Hide);
}

void AFPSTemplateFirearm::HideFirearm(bool Hide)
{
	if (Hide != bFirearmHidden)
	{
		bFirearmHidden = Hide;
		OnRep_FirearmHidden();
		if (!HasAuthority())
		{
			Server_SetHidden(Hide);
		}
	}
}

bool AFPSTemplateFirearm::Server_AttachToSocket_Validate(const FName& Socket)
{
	return true;
}

void AFPSTemplateFirearm::Server_AttachToSocket_Implementation(const FName& Socket)
{
	AttachToSocket(Socket);
}

void AFPSTemplateFirearm::OnRep_AttachedToSocket()
{
	if (AttachedToSocket == NAME_None)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		return;
	}
	if (GetCharacterComponent())
	{
		CharacterComponent->AttachItem(this, AttachedToSocket);
		OnAttachedToSocket(AttachedToSocket);
	}
}

void AFPSTemplateFirearm::AttachToSocket(const FName Socket)
{
	AttachedToSocket = Socket;
	OnRep_AttachedToSocket();
	if (!HasAuthority())
	{
		Server_AttachToSocket(Socket);
	}
}

void AFPSTemplateFirearm::DestroyAllParts()
{
	for (AFPSTemplate_PartBase* Part : GetFirearmParts())
	{
		if (IsValid(Part))
		{
			Part->Destroy();
		}
	}
}

AFPSTemplate_SightBase* AFPSTemplateFirearm::GetCurrentSight() const
{
	if (CurrentSightComponent)
	{
		return CurrentSightComponent->GetPart<AFPSTemplate_SightBase>();
	}
	return nullptr;
}
