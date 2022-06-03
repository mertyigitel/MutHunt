//Copyright 2021, Dakota Dawe, All rights reserved

#include "Actors/FPSTemplateCharacter.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Blueprint/UserWidget.h"
#include "Components/FPSTemplate_CharacterComponent.h"
#include "Components/FPS_CharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

AFPSTemplateCharacter::AFPSTemplateCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UFPS_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	GetMesh()->bCastHiddenShadow = true;

	// We will use our own component
	//CharacterComponent = CreateDefaultSubobject<UFPSTemplate_CharacterComponent>(TEXT("CharacterComponent"));
}