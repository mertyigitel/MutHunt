// Copyright 2022, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "FPS_StaticMeshComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup=(Rendering, Common), hidecategories=(Object,Activation,"Components|Activation"), ShowCategories=(Mobility), editinlinenew, meta=(BlueprintSpawnableComponent))
class ULTIMATEFPSTEMPLATE_API UFPS_StaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
public:
	UFPS_StaticMeshComponent();

protected:
	uint8 MaxOwnerAttempts = 10;
	UPROPERTY(EditDefaultsOnly, Category = "TESTING")
	float DesiredFOV = 90.0f;
	UPROPERTY()
	APlayerController* PlayerController;

	virtual void BeginPlay() override;
	virtual FMatrix GetRenderMatrix() const override;
};
