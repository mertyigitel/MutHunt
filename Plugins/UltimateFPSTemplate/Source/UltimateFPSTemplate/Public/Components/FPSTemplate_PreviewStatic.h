// Copyright 2022, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "FPSTemplate_PreviewStatic.generated.h"


UCLASS(Blueprintable, ClassGroup=(Rendering, Common), hidecategories=(Object,Activation,Rendering,LOD,RayTracing,TextureStreaming,Lighting,Collision,MaterialParameters,HLOD,Mobile,Physics,Navigation,VirtualTexture,Custom,Transform,Mobility,ComponentTick,Tags,AssetUserData,ComponentReplication,Variable,Cooking), editinlinenew, meta=(BlueprintSpawnableComponent))
class ULTIMATEFPSTEMPLATE_API UFPSTemplate_PreviewStatic : public UStaticMeshComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFPSTemplate_PreviewStatic();
};
