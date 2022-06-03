// Copyright 2022, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FPSTemplate_AttachmentInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFPSTemplate_AttachmentInterface : public UInterface
{
	GENERATED_BODY()
};

class UFPSTemplate_PartComponent;

class ULTIMATEFPSTEMPLATE_API IFPSTemplate_AttachmentInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AttachmentInterface")
	void PartsChanged();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AttachmentInterface")
	void AddPartCache(UFPSTemplate_PartComponent* PartComponent);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FPSTemplate | AttachmentInterface")
	void SetSight(UFPSTemplate_PartComponent* SightComponent);
};
