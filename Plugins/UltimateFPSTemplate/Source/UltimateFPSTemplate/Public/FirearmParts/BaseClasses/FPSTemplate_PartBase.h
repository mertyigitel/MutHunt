// Copyright 2021, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSTemplateDataTypes.h"
//#include "Components/FPSTemplate_CharacterComponent.h"
#include "Components/FPSTemplate_PartComponent.h"
//#include "Actors/FPSTemplateFirearm.h"
#include "Components/MeshComponent.h"
#include "FPSTemplate_PartBase.generated.h"

class UMeshComponent;
class UFPSTemplate_PartComponent;
class UFPSTemplate_CharacterComponent;

#define MAX_PartStack 10

UCLASS()
class ULTIMATEFPSTEMPLATE_API AFPSTemplate_PartBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSTemplate_PartBase();

protected:
	TWeakObjectPtr<UMeshComponent> PartMesh;
	virtual void SetupPartMesh();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	FFirearmPartStats PartStats;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	FFirearmPartData PartData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default")
	EPartType PartType;
	// Controls the length of pull which affects how close/far you are when aiming to your optic
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPSTemplate | Default", meta = (EditCondition = "PartType == EPartType::Stock"))
	float StockLengthOfPull;
	// Used for the stock (example) which has a negative minimum offset because it slides backwards in reverse
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Default")
	bool bInvertMovingOffset;
	// Trace channel to use for firearm collision, Highly recommended you create your own in your project.
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Default")
	TEnumAsByte<ECollisionChannel> FirearmCollisionChannel;
	// If true this part will be aimable if the part mesh has a valid Aim Socket
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Aim")
	bool bIsAimable;
	// Socket that is used for aiming such as S_Aim that is on optics and the lightlaser mesh
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Aim")
	FName AimSocket;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "FPSTemplate | Parts")
	TArray<UFPSTemplate_PartComponent*> PartComponents;

	TWeakObjectPtr<UFPSTemplate_PartComponent> OwningPartComponent;
	AActor* OwningActor;
	UFPSTemplate_CharacterComponent* OwningCharacterComponent;

	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Customization")
	float OffsetSnapDistance;

	float AccumulatedOffset;
	float OldAccumulatedOffset;
	float MinOffset;
	float MaxOffset;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentOffset)
	float CurrentOffset;
	UFUNCTION()
	void OnRep_CurrentOffset();
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetOffset(float Offset);
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | AssetID")
	FPrimaryAssetType AssetType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | AssetID")
	FName AssetName;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "FPSTemplate | Events")
	void OnUse();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Use();
	virtual void Server_Use_Implementation();

	bool bHasRenderTarget;

	void CacheCharacterAndFirearm();
	
public:
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Stats")
	virtual FFirearmPartStats GetPartStats();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Stats")
	FFirearmPartData GetPartData() const { return PartData; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	EPartType GetPartType() const { return PartType; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	AActor* GetOwningActor();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	UFPSTemplate_CharacterComponent* GetOwningCharacterComponent();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	UMeshComponent* GetPartMesh() const { return PartMesh.Get(); }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	UFPSTemplate_PartComponent* GetOwningPartComponent() const { return OwningPartComponent.Get(); }
	
	TArray<UFPSTemplate_PartComponent*> GetPartComponents();

	void PartsUpdated();
	virtual void CacheParts() {}

	void SetMinMaxOffset(float Min, float Max);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default")
	void SetSnapDistance(float Distance);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default")
	bool AddOffset(float Offset);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default")
	bool SetOffset(float Offset);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Default")
	void FinishedMovingPart();
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	float GetPartOffset() const { return CurrentOffset; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Default")
	bool IsMovementInverted() const { return bInvertMovingOffset; }

	UFUNCTION(BlueprintNativeEvent, Category = "FPSTemplate | Events")
	void OnPartRemoved(class UFPSTemplate_PartComponent* PartComponent);

	UFUNCTION(BlueprintNativeEvent, Category = "FPSTemplate | Use")
	void Use();
	virtual void Use_Implementation();
	
	TArray<UFPSTemplate_PartComponent*> GetSightComponents();

	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Aiming")
	bool IsAimable() const { return bIsAimable; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	void DisableAiming() { bIsAimable = false;}
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	void EnableAiming();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	virtual FTransform GetAimSocketTransform();
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Aiming")
	float GetStockLengthOfPull() const { return StockLengthOfPull; }

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Render")
	bool HasRenderTarget() const { return bHasRenderTarget; }
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Render")
	virtual void DisableRenderTarget(bool Disable) {}

	void SetOwningPartComponent(UFPSTemplate_PartComponent* PartComponent) { OwningPartComponent = PartComponent; }
};
