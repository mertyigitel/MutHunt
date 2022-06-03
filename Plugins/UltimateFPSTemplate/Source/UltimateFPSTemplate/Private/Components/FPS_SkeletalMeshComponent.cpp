// Copyright 2022, Dakota Dawe, All rights reserved


#include "Components/FPS_SkeletalMeshComponent.h"
#include "Misc/FPSTemplateStatics.h"

UFPS_SkeletalMeshComponent::UFPS_SkeletalMeshComponent()
{
	
}

void UFPS_SkeletalMeshComponent::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerController = GetWorld()->GetFirstPlayerController();
}

FMatrix UFPS_SkeletalMeshComponent::GetRenderMatrix() const
{
	FMatrix Matrix = Super::GetRenderMatrix();
	if (!PlayerController || !PlayerController->GetPawn())
	{
		return Matrix;
	}
	
	if (GetOwner())
	{
		AActor* OwningActor = GetOwner();
		for (uint8 i = 0; i < MaxOwnerAttempts; ++i)
		{
			if (APawn* OwningPawn = Cast<APawn>(OwningActor))
			{
				OwningActor = OwningPawn;
				break;
			}
			
			if (OwningActor)
			{
				OwningActor = OwningActor->GetOwner();
			}
			else
			{
				return Matrix;
			}
		}
		if (PlayerController->GetPawn() != OwningActor)
		{
			return Matrix;
		}
	}

	UFPSTemplateStatics::CalculateAdjustedMatrix(this, PlayerController, DesiredFOV, Matrix);

	return Matrix;
}