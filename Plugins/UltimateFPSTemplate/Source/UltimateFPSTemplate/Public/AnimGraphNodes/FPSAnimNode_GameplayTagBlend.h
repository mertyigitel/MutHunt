// Copyright 2022, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AnimNodes/AnimNode_BlendListBase.h"
#include "FPSAnimNode_GameplayTagBlend.generated.h"

/**
 * 
 */
USTRUCT()
struct ULTIMATEFPSTEMPLATE_API FFPSAnimNode_GameplayTagBlend : public FAnimNode_BlendListBase
{
	GENERATED_BODY()

public:
	FFPSAnimNode_GameplayTagBlend();
	
	UPROPERTY(EditAnywhere, Category = "Settings", Meta = (FoldProperty))
	FGameplayTagContainer GameplayTagContainer;
	
	UPROPERTY(EditAnywhere, Category = "Settings", Meta = (FoldProperty, PinShownByDefault))
	FGameplayTag ActiveGameplayTag;

protected:
	virtual int32 GetActiveChildIndex() override;

public:
	const FGameplayTag& GetActiveTag() const;
	const FGameplayTagContainer& GetTags() const;

#if WITH_EDITOR
	void RefreshPoses();
#endif
};
