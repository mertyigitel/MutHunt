//Copyright 2021, Dakota Dawe, All rights reserved

#include "Components/FPSTemplate_PartComponent.h"
#include "Components/FPSTemplate_PreviewStatic.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Components/FPSTemplate_PreviewSkeletal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "Misc/FPSTemplateStatics.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Net/UnrealNetwork.h"

#define COMPONENT_TAG_NAME FName("FPSPart")

// Sets default values for this component's properties
UFPSTemplate_PartComponent::UFPSTemplate_PartComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Minimum = 0.0f;
	Maximum = 0.0f;
	DisplayPartIndex = INDEX_NONE;
	DisplayPartMinMax = 0.0f;
	bSetPartInitialOffsetAtDisplayMinMax = false;
	bSetInitialOffset = false;
	OldDisplayMinMax = 0.0f;
	OffsetSnapDistance = 0.0f;

	PreviewStatic = CreateDefaultSubobject<UFPSTemplate_PreviewStatic>(TEXT("PreviewStaticMesh"));
	PreviewStatic->SetupAttachment(GetAttachmentRoot());
	PreviewStatic->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewStatic->SetHiddenInGame(true);
	PreviewStatic->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	
	PartTransformReference = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	PartTransformReference->SetupAttachment(PreviewStatic);
	
	PreviewSkeletal = CreateDefaultSubobject<UFPSTemplate_PreviewSkeletal>(TEXT("PreviewSkeletalMesh"));
	PreviewSkeletal->SetupAttachment(PreviewStatic);
	PreviewSkeletal->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewSkeletal->SetHiddenInGame(true);
	PreviewSkeletal->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	
	SetIsReplicatedByDefault(true);

#if WITH_EDITORONLY_DATA
	bShowMinMaxLine = false;
	LineThickness = 0.2f;
	HeightOffset = 0.0f;
#endif
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
	PreviewSkeletal->DestroyComponent();
	PreviewStatic->DestroyComponent();
	
	HandlePartConstruction();

#if WITH_EDITOR
	bIsLive = true;
	if (EditorDebugHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(EditorDebugHandle);
	}
#endif
}

void UFPSTemplate_PartComponent::HandlePartConstruction()
{
	AActor* CurrentOwner = GetOwner();
	if (CurrentOwner && CurrentOwner->HasAuthority())
	{
		bool bSpawnDefaultParts = true;
		if (const AFPSTemplateFirearm* Firearm = Cast<AFPSTemplateFirearm>(CurrentOwner))
		{
			bSpawnDefaultParts = Firearm->GetShouldSpawnDefaultOnPreset();
		}
		else
		{
			for (uint8 i = 0; i < 10; ++i)
			{
				CurrentOwner = CurrentOwner->GetOwner();
				if (const AFPSTemplateFirearm* OwningFirearm = Cast<AFPSTemplateFirearm>(CurrentOwner))
				{
					bSpawnDefaultParts = OwningFirearm->GetShouldSpawnDefaultOnPreset();
					break;
				}
			}
		}
		
		//UE_LOG(LogTemp, Warning, TEXT("Spawn DefaultParts %s   bSpawnDefaultParts: %d"), *GetName(), bSpawnDefaultParts);
		if (bSpawnDefaultParts && IsValid(DefaultPart))
		{
			AddPart(DefaultPart);
		}
	}
}

void UFPSTemplate_PartComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFPSTemplate_PartComponent, Part);
}

#if WITH_EDITOR
void UFPSTemplate_PartComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FString PropertyName = PropertyChangedEvent.GetPropertyName().ToString();
	
	if (PropertyName.Equals(TEXT("DisplayPartMinMax")))
	{
		if (DisplayPartMinMax - OldDisplayMinMax >= OffsetSnapDistance || DisplayPartMinMax - OldDisplayMinMax <= -OffsetSnapDistance)
		{
			OldDisplayMinMax = DisplayPartMinMax;
			PreviewStatic->SetRelativeLocation(FVector(0.0f, DisplayPartMinMax, 0.0f));
		}
	}
	else if (!PropertyName.Contains(TEXT("Preview")))
	{
		if (UActorComponent* ActorComponent = GetMeshComponentFromPart())
		{
			if (const UStaticMeshComponent* PartStaticMesh = Cast<UStaticMeshComponent>(ActorComponent))
			{
				PreviewStatic->SetStaticMesh(PartStaticMesh->GetStaticMesh());
				PreviewStatic->SetHiddenInGame(true);
				PreviewSkeletal->SetSkeletalMesh(nullptr);
			}
			else if (const USkeletalMeshComponent* PartSkeletalMesh = Cast<USkeletalMeshComponent>(ActorComponent))
			{
				PreviewSkeletal->SetSkeletalMesh(PartSkeletalMesh->SkeletalMesh);
				PreviewSkeletal->SetHiddenInGame(true);
				PreviewStatic->SetStaticMesh(nullptr);
			}
		}
	}
	else if (PropertyName.Contains(TEXT("Preview")))
	{
		DisplayPartIndex = INDEX_NONE;
		if (PropertyName.Contains(TEXT("Static")))
		{
			if (PreviewStatic)
			{
				PreviewSkeletal->SetSkeletalMesh(nullptr);
			}
		}
		else if (PropertyName.Contains(TEXT("Skeletal")))
		{
			if (PreviewSkeletal)
			{
				PreviewStatic->SetStaticMesh(nullptr);
			}
		}
	}
}

void UFPSTemplate_PartComponent::PostInitProperties()
{
	Super::PostInitProperties();
	if (!bIsLive && GetWorld() && !EditorDebugHandle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(EditorDebugHandle, this, &UFPSTemplate_PartComponent::DrawDebugEditor, 0.01f, true);
	}
}

void UFPSTemplate_PartComponent::DrawDebugEditor()
{
	if (GetWorld())
	{
		if (bIsLive || !bShowMinMaxLine)
		{
			GetWorld()->GetTimerManager().ClearTimer(EditorDebugHandle);
		}
		else
		{
			if (Maximum != 0.0f || Minimum != 0.0f)
			{
				bool bInverted = false;
				if (Minimum < 0.0f)
				{
					bInverted = true;
				}
				FVector Start = GetComponentLocation();
				FVector End;
				if (bInverted)
				{
					End = Start + GetRightVector() * Minimum;
				}
				else
				{
					End = Start + GetRightVector() * Maximum;
				}
		
				Start.Z += HeightOffset;
				End.Z += HeightOffset;

				DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.01f, 0, LineThickness);
			}
		}
	}
}

UActorComponent* UFPSTemplate_PartComponent::GetMeshComponentFromPart() const
{
	if (DisplayPartIndex > -1 && DisplayPartIndex < PossibleParts.Num())
	{
		if (const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(PossibleParts[DisplayPartIndex]))
		{
			const TArray<USCS_Node*>& ActorBlueprintNodes = BPClass->SimpleConstructionScript->GetAllNodes();
		
			for (const USCS_Node* Node : ActorBlueprintNodes)
			{
				if (Node && Node->ComponentTemplate)
				{
					if (Node->ComponentClass == UStaticMeshComponent::StaticClass() || Node->ComponentClass == USkeletalMeshComponent::StaticClass())
					{
						if (Node->ComponentTemplate->ComponentHasTag(COMPONENT_TAG_NAME))
						{
							return Node->ComponentTemplate;
						}
					}
				}
			}
		}
	}
	return nullptr;
}
#endif

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

FVector UFPSTemplate_PartComponent::GetMaxOffsetLocation()
{
	FVector EndLocation = GetComponentLocation() + GetComponentRotation().Vector() * Maximum;
	if (IsValid(GetPart()))
	{
		const float Distance = GetPart()->GetPartOffset();
		if (GetPart()->IsMovementInverted())
		{
			EndLocation = GetComponentLocation() + GetPart()->GetActorRightVector() * (Minimum);
		}
		else
		{
			EndLocation = GetPart()->GetActorLocation() + GetPart()->GetActorRightVector() * (Maximum - Distance);
		}
		//DrawDebugSphere(GetWorld(), EndLocation, 2.0f, 14.0f, FColor::Red);
	}
	return EndLocation;
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
					for (const UFPSTemplate_PartComponent* PartComponent : Part->GetPartComponents())
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

				if (!bSetInitialOffset && bSetPartInitialOffsetAtDisplayMinMax && DisplayPartMinMax <= Maximum && DisplayPartMinMax >= Minimum)
				{
					bSetInitialOffset = true;
					Part->SetOffset(DisplayPartMinMax);
				}
				else
				{
					Part->SetOffset(PartOffset);
				}
				Part->SetSnapDistance(OffsetSnapDistance);
				Part->PartsUpdated();
			}
		}
		else if (IsValid(Part)) // Remove the part and dont add a new one
		{
			for (const UFPSTemplate_PartComponent* PartComponent : Part->GetPartComponents())
			{
				if (PartComponent && IsValid(PartComponent->GetPart()))
				{
					PartComponent->GetPart()->Destroy();
				}
			}
			AActor* OwningActor = Part->GetOwningActor();
			if (IsValid(OwningActor) && OwningActor->Implements<UFPSTemplate_AttachmentInterface>())
			{
				Part->Destroy();
				IFPSTemplate_AttachmentInterface::Execute_PartsChanged(OwningActor);
			}
		}
	}
	return true;
}

bool UFPSTemplate_PartComponent::Server_AddExistingPart_Validate(AFPSTemplate_PartBase* INPart)
{
	return true;
}

void UFPSTemplate_PartComponent::Server_AddExistingPart_Implementation(AFPSTemplate_PartBase* INPart)
{
	AddExistingPart(INPart);
}

bool UFPSTemplate_PartComponent::AddExistingPart(AFPSTemplate_PartBase* INPart)
{
	if (!IsPartCompatible(INPart->GetClass()) || !GetOwner())
	{
		return false;
	}
	
	if (!HasAuthority())
	{
		Server_AddExistingPart(INPart);
	}
	else
	{
		if (INPart)
		{
			float PartOffset = 0.0f;
			if (IsValid(Part))
			{
				PartOffset = Part->GetPartOffset();
				for (const UFPSTemplate_PartComponent* PartComponent : Part->GetPartComponents())
				{
					if (PartComponent && PartComponent->GetPart())
					{
						PartComponent->GetPart()->Destroy();
					}
				}
				Part->OnPartRemoved(this);
			}
			INPart->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			INPart->SetActorTransform(GetAttachmentTransform());
			Part = INPart;
			Part->SetMinMaxOffset(Minimum, Maximum);
			Part->SetOwningPartComponent(this);
			if (!bSetInitialOffset && bSetPartInitialOffsetAtDisplayMinMax && DisplayPartMinMax <= Maximum && DisplayPartMinMax >= Minimum)
			{
				bSetInitialOffset = true;
				Part->SetOffset(DisplayPartMinMax);
			}
			else
			{
				Part->SetOffset(PartOffset);
			}
			Part->SetSnapDistance(OffsetSnapDistance);
			Part->PartsUpdated();
		}
		else if (IsValid(Part)) // Remove the part and dont add a new one
		{
			for (const UFPSTemplate_PartComponent* PartComponent : Part->GetPartComponents())
			{
				if (PartComponent && IsValid(PartComponent->GetPart()))
				{
					PartComponent->GetPart()->Destroy();
				}
			}
			AActor* OwningActor = Part->GetOwningActor();
			if (IsValid(OwningActor) && OwningActor->Implements<UFPSTemplate_AttachmentInterface>())
			{
				Part->Destroy();
				IFPSTemplate_AttachmentInterface::Execute_PartsChanged(OwningActor);
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
