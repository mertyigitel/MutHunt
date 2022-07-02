// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/FPSTemplateProjectile.h"
#include "Projectile.generated.h"

UCLASS()
class MUTHUNT_API AProjectile : public AFPSTemplateProjectile
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

private:
	
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

public:	
	

};
