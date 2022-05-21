// Copyright 2021, Dakota Dawe, All rights reserved


#include "FirearmParts/FPSTemplate_Barrel.h"
#include "Actors/FPSTemplateFirearm.h"
#include "FirearmParts/FPSTemplate_Muzzle.h"
#include "Components/FPSTemplate_PartComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AFPSTemplate_Barrel::AFPSTemplate_Barrel()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bUseMuzzleTemperature = false;
	TemperatureIncreasePerShot = 4.0f;
	TemperatureDecreasePerTick = 15.0f;
	MinTemperatureEffect = 80.0f;
	MaxTemperatureEffect = 1000.0f;
	MuzzleTemperature = 0.0f;
	MuzzleTemperatureNormalized = 0.0f;
	TemperatureCooldownTickRate = 10.0f;
	
	MuzzleSocket = FName("S_Muzzle");
	PartStats.Weight = 0.7f;
	PartStats.ErgonomicsChangePercentage = -5.0f;
	PartStats.RecoilChangePercentage = -2.0f;

	PartType = EPartType::Barrel;
}

// Called when the game starts or when spawned
void AFPSTemplate_Barrel::BeginPlay()
{
	Super::BeginPlay();

	if (!bUseMuzzleTemperature)
	{
		PrimaryActorTick.bCanEverTick = false;
	}
	else
	{
		if (PartMesh.IsValid())
		{
			MuzzleMaterial = PartMesh->CreateDynamicMaterialInstance(0);
			if (MuzzleMaterial.IsValid())
			{
				PartMesh->SetMaterial(0, MuzzleMaterial.Get());
				SetActorTickInterval(1.0f / TemperatureCooldownTickRate);
			}
		}
	}

	SetActorTickEnabled(false);
}

void AFPSTemplate_Barrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSTemplate_Barrel, MuzzleDevice);
}

void AFPSTemplate_Barrel::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (MuzzleMaterial.IsValid())
	{
		MuzzleMaterial->SetScalarParameterValue(FName("MuzzleTemperature"), MuzzleTemperatureNormalized);
	}
	MuzzleTemperature -= TemperatureDecreasePerTick * DeltaSeconds;
	if (MuzzleTemperature < 0.0f)
	{
		MuzzleTemperature = 0.0f;
		SetActorTickEnabled(false);
	}
	NormalizeTemperature();
}

FTransform AFPSTemplate_Barrel::GetMuzzleSocketTransform() const
{
	if (IsValid(MuzzleDevice))
	{
		return MuzzleDevice->GetMuzzleSocketTransform();
	}
	return PartMesh.IsValid() ? PartMesh->GetSocketTransform(MuzzleSocket) : FTransform();
}

bool AFPSTemplate_Barrel::DoesMuzzleSocketExist() const
{
	return PartMesh.IsValid() ? PartMesh->DoesSocketExist(MuzzleSocket) : false;
}

AFPSTemplate_Muzzle* AFPSTemplate_Barrel::GetMuzzleDevice()
{
	return Cast<AFPSTemplate_Muzzle>(GetMuzzleDeviceActor());
}

AActor* AFPSTemplate_Barrel::GetMuzzleDeviceActor()
{
	if (IsValid(MuzzleDevice))
	{
		return MuzzleDevice->GetMuzzleAttachment();
	}
	return this;
}

void AFPSTemplate_Barrel::CacheParts()
{
	for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent)
		{
			AFPSTemplate_Muzzle* Muzzle = PartComponent->GetPart<AFPSTemplate_Muzzle>();
			if (IsValid(Muzzle))
			{
				MuzzleDevice = Muzzle;
			}
		}
	}
}

void AFPSTemplate_Barrel::NormalizeTemperature()
{
	MuzzleTemperatureNormalized = UKismetMathLibrary::NormalizeToRange(MuzzleTemperature, MinTemperatureEffect, MaxTemperatureEffect);
	MuzzleTemperatureNormalized = FMath::Clamp(MuzzleTemperatureNormalized, 0.0f, 1.0f);
	MuzzleTemperatureChanged(MuzzleTemperature);
	//UE_LOG(LogTemp, Warning, TEXT("Temperature: %f   Normalized: %f"), MuzzleTemperature, MuzzleTemperatureNormalized);
}

void AFPSTemplate_Barrel::FiredShot()
{
	if (bUseMuzzleTemperature)
	{
		SetActorTickEnabled(true);
		MuzzleTemperature += TemperatureIncreasePerShot;
		NormalizeTemperature();
	}
}
