// Copyright 2021, Dakota Dawe, All rights reserved


#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "Components/FPSTemplate_PartComponent.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Components/FPSTemplate_CharacterComponent.h"

#include "Net/UnrealNetwork.h"

#define COMPONENT_TAG_NAME FName("FPSPart")

// Sets default values
AFPSTemplate_PartBase::AFPSTemplate_PartBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	NetUpdateFrequency = 1.0f;
	MinNetUpdateFrequency = 0.5f;

	FirearmCollisionChannel = ECC_GameTraceChannel2;
	
	PartStats.Weight = 0.22f;
	PartStats.ErgonomicsChangePercentage = 0.0f;
	PartStats.RecoilChangePercentage = 0.0f;

	PartType = EPartType::Other;

	bIsAimable = false;
	AimSocket = FName("S_Aim");

	bHasRenderTarget = false;

	bInvertMovingOffset = false;

	AccumulatedOffset = 0.0f;
}

// Called when the game starts or when spawned
void AFPSTemplate_PartBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		GetComponents<UFPSTemplate_PartComponent>(PartComponents);
	}
	
	SetupPartMesh();
}

void AFPSTemplate_PartBase::SetupPartMesh()
{
	TArray<UMeshComponent*> ActorMeshComponents;
	GetComponents<UMeshComponent>(ActorMeshComponents);
	bool bFoundWithTag = false;
	for (UMeshComponent* MeshComponent : ActorMeshComponents)
	{
		if (IsValid(MeshComponent) && MeshComponent->ComponentHasTag(COMPONENT_TAG_NAME))
		{
			bFoundWithTag = true;
			PartMesh = MeshComponent;
			//PartMesh->bUseAttachParentBound = true;
			PartMesh->SetCollisionResponseToChannel(FirearmCollisionChannel, ECR_Ignore);
			PartMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			PartMesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
			PartMesh->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
			break;
		}
	}

	checkf(bFoundWithTag, TEXT("Part: %s has NO valid component with tag %s"), *GetName(), *COMPONENT_TAG_NAME.ToString());
}

void AFPSTemplate_PartBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AFPSTemplate_PartBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSTemplate_PartBase, PartComponents);
	DOREPLIFETIME(AFPSTemplate_PartBase, CurrentOffset);
}

FPrimaryAssetId AFPSTemplate_PartBase::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, AssetName);
}

bool AFPSTemplate_PartBase::Server_Use_Validate()
{
	return true;
}

void AFPSTemplate_PartBase::Server_Use_Implementation()
{
	
}

void AFPSTemplate_PartBase::OnPartRemoved_Implementation(UFPSTemplate_PartComponent* PartComponent)
{
	Destroy();
	PartsUpdated();
}

void AFPSTemplate_PartBase::CacheCharacterAndFirearm()
{
	GetOwningCharacterComponent();
}

FFirearmPartStats AFPSTemplate_PartBase::GetPartStats()
{
	FFirearmPartStats ReturnPartStats = PartStats;
	for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			PartComponent->GetPart()->CacheParts();
			ReturnPartStats += PartComponent->GetPart()->GetPartStats();
		}
	}
	return ReturnPartStats;
}

AActor* AFPSTemplate_PartBase::GetOwningActor()
{
	if (IsValid(OwningActor))
	{
		return OwningActor;
	}
	AActor* PartOwner = GetOwner();
	for (uint8 i = 0; i < MAX_PartStack; ++i)
	{
		if (IsValid(PartOwner))
		{
			if (PartOwner->Implements<UFPSTemplate_AttachmentInterface>())
			{
				OwningActor = PartOwner;
				return OwningActor;
			}

			PartOwner = PartOwner->GetOwner();
		}
	}
	return nullptr;
}

UFPSTemplate_CharacterComponent* AFPSTemplate_PartBase::GetOwningCharacterComponent()
{
	if (IsValid(OwningCharacterComponent))
	{
		return OwningCharacterComponent;
	}
	AFPSTemplateFirearm* Firearm = Cast<AFPSTemplateFirearm>(GetOwningActor());
	if (IsValid(Firearm))
	{
		if (UFPSTemplate_CharacterComponent* CharacterComponent = Firearm->GetCharacterComponent())
		{
			OwningCharacterComponent = CharacterComponent;
			return CharacterComponent;
		}
	}
	return nullptr;
}

TArray<UFPSTemplate_PartComponent*> AFPSTemplate_PartBase::GetPartComponents()
{
	TArray<UFPSTemplate_PartComponent*> AllPartComponents = PartComponents;
	for (const UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (IsValid(PartComponent) && IsValid(PartComponent->GetPart()))
		{
			AllPartComponents.Append(PartComponent->GetPart()->GetPartComponents());
		}
	}
	return AllPartComponents;
}

void AFPSTemplate_PartBase::PartsUpdated()
{
	if (IsValid(GetOwningActor()) && OwningActor->Implements<UFPSTemplate_AttachmentInterface>())
	{
		IFPSTemplate_AttachmentInterface::Execute_AddPartCache(OwningActor, OwningPartComponent.Get());
		IFPSTemplate_AttachmentInterface::Execute_PartsChanged(OwningActor);
	}
}

void AFPSTemplate_PartBase::SetSnapDistance(float Distance)
{
	OffsetSnapDistance = Distance;

	if (OffsetSnapDistance > 0.0f)
	{
		MaxOffset -= fmod(MaxOffset, OffsetSnapDistance);
		//UE_LOG(LogTemp, Warning, TEXT("MaxOffset: %f"), MaxOffset);
	}
}

void AFPSTemplate_PartBase::SetMinMaxOffset(float Min, float Max)
{
	MinOffset = Min;
	MaxOffset = Max;
}

void AFPSTemplate_PartBase::OnRep_CurrentOffset()
{
	SetActorRelativeLocation(FVector(0.0f, CurrentOffset, 0.0f));
}

bool AFPSTemplate_PartBase::Server_SetOffset_Validate(float Offset)
{
	return true;
}

void AFPSTemplate_PartBase::Server_SetOffset_Implementation(float Offset)
{
	if (bInvertMovingOffset)
	{
		Offset *= -1.0f;
	}
	CurrentOffset = Offset;
	OnRep_CurrentOffset();
}

bool AFPSTemplate_PartBase::AddOffset(float Offset)
{
	if (bInvertMovingOffset)
	{
		Offset *= -1.0f;
	}

	if (OffsetSnapDistance > 0.0f)
	{
		bool bIncrease = false;
		if (Offset > 0.0f)
		{
			bIncrease = true;
		}
		else if (Offset < 0.0f)
		{
			bIncrease = false;
		}
		else
		{
			return false;
		}

		AccumulatedOffset += Offset;
		if (bIncrease && CurrentOffset < MaxOffset && AccumulatedOffset >= OffsetSnapDistance)
		{
			bIncrease = true;
			AccumulatedOffset = 0.0f;
		}
		else if (!bIncrease && CurrentOffset > MinOffset && AccumulatedOffset <= -OffsetSnapDistance)
		{
			bIncrease = false;
			AccumulatedOffset = 0.0f;
		}
		else
		{
			return false;
		}
	
		bIncrease ? CurrentOffset += OffsetSnapDistance : CurrentOffset -= OffsetSnapDistance;
	}
	else
	{
		CurrentOffset += Offset;
	}
	bool MaxedOffset = false;

	if (CurrentOffset > MaxOffset)
	{
		CurrentOffset = MaxOffset;
		MaxedOffset = true;
	}
	if (CurrentOffset < MinOffset)
	{
		CurrentOffset = MinOffset;
		MaxedOffset = true;
	}
	SetActorRelativeLocation(FVector(0.0f, CurrentOffset, 0.0f));
	return MaxedOffset;
}

bool AFPSTemplate_PartBase::SetOffset(float Offset)
{
	if (bInvertMovingOffset)
	{
		Offset *= -1.0f;
	}
	bool MaxedOffset = false;
	if (Offset > MaxOffset || Offset < MinOffset)
	{
		MaxedOffset = true;
	}
	else
	{
		CurrentOffset = Offset;
	}
	SetActorRelativeLocation(FVector(0.0f, CurrentOffset, 0.0f));
	return MaxedOffset;
}

void AFPSTemplate_PartBase::FinishedMovingPart()
{
	AccumulatedOffset = 0.0f;
	OldAccumulatedOffset = 0.0f;
	if (!HasAuthority())
	{
		Server_SetOffset(CurrentOffset);
	}
}

void AFPSTemplate_PartBase::Use_Implementation()
{
	OnUse();
}

TArray<UFPSTemplate_PartComponent*> AFPSTemplate_PartBase::GetSightComponents()
{
	TArray<UFPSTemplate_PartComponent*> SightComponents;
	for (UFPSTemplate_PartComponent* PartComponent : PartComponents)
	{
		if (PartComponent && IsValid(PartComponent->GetPart()))
		{
			if (PartComponent->GetPart()->IsAimable())
			{
				SightComponents.Add(PartComponent);
			}
			SightComponents.Append(PartComponent->GetPart()->GetSightComponents());
		}
	}
	return SightComponents;
}

void AFPSTemplate_PartBase::EnableAiming()
{
	if (!GetAimSocketTransform().GetLocation().Equals(FVector::ZeroVector))
	{
		bIsAimable = true;
	}
}

FTransform AFPSTemplate_PartBase::GetAimSocketTransform()
{
	return PartMesh.IsValid() ? PartMesh->GetSocketTransform(AimSocket) : FTransform();
}
