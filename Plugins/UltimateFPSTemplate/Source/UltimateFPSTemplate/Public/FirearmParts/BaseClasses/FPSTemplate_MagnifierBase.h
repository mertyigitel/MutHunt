// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "FirearmParts/BaseClasses/FPSTemplate_SightBase.h"
#include "FPSTemplate_MagnifierBase.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_MagnifierBase : public AFPSTemplate_SightBase
{
	GENERATED_BODY()
public:
	AFPSTemplate_MagnifierBase();

protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_FlippedOut, Category = "FPSTemplate | Magnifier")
	bool bFlippedOut;
	UFUNCTION()
	void OnRep_FlippedOut();
	
	bool bFullyFlippedOut;
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Magnifier")
	void SetFullyFlipped(bool bIsFullyFlippedOut) { bFullyFlippedOut = bIsFullyFlippedOut; }

	TWeakObjectPtr<AFPSTemplate_SightBase> SightInfront;

	void SetupMagnifier();

	virtual void BeginPlay() override;
	virtual void OnRep_Owner() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_Flip(bool bFlip);
	
public:
	virtual void Use_Implementation() override;
	AFPSTemplate_SightBase* GetSightInfront() const { return SightInfront.Get(); }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	bool IsFlippedOut() const { return bFlippedOut; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	bool IsFullyFlipped() const { return bFullyFlippedOut; }
};
