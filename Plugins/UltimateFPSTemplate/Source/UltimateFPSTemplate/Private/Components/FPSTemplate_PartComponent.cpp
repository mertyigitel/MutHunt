//Copyright 2021, Dakota Dawe, All rights reserved

#include "Components/FPSTemplate_PartComponent.h"

#include "Actors/FPSTemplateFirearm.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"

#include "Components/StaticMeshComponent.h"
#include "Misc/FPSTemplateStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UFPSTemplate_PartComponent::UFPSTemplate_PartComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	Minimum = 0.0f;
	Maximum = 0.0f;

	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(GetAttachmentRoot());
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMesh->SetHiddenInGame(true);
	PartTransformReference = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	PartTransformReference->SetupAttachment(PreviewMesh);
	
	SetIsReplicatedByDefault(true);
}

void UFPSTemplate_PartComponent::OnRep_Part() const
{
	if (Part)
	{
		Part->SetMinMaxOffset(Minimum, Maximum);
		Part->PartsUpdated();
	}
}

// Called when the game starts
void UFPSTemplate_PartComponent::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(PreviewMesh))
	{
		//PreviewMesh->DestroyComponent(true);
	}
	if (IsValid(DefaultPart) && GetOwner() && GetOwner()->HasAuthority())
	{
		AddPart(DefaultPart);
	}
}

void UFPSTemplate_PartComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFPSTemplate_PartComponent, Part);
}

FTransform UFPSTemplate_PartComponent::GetAttachmentTransform() const
{
	return PartTransformReference->GetComponentTransform();
}

bool UFPSTemplate_PartComponent::Server_AddPart_Validate(TSubclassOf<AFPSTemplate_PartBase> PartClass)
{
	return true;
}


void UFPSTemplate_PartComponent::Server_AddPart_Implementation(TSubclassOf<AFPSTemplate_PartBase> PartClass)
{
	AddPart(PartClass);
}

bool UFPSTemplate_PartComponent::AddPart(TSubclassOf<AFPSTemplate_PartBase> PartClass)
{
	if (!IsPartCompatible(PartClass) || !GetOwner())
	{
		return false;
	}

	if (!HasAuthority())
	{
		Server_AddPart(PartClass);
	}
	else
	{
		if (PartClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();

			AFPSTemplate_PartBase* SpawnedPart = GetWorld()->SpawnActor<AFPSTemplate_PartBase>(PartClass, SpawnParams);
			if (SpawnedPart)
			{
				float PartOffset = 0.0f;
				if (IsValid(Part))
				{
					PartOffset = Part->GetPartOffset();
					for (UFPSTemplate_PartComponent* PartComponent : Part->GetPartComponents())
					{
						if (PartComponent && PartComponent->GetPart())
						{
							PartComponent->GetPart()->Destroy();
						}
					}
					Part->OnPartRemoved(this);
				}
				SpawnedPart->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				SpawnedPart->SetActorTransform(GetAttachmentTransform());
				Part = SpawnedPart;
				Part->SetMinMaxOffset(Minimum, Maximum);
				Part->SetOwningPartComponent(this);
				Part->SetOffset(PartOffset);
				Part->PartsUpdated();
			}
		}
		else if (IsValid(Part)) // Remove the part and dont add a new one
		{
			for (UFPSTemplate_PartComponent* PartComponent : Part->GetPartComponents())
			{
				if (PartComponent && IsValid(PartComponent->GetPart()))
				{
					PartComponent->GetPart()->Destroy();
				}
			}
			if (AFPSTemplateFirearm* Firearm = Part->GetOwningFirearm())
			{
				Part->Destroy();
				Firearm->PartsChanged();
			}
		}
	}
	return true;
}

bool UFPSTemplate_PartComponent::IsPartCompatible(TSubclassOf<AFPSTemplate_PartBase> PartClass) const
{
	if (!PartClass)
	{
		return true;
	}
	
	for (TSubclassOf<AFPSTemplate_PartBase> PossiblePart : PossibleParts)
	{
		if (PossiblePart && PossiblePart == PartClass)
		{
			return true;
		}
	}
	return false;
}
