// Copyright 2022, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "FPS_SkeletalMeshComponent.generated.h"

struct FMatrices
{
	FMatrix ViewMatrix;
	FMatrix InverseViewMatrix;
	FMatrix ProjectionMatrix;
	FMatrix InverseProjectionMatrix;
	FMatrix InverseViewProjectionMatrix;
	float NearClippingPlaneDistance;
};

UCLASS(Blueprintable, ClassGroup=(Rendering, Common), hidecategories=Object, config=Engine, editinlinenew, meta=(BlueprintSpawnableComponent))
class ULTIMATEFPSTEMPLATE_API UFPS_SkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:
	UFPS_SkeletalMeshComponent();

protected:
	uint8 MaxOwnerAttempts = 10;
	UPROPERTY(EditDefaultsOnly, Category = "TESTING")
	float DesiredFOV = 90.0f;
	UPROPERTY()
	APlayerController* PlayerController;
	
	virtual void BeginPlay() override;
	virtual FMatrix GetRenderMatrix() const override;
};
