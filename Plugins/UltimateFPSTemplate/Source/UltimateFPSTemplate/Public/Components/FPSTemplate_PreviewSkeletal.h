// Copyright 2022, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "FPSTemplate_PreviewSkeletal.generated.h"


UCLASS(Blueprintable, ClassGroup=(Rendering, Common), hidecategories=(Object,Activation,Rendering,LOD,RayTracing,TextureStreaming,Lighting,Collision,MaterialParameters,HLOD,Mobile,Physics,Navigation,VirtualTexture,Custom,Transform,Mobility,ComponentTick,Tags,AssetUserData,ComponentReplication,Variable,Cooking,Animation,Clothing,MasterPoseComponent,Optimization,ClothingSimulation,SkeletalMesh), editinlinenew, meta=(BlueprintSpawnableComponent))
class ULTIMATEFPSTEMPLATE_API UFPSTemplate_PreviewSkeletal : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFPSTemplate_PreviewSkeletal();
};
