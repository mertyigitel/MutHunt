// Copyright 2022, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSTemplate_CrosshairWidget.generated.h"

class USizeBox;
class APawn;
class UCanvasPanelSlot;
class UImage;

UCLASS()
class ULTIMATEFPSTEMPLATE_API UFPSTemplate_CrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFPSTemplate_CrosshairWidget(const FObjectInitializer& ObjectInitializer);
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplateCrosshair", Meta = (BindWidget, OptionalWidget = true))
	UImage* I_Circle;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplateCrosshair", Meta = (BindWidget, OptionalWidget = true))
	USizeBox* SB_Top;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplateCrosshair", Meta = (BindWidget, OptionalWidget = true))
	USizeBox* SB_Bottom;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplateCrosshair", Meta = (BindWidget, OptionalWidget = true))
	USizeBox* SB_Left;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplateCrosshair", Meta = (BindWidget, OptionalWidget = true))
	USizeBox* SB_Right;

	UPROPERTY(EditAnywhere, Category = "FPSTemplateCrosshair")
	float TestVelocity;
	UPROPERTY(EditAnywhere, Category = "FPSTemplateCrosshair")
	float MaxVelocity;
	UPROPERTY(EditAnywhere, Category = "FPSTemplateCrosshair")
	FVector2D CircleSize;
	UPROPERTY(EditAnywhere, Category = "FPSTemplateCrosshair")
	float MaxCircleScale;
	UPROPERTY(EditAnywhere, Category = "FPSTemplateCrosshair")
	float MaxOffset;
	UPROPERTY(EditAnywhere, Category = "FPSTemplateCrosshair")
	float UpdatesPerSecond;
	UPROPERTY(EditAnywhere, Category = "FPSTemplateCrosshair")
	bool UpdatesFromOutsideSource;
	UPROPERTY(EditAnywhere, Category = "FPSTemplateCrosshair")
	float InterpolationRate;

	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplateCrosshair", Meta = (ExposeOnSpawn))
	APawn* OwningPawn;

	FTimerHandle TCrosshair;
	float CrosshairScale;
	float CircleScale;

	TWeakObjectPtr<UCanvasPanelSlot> CPS_Circle;
	TWeakObjectPtr<UCanvasPanelSlot> CPS_Top;
	TWeakObjectPtr<UCanvasPanelSlot> CPS_Bottom;
	TWeakObjectPtr<UCanvasPanelSlot> CPS_Left;
	TWeakObjectPtr<UCanvasPanelSlot> CPS_Right;

	virtual void SynchronizeProperties() override;
	virtual void NativeConstruct() override;
	void SetCanvasPanelSlots();

	UCanvasPanelSlot* GetCanvasPanelSlot(UWidget* Widget) const;
	void HandleCrosshairOffset();

public:
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Crosshair")
	void HandleCrosshairOffset(float Velocity);
};
