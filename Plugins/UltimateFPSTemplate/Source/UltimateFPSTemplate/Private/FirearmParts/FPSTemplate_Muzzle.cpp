// Copyright 2021, Dakota Dawe, All rights reserved


#include "FirearmParts/FPSTemplate_Muzzle.h"
#include "FirearmParts/FPSTemplate_Barrel.h"
#include "Components/FPSTemplate_PartComponent.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/FPSTemplateStatics.h"

#include "Net/UnrealNetwork.h"

AFPSTemplate_Muzzle::AFPSTemplate_Muzzle()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	NetUpdateFrequency = 1.0f;
	
	bIsSuppressor = false;
	bUseMuzzleTemperature = false;
	TemperatureIncreasePerShot = 4.0f;
	TemperatureDecreasePerTick = 15.0f;
	MinTemperatureEffect = 80.0f;
	MaxTemperatureEffect = 1000.0f;
	MuzzleTemperature = 0.0f;
	MuzzleTemperatureNormalized = 0.0f;
	TemperatureCooldownTickRate = 10.0f;
	
	MuzzleSocket = FName("S_Muzzle");
	
	PartStats.Weight = 1.6f;
	PartStats.RecoilChangePercentage = -10.0f;

	PartType = EPartType::MuzzleDevice;
}

void AFPSTemplate_Muzzle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSTemplate_Muzzle, MuzzleAttachment);
}

void AFPSTemplate_Muzzle::BeginPlay()
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

void AFPSTemplate_Muzzle::Tick(float DeltaSeconds)
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

FTransform AFPSTemplate_Muzzle::GetMuzzleSocketTransform()
{
	if (IsValid(MuzzleAttachment) && MuzzleAttachment->PartMesh.IsValid())
	{
		return MuzzleAttachment->PartMesh->GetSocketTransform(MuzzleSocket);
	}
	return PartMesh.IsValid() ? PartMesh->GetSocketTransform(MuzzleSocket) : FTransform();
}

bool AFPSTemplate_Muzzle::DoesMuzzleSocketExist()
{
	return PartMesh.IsValid() ? PartMesh->DoesSocketExist(MuzzleSocket) : false;
}

bool AFPSTemplate_Muzzle::IsSuppressor() const
{
	if (IsValid(MuzzleAttachment))
	{
		return MuzzleAttachment->bIsSuppressor;
	}
	return bIsSuppressor;
}

UNiagaraSystem* AFPSTemplate_Muzzle::GetFireNiagaraSystem()
{
	const int32 RandomIndex = UFPSTemplateStatics::GetRandomIndexForArray(FireNiagaraSystems.Num());
	if (RandomIndex != INDEX_NONE)
	{
		return FireNiagaraSystems[RandomIndex];
	}
	return nullptr;
}

void AFPSTemplate_Muzzle::CacheParts()
{
	for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent)
		{
			AFPSTemplate_Muzzle* Muzzle = PartComponent->GetPart<AFPSTemplate_Muzzle>();
			if (IsValid(Muzzle))
			{
				MuzzleAttachment = Muzzle;
			}
		}
	}
}

void AFPSTemplate_Muzzle::NormalizeTemperature()
{
	MuzzleTemperatureNormalized = UKismetMathLibrary::NormalizeToRange(MuzzleTemperature, MinTemperatureEffect, MaxTemperatureEffect);
	MuzzleTemperatureNormalized = FMath::Clamp(MuzzleTemperatureNormalized, 0.0f, 1.0f);
	MuzzleTemperatureChanged(MuzzleTemperature);
	//UE_LOG(LogTemp, Warning, TEXT("Temperature: %f   Normalized: %f"), MuzzleTemperature, MuzzleTemperatureNormalized);
}

void AFPSTemplate_Muzzle::FiredShot()
{
	if (bUseMuzzleTemperature)
	{
		SetActorTickEnabled(true);
		MuzzleTemperature += TemperatureIncreasePerShot;
		NormalizeTemperature();
	}
}
