//Copyright 2021, Dakota Dawe, All rights reserved
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "FPSTemplate_PartComponent.generated.h"

class AFPSTemplate_PartBase;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class UFPSTemplate_PreviewStatic;
class UFPSTemplate_PreviewSkeletal;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTIMATEFPSTEMPLATE_API UFPSTemplate_PartComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFPSTemplate_PartComponent();

protected:
	// All the possible attachments this component can have. Is used for part compatibility and customization
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Customization")
	TArray<TSubclassOf<AFPSTemplate_PartBase>> PossibleParts;
	// If this is set this part will be spawned by default and attached to the firearm/attachment
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Customization")
	TSubclassOf<AFPSTemplate_PartBase> DefaultPart;
	// This is used for whatever, I use it for the firearm customizer. Use as you wish
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Customization")
	FText ComponentName;
	// The minimum distance you can move the part towards your camera (Up the rail). Recommended leave at 0 except for specific cases (look at the stock on the M4)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Customization")
	float Minimum;
	// The maximum distance you can move the part away from the camera (Down the rail)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Customization")
	float Maximum;
	// This will place the part upon its first spawning at the location in the editor (Display Part Min Max)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Customization")
	bool bSetPartInitialOffsetAtDisplayMinMax;
	// Snap distance. Picatinny rail = 1.0, M-LOK = 4.0, Keymod = 2.0, No Snap = 0.0
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPSTemplate | Customization")
	float OffsetSnapDistance;
	UPROPERTY(BlueprintReadOnly, Category = "FPSTemplate | Customization")
	USceneComponent* PartTransformReference;
	
	// Slide the part up/down the rail so you can visualize and figure out the Minimum and Maximum values
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Preview")
	float DisplayPartMinMax;
	// Sets the preview part to be the mesh of the index of Possible Parts (0 = first part in Possible Parts)
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Preview")
	int32 DisplayPartIndex;
#if WITH_EDITORONLY_DATA
	// Whether or not to show the Minimum/Maximum lines for preview
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Preview")
	bool bShowMinMaxLine;
	// MinMax Line Thickness
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Preview")
	float LineThickness;
	// The height offset of the line for better visuals
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Preview")
	float HeightOffset;
#endif
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Preview")
	UFPSTemplate_PreviewStatic* PreviewStatic;
	UPROPERTY(EditDefaultsOnly, Category = "FPSTemplate | Preview")
	UFPSTemplate_PreviewSkeletal* PreviewSkeletal;

	UPROPERTY(ReplicatedUsing = OnRep_Part, BlueprintReadOnly, Category = "FPSTemplate")
	AFPSTemplate_PartBase* Part;
	UFUNCTION()
	void OnRep_Part() const;

	FTransform CurrentPartTransform;
	float OldDisplayMinMax;
	bool bSetInitialOffset;
	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	UActorComponent* GetMeshComponentFromPart() const;

	virtual void PostInitProperties() override;
	bool bIsLive;
	FTimerHandle EditorDebugHandle;
	void DrawDebugEditor();
#endif
	
	bool HasAuthority() const { return IsValid(GetOwner()) ? GetOwner()->HasAuthority() : false; }

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AddPart(TSubclassOf<AFPSTemplate_PartBase> PartClass);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AddExistingPart(AFPSTemplate_PartBase* INPart);

	void HandlePartConstruction();
	
public:
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	FTransform GetAttachmentTransform() const;
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	AFPSTemplate_PartBase* GetPart() const { return Part; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	TArray<TSubclassOf<AFPSTemplate_PartBase>>& GetPossibleParts() { return PossibleParts; }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	FVector2D GetMinMaxOffset() const { return FVector2D(Minimum, Maximum); }
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Part")
	FVector GetMaxOffsetLocation();
	
	template <class T>
	T* GetPart() const { return Cast<T>(Part); }

	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	bool AddPart(TSubclassOf<AFPSTemplate_PartBase> PartClass);
	UFUNCTION(BlueprintCallable, Category = "FPSTemplate | Part")
	bool AddExistingPart(AFPSTemplate_PartBase* INPart);
	UFUNCTION(BlueprintPure, Category = "FPSTemplate | Animation")
	bool IsPartCompatible(TSubclassOf<AFPSTemplate_PartBase> PartClass) const;
};
