// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FPSTemplateDataTypes.h"
#include "Engine/EngineTypes.h"
#include "FPSTemplateStatics.generated.h"

#define FPSLog(Text, ...)\
{\
	UE_LOG(LogTemp, Warning, Text, __VA_ARGS__);\
}

#define FPSLogPrint(Text, ...)\
{\
	GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, FString::Printf(Text, __VA_ARGS__));\
}

#define FPSLogNPrint(Text, ...)\
{\
	GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, FString::Printf(Text, __VA_ARGS__));\
	UE_LOG(LogTemp, Warning, Text, __VA_ARGS__);\
}

class UPrimitiveComponent;
class UMeshComponent;
class APlayerController;
class UFPSTemplate_PhysicalMaterial;
class AFPSTemplateFirearm;

UCLASS()
class ULTIMATEFPSTEMPLATE_API UFPSTemplateStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
protected:
	static float GetSpeedOfSoundDelayLocation(const FVector& LocationA, const FVector& LocationB);
	static float GetSpeedOfSoundDelayActor(const AActor* ActorA, const AActor* ActorB);
	static void PlaySoundDelayed_Implementation(const AActor* WorldActor, const FVector Location, FSoundSettings SoundSettings);
public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Effects")
	static void SpawnImpactEffect(const FHitResult& HitResult, const FImpactEffects& ImpactEffect);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Effects")
	static void SpawnImpactEffectSoundDelayed(const FHitResult& HitResult, const FImpactEffects& ImpactEffect);
	static void SpawnEffects(const FHitResult& HitResult, const FImpactEffects& ImpactEffects, bool bDelayed = false);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Effects")
	static void PlaySoundDelayed(const AActor* WorldActor, const FVector& Location, FSoundSettings SoundSettings);
	
	// Applies MOA to the muzzles rotation for accuracy. 1 MOA = 1 inch at 100 yards
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	static FRotator SetMuzzleMOA(FRotator MuzzleRotation, float MOA = 1.0f);
	// Gets the estimated zero for magnified scopes based on the sight and muzzle
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Firearm")
	static FRotator GetEstimatedMuzzleToScopeZero(const FTransform& MuzzleTransform, const FTransform& SightTransform, const float RangeMeters);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Firearm")
	static FTransform FixTransform(const FTransform& Transform);
	
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	static bool Ragdoll(UPrimitiveComponent* MeshComponent);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Character")
	static void RagdollWithImpact(UPrimitiveComponent* MeshComponent, FVector ImpactLocation = FVector::ZeroVector, float ImpactForce = 0.0f);	
	
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
	static FProjectileTransform CreateProjectileTransform(const FTransform& MuzzleTransform);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Projectile")
	static FTransform CreateTransformForProjectile(const FProjectileTransform& ProjectileTransform);
	
	static FFirearm GetFirearmStruct(AFPSTemplateFirearm* Firearm);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Serialize")
	static FString SerializeFirearm(AFPSTemplateFirearm* Firearm, FString& Error);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Serialize")
	static FFirearm DeserializeFirearm(const FString& JsonString, FString& Error);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Serialize")
	static AFPSTemplateFirearm* ConstructFirearm(AActor* WorldActor, FFirearm FirearmStruct, FString& Error);

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | File")
	static bool SaveStringToFile(const FString& Path, const FString& FileName, const FString& FileContent);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | File")
	static bool LoadFileToString(const FString& Path, const FString& FileName, FString& OutString);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | File")
	static bool DeleteFile(const FString& Path, const FString& FileName);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | File")
	static bool GetAllFiles(FString Path, TArray<FString>& OutFiles);

	static bool CalculateAdjustedMatrix(const UMeshComponent* Component, APlayerController* PlayerController, float DesiredFOV, FMatrix& OutMatrix);

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Conversion", meta = (BlueprintAutocast))
	static FTransform ProjectileTransformToTransform(const FProjectileTransform& ProjectileTransform);

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | String")
	static FString GetTextAfterPeriod(const FString& INString);
	// Rounds a float to the 2nd decimal place and returns in string format
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | String")
	static FString RoundFloat2Pos(float Value);
	// Rounds a float to the 3rd decimal place and returns in string format
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | String")
	static FString RoundFloat3Pos(float Value);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | String")
	static FString GetFormattedCopyPasteTransform(const FTransform& Transform);
	
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Array")
	static int32 GetRandomIndexForArray(uint8 ArrayLength);

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Properties")
	static bool ChangeBoolProperty(UObject* Object, const bool NewBool, const FName NameOfProperty);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Properties")
	static bool ChangeByteProperty(UObject* Object, const uint8 NewByte, const FName NameOfProperty);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Properties")
	static bool ChangeIntProperty(UObject* Object, const int32 NewInt, const FName NameOfProperty);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Properties")
	static bool ChangeInt64Property(UObject* Object, const int64 NewInt, const FName NameOfProperty);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Properties")
	static bool ChangeFloatProperty(UObject* Object, const float NewFloat, const FName NameOfProperty);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Properties")
	static bool ChangeNameProperty(UObject* Object, const FName& NewName, const FName NameOfProperty);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Properties")
	static bool ChangeStringProperty(UObject* Object, const FString& NewString, const FName NameOfProperty);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Properties")
	static bool ChangeTextProperty(UObject* Object, const FText& NewText, const FName NameOfProperty);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Properties")
	static bool ChangeVectorProperty(UObject* Object, const FVector& NewVector, const FName NameOfProperty);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Properties")
	static bool ChangeRotatorProperty(UObject* Object, const FRotator& NewRotator, const FName NameOfProperty);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Properties")
	static bool ChangeTransformProperty(UObject* Object, const FTransform& NewTransform, const FName NameOfProperty);
};
