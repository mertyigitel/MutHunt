//Copyright 2021, Dakota Dawe, All rights reserved
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSTemplateCharacter.generated.h"

class UInputComponent;
class UFPSTemplate_CharacterComponent;

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplateCharacter : public ACharacter
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UFPSTemplate_CharacterComponent* CharacterComponent;

public:
	AFPSTemplateCharacter(const FObjectInitializer& ObjectInitializer);
};
