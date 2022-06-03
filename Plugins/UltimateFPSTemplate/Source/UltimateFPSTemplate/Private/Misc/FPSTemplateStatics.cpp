// Copyright 2021, Dakota Dawe, All rights reserved


#include "Misc/FPSTemplateStatics.h"
#include "Misc/FPSTemplate_PhysicalMaterial.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Components/FPSTemplate_PartComponent.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Particles/ParticleSystem.h"
#include "JsonObjectConverter.h"
#include "Components/DecalComponent.h"
#include "HAL/FileManagerGeneric.h"
#include "Misc/FileHelper.h"
#include "Sound/SoundBase.h"
#include "Misc/Paths.h"

constexpr float SpeedOfSoundMs = 343.0f;

/*
void UFPSTemplateStatics::SpawnProjectileImpactEffects(const FHitResult& HitResult, const FEffectImpactSettings& DecalImpactSettings, const FSoundSettings& SoundImpactSettings)
{
	if (UFPSTemplate_PhysicalMaterial* PhysMat = Cast<UFPSTemplate_PhysicalMaterial>(HitResult.PhysMaterial))
	{
		FImpactEffects ImpactEffects = PhysMat->ProjectileImpactEffects;
		ImpactEffects.EffectSettings = DecalImpactSettings;
		ImpactEffects.SoundSettings = SoundImpactSettings;
		SpawnEffects(HitResult, ImpactEffects);
	}
}

void UFPSTemplateStatics::SpawnFootstepImpactEffects(const FHitResult& HitResult, const FEffectImpactSettings& DecalImpactSettings, const FSoundSettings& SoundImpactSettings)
{
	if (UFPSTemplate_PhysicalMaterial* PhysMat = Cast<UFPSTemplate_PhysicalMaterial>(HitResult.PhysMaterial))
	{
		FImpactEffects ImpactEffects = PhysMat->FootstepImpactEffect;
		ImpactEffects.EffectSettings = DecalImpactSettings;
		ImpactEffects.SoundSettings = SoundImpactSettings;
		SpawnEffects(HitResult, ImpactEffects);
	}
}*/

void UFPSTemplateStatics::SpawnImpactEffect(const FHitResult& HitResult, const FImpactEffects& ImpactEffect)
{
	SpawnEffects(HitResult, ImpactEffect);
}

void UFPSTemplateStatics::SpawnImpactEffectSoundDelayed(const FHitResult& HitResult, const FImpactEffects& ImpactEffect)
{
	SpawnEffects(HitResult, ImpactEffect, true);
}

void UFPSTemplateStatics::SpawnEffects(const FHitResult& HitResult, const FImpactEffects& ImpactEffects, bool bDelayed)
{
	if (const AActor* WorldActor = HitResult.GetActor())
	{
		if (const UWorld* World = WorldActor->GetWorld())
		{
			const FSoundSettings SoundSettings = ImpactEffects.SoundSettings;
			const float Delay = GetSpeedOfSoundDelayActor(HitResult.GetActor(), UGameplayStatics::GetPlayerCharacter(World, 0));

			if (!bDelayed || Delay < 0.15f)
			{
				if (USoundBase* ImpactSound = ImpactEffects.SoundSettings.Sound)
				{
					UGameplayStatics::SpawnSoundAtLocation(World, ImpactSound, HitResult.Location, FRotator::ZeroRotator, SoundSettings.VolumeMultiplier, SoundSettings.PitchMultiplier, 0.0f, ImpactSound->AttenuationSettings);
				}
			}
			else
			{
				PlaySoundDelayed(WorldActor, HitResult.Location, SoundSettings);
			}
			
			const FEffectImpactSettings DecalImpact = ImpactEffects.EffectSettings;
			FRotator Rotation = FRotator::ZeroRotator;
			if (!DecalImpact.bUseParticlesRotation)
			{
				Rotation = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
			}
			
			if (UFXSystemAsset* ImpactEffect = ImpactEffects.EffectSettings.ImpactEffect)
			{
				if (UNiagaraSystem* NEffect = Cast<UNiagaraSystem>(ImpactEffect))
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, NEffect, HitResult.Location, Rotation);
				}
				else if (UParticleSystem* PEffect = Cast<UParticleSystem>(ImpactEffect))
				{
					UGameplayStatics::SpawnEmitterAtLocation(World, PEffect, HitResult.Location, Rotation);
				}
			}
			
			if (UMaterialInstance* ImpactDecal = ImpactEffects.EffectSettings.ImpactDecal)
			{
				if (USceneComponent* HitComponent = HitResult.GetComponent())
				{
					Rotation.Pitch += 180.0f;
					if (UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAttached(ImpactDecal, DecalImpact.DecalSize, HitComponent, NAME_None,
						HitResult.Location, Rotation, EAttachLocation::KeepWorldPosition, DecalImpact.DecalLifeTime))
					{
						DecalComponent->SetFadeScreenSize(ImpactEffects.EffectSettings.DecalFadeScreenSize);
					}
				}
			}
		}
	}
}

void UFPSTemplateStatics::PlaySoundDelayed(const AActor* WorldActor, const FVector& Location, FSoundSettings SoundSettings)
{
	if (WorldActor && WorldActor->GetWorld())
	{
		if (const ACharacter* LocalCharacter = UGameplayStatics::GetPlayerCharacter(WorldActor->GetWorld(), 0))
		{
			const float Delay = GetSpeedOfSoundDelayLocation(Location, LocalCharacter->GetActorLocation());
			if (Delay < 0.15f)
			{
				if (USoundBase* ImpactSound = SoundSettings.Sound)
				{
					UGameplayStatics::SpawnSoundAtLocation(WorldActor->GetWorld(), ImpactSound, Location, FRotator::ZeroRotator, SoundSettings.VolumeMultiplier, SoundSettings.PitchMultiplier, 0.0f, ImpactSound->AttenuationSettings);
				}
			}
			else
			{
				FTimerHandle FSoundDelay;
				const FTimerDelegate SoundDelayDelegate = FTimerDelegate::CreateStatic(&UFPSTemplateStatics::PlaySoundDelayed_Implementation, WorldActor, Location, SoundSettings);
				WorldActor->GetWorld()->GetTimerManager().SetTimer(FSoundDelay, SoundDelayDelegate, Delay, false);
			}
		}
	}
}

void UFPSTemplateStatics::PlaySoundDelayed_Implementation(const AActor* WorldActor, const FVector Location,
	FSoundSettings SoundSettings)
{
	if (WorldActor && WorldActor->GetWorld())
	{
		if (USoundBase* ImpactSound = SoundSettings.Sound)
		{
			UGameplayStatics::SpawnSoundAtLocation(WorldActor->GetWorld(), ImpactSound, Location, FRotator::ZeroRotator, SoundSettings.VolumeMultiplier, SoundSettings.PitchMultiplier, 0.0f, ImpactSound->AttenuationSettings);
		}
	}
}

float UFPSTemplateStatics::GetSpeedOfSoundDelayLocation(const FVector& LocationA, const FVector& LocationB)
{
	const float Distance = FVector::Distance(LocationA, LocationB) / 100.0f;
	return Distance / SpeedOfSoundMs;
}

float UFPSTemplateStatics::GetSpeedOfSoundDelayActor(const AActor* ActorA, const AActor* ActorB)
{
	if (ActorA && ActorB)
	{
		return GetSpeedOfSoundDelayLocation(ActorA->GetActorLocation(), ActorB->GetActorLocation());
	}
	return 0.1f;
}

FRotator UFPSTemplateStatics::SetMuzzleMOA(FRotator MuzzleRotation, float MOA)
{
	MOA *= 0.016668f; // Sets MOA to be in exact adjustments. 1 inch at 100 yards = 0.016668
	float MOAChange = (FMath::RandRange(-MOA / 2.0f, MOA / 2.0f));
	MuzzleRotation.Yaw += MOAChange;
	
	MOAChange = (FMath::RandRange(-MOA / 2.0f, MOA / 2.0f));
	MuzzleRotation.Pitch += MOAChange * 0.6f; // reduce vertical MOA shift for consistency

	return MuzzleRotation;
}

FRotator UFPSTemplateStatics::GetEstimatedMuzzleToScopeZero(const FTransform& MuzzleTransform,
	const FTransform& SightTransform, const float RangeMeters)
{
	FVector EndLocation = SightTransform.GetLocation() + SightTransform.GetRotation().Vector() * RangeMeters;
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleTransform.GetLocation(), EndLocation);
	LookAtRotation.Pitch += 0.04f; // Increase vertical projectile launch angle (thanks gravity)
	return LookAtRotation;
}

FTransform UFPSTemplateStatics::FixTransform(const FTransform& Transform)
{
	FVector Vec = Transform.GetLocation();
	const float TempVal = Vec.Z;
	Vec.Z = Vec.X;
	Vec.X = TempVal;

	return FTransform(Transform.Rotator(), Vec, Transform.GetScale3D());
}

bool UFPSTemplateStatics::Ragdoll(UPrimitiveComponent* MeshComponent)
{
	if (Cast<USkeletalMeshComponent>(MeshComponent))
	{
		if (ACharacter* Character = Cast<ACharacter>(MeshComponent->GetOwner()))
		{
			Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetSimulatePhysics(true);
		return true;
	}
	return false;
}

void UFPSTemplateStatics::RagdollWithImpact(UPrimitiveComponent* MeshComponent, FVector ImpactLocation, float ImpactForce)
{
	if (Ragdoll(MeshComponent) && ImpactLocation != FVector::ZeroVector && ImpactForce != 0.0f)
	{
		ImpactForce *= MeshComponent->GetMass();
		MeshComponent->AddRadialImpulse(ImpactLocation, 100.0f, ImpactForce, ERadialImpulseFalloff::RIF_Linear);
	}
}

FProjectileTransform UFPSTemplateStatics::CreateProjectileTransform(const FTransform& MuzzleTransform)
{
	return FProjectileTransform(MuzzleTransform);
}

FTransform UFPSTemplateStatics::CreateTransformForProjectile(const FProjectileTransform& ProjectileTransform)
{
	return FProjectileTransform::GetTransformFromProjectile(ProjectileTransform);
}

FFirearm UFPSTemplateStatics::GetFirearmStruct(AFPSTemplateFirearm* Firearm)
{
	if (Firearm)
	{
		FFirearm FirearmStruct;
		FirearmStruct.FirearmClass = Firearm->GetClass();
		for (UFPSTemplate_PartComponent* PartComponent : Firearm->GetAllPartComponents())
		{// && PartComponent->GetPart()->GetOwner() == Firearm
			if (PartComponent && IsValid(PartComponent->GetPart()))
			{
				FFirearmPartList PartList;
				PartList.ComponentName = PartComponent->GetName();
				PartList.Part = PartComponent->GetPart()->GetClass();
				PartList.PartOffset = PartComponent->GetPart()->GetPartOffset();
				if (PartComponent->GetPart()->GetOwner() == Firearm)
				{
					PartList.ParentFirearm = PartComponent->GetPart()->GetOwner()->GetClass();
				}
				else
				{
					PartList.Parent = PartComponent->GetPart()->GetOwner()->GetClass();
				}
				FirearmStruct.PartsList.Add(PartList);
			}
		}
		return FirearmStruct;
	}
	return FFirearm();
}

FString UFPSTemplateStatics::SerializeFirearm(AFPSTemplateFirearm* Firearm, FString& Error)
{
	if (!IsValid(Firearm))
	{
		Error = "Firearm INVALID";
		return "";
	}
	FString SerializedString;
	if (Firearm)
	{
		FFirearm FirearmStruct = GetFirearmStruct(Firearm);
		
		if (!FJsonObjectConverter::UStructToJsonObjectString(FirearmStruct, SerializedString))
		{
			Error = "Could Not Serialize String";
			return "";
		}
	}

	//FPlatformMisc::ClipboardCopy(ToCStr(SerializedString));
	return SerializedString;
}

FFirearm UFPSTemplateStatics::DeserializeFirearm(const FString& JsonString, FString& Error)
{
	FFirearm FirearmStruct;
	if (FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &FirearmStruct, 0, 0))
	{
		FPSLog(TEXT("SUCCESS DESERIALIZED"));
	}
	else
	{
		Error = "Could NOT Deserialize Json String";
	}

	return FirearmStruct;
}

AFPSTemplateFirearm* UFPSTemplateStatics::ConstructFirearm(AActor* WorldActor, FFirearm FirearmStruct, FString& Error)
{
	if (!IsValid(WorldActor))
	{
		Error = "WorldActor INVALID";
		return nullptr;
	}
	if (!WorldActor->HasAuthority())
	{
		Error = "NO AUTHORITY";
		return nullptr;
	}
	
	UWorld* World = WorldActor->GetWorld();
	if (!World)
	{
		Error = "Could NOT Get World";
		return nullptr;
	}

	if (AFPSTemplateFirearm* Firearm = World->SpawnActorDeferred<AFPSTemplateFirearm>(FirearmStruct.FirearmClass, FTransform()))
	{
		Firearm->SetIsLoadedFromPreset();
		UGameplayStatics::FinishSpawningActor(Firearm, FTransform());
		for (FFirearmPartList& PartList : FirearmStruct.PartsList)
		{
			PartList.bHasBeenCreated = true;
			if (PartList.ParentFirearm)
			{
				for (UFPSTemplate_PartComponent* PartComponent : Firearm->GetAllPartComponents())
				{
					//UE_LOG(LogTemp, Warning, TEXT("PartComp: %s"), *PartComponent->GetName());
					if (PartComponent && PartComponent->GetName().Equals(PartList.ComponentName))
					{
						if (PartComponent->AddPart(PartList.Part) && IsValid(PartComponent->GetPart()))
						{
							if (PartComponent->GetPart()->IsMovementInverted())
							{
								PartList.PartOffset *= -1.0f;
							}
							PartComponent->GetPart()->SetOffset(PartList.PartOffset);
						}
					}
				}
			}
			else
			{
				for (UFPSTemplate_PartComponent* PartComponent : Firearm->GetAllPartComponents())
				{
					if (PartComponent && PartComponent->GetName().Equals(PartList.ComponentName))
					{
						//FPSLog(TEXT("Matching Part: %s	 To Part Component: %s"), *PartList.Part->GetName(), *PartComponent->GetName());
						if (PartComponent->AddPart(PartList.Part) && IsValid(PartComponent->GetPart()))
						{
							PartComponent->GetPart()->SetOffset(PartList.PartOffset);
						}
					}
				}
			}
		}
		return Firearm;
	}

	return nullptr;
}

bool UFPSTemplateStatics::SaveStringToFile(const FString& Path, const FString& FileName, const FString& FileContent)
{
	return FFileHelper::SaveStringToFile(FileContent, *(Path + FileName));
}

bool UFPSTemplateStatics::LoadFileToString(const FString& Path, const FString& FileName, FString& OutString)
{
	return FFileHelper::LoadFileToString(OutString, *(Path + FileName));
}

bool UFPSTemplateStatics::DeleteFile(const FString& Path, const FString& FileName)
{
	IPlatformFile& PlatformFile = IPlatformFile::GetPlatformPhysical();
	return PlatformFile.DeleteFile(*(Path + FileName));
}

bool UFPSTemplateStatics::GetAllFiles(FString Path, TArray<FString>& OutFiles)
{
	bool ValidFiles = false;
	if (FPaths::DirectoryExists(Path))
	{
		ValidFiles = true;
		Path.Append("*");
		FFileManagerGeneric::Get().FindFiles(OutFiles, *Path, true, false);
	}

	return ValidFiles;
}

FString UFPSTemplateStatics::RoundFloat2Pos(float Value)
{
	return FString::Printf(TEXT("%.2f"), Value);
}

FString UFPSTemplateStatics::RoundFloat3Pos(float Value)
{
return FString::Printf(TEXT("%.3f"), Value);
}

FString UFPSTemplateStatics::GetFormattedCopyPasteTransform(const FTransform& Transform)
{
	const FQuat Rot = Transform.GetRotation();
	const FVector Loc = Transform.GetLocation();
	FString RotStr = FString::Printf(TEXT("(Rotation=(X=%f,Y=%f,Z=%f,W=%f),Translation=(X=%f,Y=%f,Z=%f),Scale3D=(X=1.000000,Y=1.000000,Z=1.000000))"), Rot.X, Rot.Y, Rot.Z, Rot.W, Loc.X, Loc.Y, Loc.Z);

	return RotStr;
}

int32 UFPSTemplateStatics::GetRandomIndexForArray(uint8 ArrayLength)
{
	if (ArrayLength)
	{
		if (ArrayLength == 1)
		{
			return 0;
		}
		return FMath::RandRange(0, ArrayLength - 1);
	}
	return INDEX_NONE;
}

bool UFPSTemplateStatics::ChangeTransformProperty(UObject* Object, const FTransform& NewTransform, const FName NameOfProperty)
{
	if (Object)
	{
		if (const UClass* Class = Object->GetClass())
		{
			if (UObject* ClassDefault = Class->GetDefaultObject())
			{
				if (const FProperty* Property = Class->FindPropertyByName(NameOfProperty))
				{
					if (FTransform* Transform = Property->ContainerPtrToValuePtr<FTransform>(ClassDefault))
					{
						Transform->SetLocation(NewTransform.GetLocation());
						Transform->SetRotation(NewTransform.GetRotation());
						Transform->SetScale3D(NewTransform.GetScale3D());
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool UFPSTemplateStatics::ChangeVectorProperty(UObject* Object, const FVector& NewVector, const FName NameOfProperty)
{
	if (Object)
	{
		if (const UClass* Class = Object->GetClass())
		{
			if (UObject* ClassDefault = Class->GetDefaultObject())
			{
				if (const FProperty* Property = Class->FindPropertyByName(NameOfProperty))
				{
					if (FVector* Vector = Property->ContainerPtrToValuePtr<FVector>(ClassDefault))
					{
						Vector->X = NewVector.X;
						Vector->Y = NewVector.Y;
						Vector->Z = NewVector.Z;
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool UFPSTemplateStatics::ChangeRotatorProperty(UObject* Object, const FRotator& NewRotator, const FName NameOfProperty)
{
	if (Object)
	{
		if (const UClass* Class = Object->GetClass())
		{
			if (UObject* ClassDefault = Class->GetDefaultObject())
			{
				if (const FProperty* Property = Class->FindPropertyByName(NameOfProperty))
				{
					if (FRotator* Rotator = Property->ContainerPtrToValuePtr<FRotator>(ClassDefault))
					{
						Rotator->Roll = NewRotator.Roll;
						Rotator->Yaw = NewRotator.Yaw;
						Rotator->Pitch = NewRotator.Pitch;
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool UFPSTemplateStatics::ChangeFloatProperty(UObject* Object, const float NewFloat, const FName NameOfProperty)
{
	if (Object)
	{
		if (const UClass* Class = Object->GetClass())
		{
			if (UObject* ClassDefault = Class->GetDefaultObject())
			{
				if (const FFloatProperty* Property = CastField<FFloatProperty>(Class->FindPropertyByName(NameOfProperty)))
				{
					Property->SetPropertyValue_InContainer(ClassDefault, NewFloat);
					return true;
				}
			}
		}
	}

	return false;
}

bool UFPSTemplateStatics::ChangeIntProperty(UObject* Object, const int32 NewInt, const FName NameOfProperty)
{
	if (Object)
	{
		if (const UClass* Class = Object->GetClass())
		{
			if (UObject* ClassDefault = Class->GetDefaultObject())
			{
				if (const FIntProperty* Property = CastField<FIntProperty>(Class->FindPropertyByName(NameOfProperty)))
				{
					Property->SetPropertyValue_InContainer(ClassDefault, NewInt);
					return true;
				}
			}
		}
	}

	return false;
}

bool UFPSTemplateStatics::ChangeInt64Property(UObject* Object, const int64 NewInt, const FName NameOfProperty)
{
	if (Object)
	{
		if (const UClass* Class = Object->GetClass())
		{
			if (UObject* ClassDefault = Class->GetDefaultObject())
			{
				if (const FInt64Property* Property = CastField<FInt64Property>(Class->FindPropertyByName(NameOfProperty)))
				{
					Property->SetPropertyValue_InContainer(ClassDefault, NewInt);
					return true;
				}
			}
		}
	}

	return false;
}

bool UFPSTemplateStatics::ChangeByteProperty(UObject* Object, const uint8 NewByte, const FName NameOfProperty)
{
	if (Object)
	{
		if (const UClass* Class = Object->GetClass())
		{
			if (UObject* ClassDefault = Class->GetDefaultObject())
			{
				if (const FByteProperty* Property = CastField<FByteProperty>(Class->FindPropertyByName(NameOfProperty)))
				{
					Property->SetPropertyValue_InContainer(ClassDefault, NewByte);
					return true;
				}
			}
		}
	}

	return false;
}

bool UFPSTemplateStatics::ChangeBoolProperty(UObject* Object, const bool NewBool, const FName NameOfProperty)
{
	if (Object)
	{
		if (const UClass* Class = Object->GetClass())
		{
			if (UObject* ClassDefault = Class->GetDefaultObject())
			{
				if (const FBoolProperty* Property = CastField<FBoolProperty>(Class->FindPropertyByName(NameOfProperty)))
				{
					Property->SetPropertyValue_InContainer(ClassDefault, NewBool);
					return true;
				}
			}
		}
	}

	return false;
}

bool UFPSTemplateStatics::ChangeStringProperty(UObject* Object, const FString& NewString, const FName NameOfProperty)
{
	if (Object)
	{
		if (const UClass* Class = Object->GetClass())
		{
			if (UObject* ClassDefault = Class->GetDefaultObject())
			{
				if (const FStrProperty* Property = CastField<FStrProperty>(Class->FindPropertyByName(NameOfProperty)))
				{
					Property->SetPropertyValue_InContainer(ClassDefault, NewString);
					return true;
				}
			}
		}
	}

	return false;
}

bool UFPSTemplateStatics::ChangeNameProperty(UObject* Object, const FName& NewName, const FName NameOfProperty)
{
	if (Object)
	{
		if (const UClass* Class = Object->GetClass())
		{
			if (UObject* ClassDefault = Class->GetDefaultObject())
			{
				if (const FNameProperty* Property = CastField<FNameProperty>(Class->FindPropertyByName(NameOfProperty)))
				{
					Property->SetPropertyValue_InContainer(ClassDefault, NewName);
					return true;
				}
			}
		}
	}

	return false;
}

bool UFPSTemplateStatics::ChangeTextProperty(UObject* Object, const FText& NewText, const FName NameOfProperty)
{
	if (Object)
	{
		if (const UClass* Class = Object->GetClass())
		{
			if (UObject* ClassDefault = Class->GetDefaultObject())
			{
				if (const FTextProperty* Property = CastField<FTextProperty>(Class->FindPropertyByName(NameOfProperty)))
				{
					Property->SetPropertyValue_InContainer(ClassDefault, NewText);
					return true;
				}
			}
		}
	}

	return false;
}

bool UFPSTemplateStatics::CalculateAdjustedMatrix(const UMeshComponent* Component, APlayerController* PlayerController, float DesiredFOV, FMatrix& OutMatrix)
{
	if (Component && PlayerController && PlayerController->PlayerCameraManager)
	{
		const float WorldFOV = PlayerController->PlayerCameraManager->GetFOVAngle();
		const float ViewmodelFOV = DesiredFOV;

		FVector ViewOrigin;
		FRotator ViewRotation;
		PlayerController->GetPlayerViewPoint(ViewOrigin, ViewRotation);

		const float WorldHalfFOVRadians = FMath::DegreesToRadians(FMath::Max(0.001f, WorldFOV)) / 2.0f;
		const float DesiredHalfFOVRadians = FMath::DegreesToRadians(FMath::Max(0.001f, ViewmodelFOV)) / 2.0f;
		const float FOVRatio = WorldHalfFOVRadians / DesiredHalfFOVRadians;

		const FMatrix PerspectiveAdjustmentMatrix = FMatrix(
			FPlane(FOVRatio, 0, 0, 0),
			FPlane(0, FOVRatio, 0, 0),
			FPlane(0, 0, 1, 0),
			FPlane(0, 0, 0, 1));

		FMatrix ViewRotationMatrix = FInverseRotationMatrix(ViewRotation) * FMatrix(
			   FPlane(0, 0, 1, 0),
			   FPlane(1, 0, 0, 0),
			   FPlane(0, 1, 0, 0),
			   FPlane(0, 0, 0, 1));
	
		if (!ViewRotationMatrix.GetOrigin().IsNearlyZero(0.0f))
		{
			ViewOrigin += ViewRotationMatrix.InverseTransformPosition(FVector::ZeroVector);
			ViewRotationMatrix = ViewRotationMatrix.RemoveTranslation();
		}

		const FMatrix ViewMatrix = FTranslationMatrix(-ViewOrigin) * ViewRotationMatrix;
		const FMatrix InverseViewMatrix = FTranslationMatrix(-ViewMatrix.GetOrigin()) * ViewMatrix.RemoveTranslation().GetTransposed();
		//Compensate for FOV Difference
		const float OffsetRange = FMath::GetMappedRangeValueUnclamped(FVector2D(45.f, 71.f), FVector2D(4.5f, -4.5f), DesiredFOV);
		
		const FVector CompensationOffset = ViewRotation.Vector() * OffsetRange;
		const FMatrix FOVCompensation = FTransform(FRotator::ZeroRotator, CompensationOffset, FVector::OneVector).ToMatrixNoScale();

		OutMatrix = Component->GetComponentToWorld().ToMatrixWithScale() * ViewMatrix * PerspectiveAdjustmentMatrix * InverseViewMatrix * FOVCompensation;

		return true;
	}
	return false;
}

FTransform UFPSTemplateStatics::ProjectileTransformToTransform(const FProjectileTransform& ProjectileTransform)
{
	return ProjectileTransform.GetTransformFromProjectile();
}

FString UFPSTemplateStatics::GetTextAfterPeriod(const FString& INString)
{
	int32 LastPeriod;
	INString.FindLastChar('.', LastPeriod);
	if (LastPeriod == INDEX_NONE)
	{
		return INString;
	}

	return INString.RightChop(LastPeriod + 1);
}