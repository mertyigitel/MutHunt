// Copyright 2022, Dakota Dawe, All rights reserved


#include "AnimGraphNodes/FPSAnimGraphNode_GameplayTagBlend.h"
#include "Misc/FPSTemplateStatics.h"

UFPSAnimGraphNode_GameplayTagBlend::UFPSAnimGraphNode_GameplayTagBlend()
{
	Node.AddPose();
}

void UFPSAnimGraphNode_GameplayTagBlend::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(FFPSAnimNode_GameplayTagBlend, GameplayTagContainer))
	{
		ReconstructNode();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

FText UFPSAnimGraphNode_GameplayTagBlend::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	return FText::FromString("Blend Poses by Gameplay Tag");
}

FText UFPSAnimGraphNode_GameplayTagBlend::GetTooltipText() const
{
	return FText::FromString("Blend Poses by Gameplay Tag");
}

void UFPSAnimGraphNode_GameplayTagBlend::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& PreviousPins)
{
#if WITH_EDITOR
	Node.RefreshPoses();
#endif
	Super::ReallocatePinsDuringReconstruction(PreviousPins);
}

FString UFPSAnimGraphNode_GameplayTagBlend::GetNodeCategory() const
{
	return FString("FPSTemplate");
}

void UFPSAnimGraphNode_GameplayTagBlend::CustomizePinData(UEdGraphPin* Pin, const FName SourcePropertyName, const int32 ArrayIndex) const
{
	bool bBlendPosePin;
	bool bBlendTimePin;
	GetBlendPinProperties(Pin, bBlendPosePin, bBlendTimePin);

	if (!bBlendPosePin && !bBlendTimePin)
	{
		return;
	}

	const FGameplayTag GameplayTag = Node.GetTags().GetByIndex(ArrayIndex - 1);
	const FString BaseName = FString(UFPSTemplateStatics::GetTextAfterPeriod(GameplayTag.ToString()) + ":");
	const FString Index = FString(" " + FString::FromInt(ArrayIndex));
	
	Pin->PinFriendlyName = FText::FromString(BaseName);

	if (bBlendPosePin)
	{
		Pin->PinFriendlyName = FText::FromString(BaseName + Index);
	}
	else if (bBlendTimePin)
	{
		Pin->PinFriendlyName = FText::FromString(BaseName + " Blend" + Index);
	}
}

void UFPSAnimGraphNode_GameplayTagBlend::GetBlendPinProperties(const UEdGraphPin* Pin, bool& bBlendPosePin, bool& bBlendTimePin)
{
	const auto PinFullName{Pin->PinName.ToString()};
	const auto SeparatorIndex{PinFullName.Find(TEXT("_"), ESearchCase::CaseSensitive)};

	if (SeparatorIndex <= 0)
	{
		bBlendPosePin = false;
		bBlendTimePin = false;
		return;
	}

	const auto PinName{PinFullName.Left(SeparatorIndex)};
	bBlendPosePin = PinName == TEXT("BlendPose");
	bBlendTimePin = PinName == TEXT("BlendTime");
}