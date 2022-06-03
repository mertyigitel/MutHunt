// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_PartBase.h"
#include "FPSTemplate_ForwardGrip.generated.h"

class UAnimSequence;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_ForwardGrip : public AFPSTemplate_PartBase
{
	GENERATED_BODY()
public:
	AFPSTemplate_ForwardGrip();
	
protected:
	// Socket for the Left Hand IK (to position the left hand)
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Sockets")
	FName HandGripSocket;
	// Grip animation/pose to play for the left hand when this grip is in use. This is optional
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	UAnimSequence* GripAnimation;

	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	FTransform GetGripTransform() const;

	UAnimSequence* GetGripAnimation() const { return GripAnimation; }
};