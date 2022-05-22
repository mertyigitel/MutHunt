//Copyright 2021, Dakota Dawe, All rights reserved

// ASSET LEAK FROM https://online-leaks.com

#include "FPSTemplateAnimInstance.h"

#include "Actors/FPSTemplateFirearm.h"
#include "FirearmParts/BaseClasses/FPSTemplate_SightBase.h"
#include "Components/FPSTemplate_CharacterComponent.h"

#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Curves/CurveVector.h"
#include "GameFramework/PawnMovementComponent.h"

UFPSTemplateAnimInstance::UFPSTemplateAnimInstance()
{
	AimInterpolationSpeed = 20.0f;
	CycleSightsInterpolationSpeed = 20.0f;
	RotationLagResetInterpolationSpeed = 20.0f;
	MotionLagResetInterpolationSpeed = 20.0f;

	bUseProceduralSpine = true;
	bUseLeftHandIK = true;
	SpineBoneCount = 1;
	
	bInterpRelativeToHand = false;
	bFirstRun = true;
	bIsAiming = false;
	AimingAlpha = 0.0f;
	LeftHandIKAlpha = 1.0f;
	RotationAlpha = false;
	bInterpAiming = false;
	RightHandBone = FName("hand_r");

	CurrentLean = ELeaning::None;

	HeadAimingRotation = FRotator(45.0f, 0.0f, 0.0f);

	SightDistance = 0.0f;
	CurveTimer = 1.0f;
	bCustomizingFirearm = false;
	bCanAim = true;
	bInterpPortPose = false;
	ShortStockPoseAlpha = 0.0f;
	bValidLeftHandPose = false;
	bInvertRotationLag = false;
	bSprinting = false;

	MakeLeftHandFollowAlpha = 1.0f;

	SwayMultiplier = 1.0f;

	bIsLocallyControlled = false;

	FreeLookRotation = FRotator(90.0f, 0.0f, 0.0f);

	bInterpRecoil = false;
	bInterpShakeCurve = false;
	bCanPlayShakeCurve = true;
	
	ShakeCurveAlpha = 0.0f;

	CustomPoseAlpha = 0.0f;
}

void UFPSTemplateAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	
	if (const AActor* OwningActor = GetOwningActor())
	{
		CharacterComponent = Cast<UFPSTemplate_CharacterComponent>(OwningActor->GetComponentByClass(TSubclassOf<UFPSTemplate_CharacterComponent>()));
		if (CharacterComponent)
		{
			bIsLocallyControlled = CharacterComponent->IsLocallyControlled();
		}
	}
}

void UFPSTemplateAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!CharacterComponent || bCustomizingFirearm) { return; }

	FVector Velocity = CharacterComponent->GetMovementComponent()->Velocity;
	Velocity.Z = 0.0f;
	CharacterVelocity = Velocity.Size();
	//CharacterDirection = CalculateDirection(Velocity, CharacterComponent->GetOwner()->GetActorRotation());
	
	IsLocallyControlled();
	AimingActor = CharacterComponent->GetAimingActor();

	if (AimingActor)
	{
		Firearm = Cast<AFPSTemplateFirearm>(AimingActor);
		if (Firearm)
		{
			BasePoseOffsetLocation = Firearm->GetBasePoseOffset().GetLocation();
			BasePoseOffsetRotation = Firearm->GetBasePoseOffset().Rotator();

			if (bUseLeftHandIK)
			{	// Handles Left Hand IK and Pose
				LeftHandPose = Firearm->GetGripAnimation();
				if (LeftHandPose)
				{
					bValidLeftHandPose = true;
				}
				else
				{
					bValidLeftHandPose = false;
				}
			
				if (Firearm->UseLeftHandIK())
				{
					SetLeftHandIK();
				}
				else
				{
					LeftHandIKAlpha = 0.0f;
				}
			}
		}
		else
		{
			BasePoseOffsetLocation = FVector::ZeroVector;
			BasePoseOffsetRotation = FRotator::ZeroRotator;
		}

		if (PortPose == EPortPose::None)
		{
			SetRelativeToHand();
			InterpRelativeToHand(DeltaSeconds);
		}
		
		if (bInterpAiming)
		{
			InterpAimingAlpha(DeltaSeconds);
		}
		
		if (CharacterComponent->IsLocallyControlled())
		{
			if (bInterpCameraZoom)
			{
				InterpCameraZoom(DeltaSeconds);
			}
			SetRotationLag(DeltaSeconds);
			if (CharacterComponent->GetMovementComponent()->Velocity.Size() || !MovementLagRotation.Equals(FRotator::ZeroRotator))
			{
				SetMovementLag(DeltaSeconds);
			}

			HandleMovementSway(DeltaSeconds);
		}

		HandleSprinting();

		if (bInterpRecoil)
		{
			RecoilInterpTo(DeltaSeconds);
		}
		
		if (bInterpPortPose)
		{
			InterpPortPose(DeltaSeconds);
		}

		if (bInterpShakeCurve)
		{
			InterpShakeCurve(DeltaSeconds);
		}
	}
	else
	{
		LeftHandIKAlpha = 0.0f;
	}
	
	if (bUseProceduralSpine && !bFreeLook)
	{
		if (!CharacterComponent->IsLocallyControlled())
		{
			SpineToInterpTo = CharacterComponent->GetBaseAimRotation();
			SpineToInterpTo = UKismetMathLibrary::NormalizedDeltaRotator(SpineToInterpTo, TryGetPawnOwner()->GetActorRotation());
			SpineToInterpTo.Roll = SpineToInterpTo.Pitch * -1.0f;
			SpineToInterpTo.Pitch = 0.0f;
			SpineRotation = UKismetMathLibrary::RInterpTo(SpineRotation, SpineToInterpTo, DeltaSeconds, 10.0f);
		}
		else
		{
			SpineToInterpTo = CharacterComponent->GetControlRotation();
			SpineToInterpTo = UKismetMathLibrary::NormalizedDeltaRotator(SpineToInterpTo, TryGetPawnOwner()->GetActorRotation());
			SpineToInterpTo.Roll = SpineToInterpTo.Pitch * -1.0f;
			SpineToInterpTo.Pitch = 0.0f;
			SpineToInterpTo.Yaw = 0.0f;
			SpineRotation = SpineToInterpTo;
		}

		if (bInterpLeaning)
		{
			InterpLeaning(DeltaSeconds);
		}
	}
}

void UFPSTemplateAnimInstance::SetFreeLook(bool FreeLook)
{
	if (CharacterComponent)
	{
		bFreeLook = FreeLook;
		if (bFreeLook)
		{
			FreeLookStartRotation = CharacterComponent->GetControlRotation();
		}
		else
		{
			CharacterComponent->FreeLookEnd.Broadcast(FreeLookStartRotation);
		}
	}
}

void UFPSTemplateAnimInstance::InterpRelativeToHand(float DeltaSeconds)
{
	// Change InterpSpeed to weight of firearm
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		AnimationIndex = AimInterface->GetAnimationIndex();
	
		float InterpSpeed = AimInterpolationSpeed;
		float Multiplier = AimInterface->GetAimInterpolationMultiplier();
		Multiplier = UKismetMathLibrary::NormalizeToRange(Multiplier, -40.0f, 150.0f);
		Multiplier = FMath::Clamp(Multiplier, 0.0f, 1.0f);
		InterpSpeed *= Multiplier;
	
		// Change InterpSpeed to be modified by firearm in hand
		RelativeToHandTransform = UKismetMathLibrary::TInterpTo(RelativeToHandTransform, FinalRelativeHand, DeltaSeconds, InterpSpeed);
		RelativeToHandLocation = RelativeToHandTransform.GetLocation();
		RelativeToHandRotation = RelativeToHandTransform.Rotator();
		
		float HandToSightDistance = FinalRelativeHand.GetLocation().X;
		bool UseFixedCameraDistance = false;
		if (CharacterComponent->IsLocallyControlled())
		{
			const FAimCameraSettings CameraSettings = AimInterface->GetCameraSettings();
			UseFixedCameraDistance = CameraSettings.bUsedFixedCameraDistance;
			if (UseFixedCameraDistance)
			{
				HandToSightDistance = CameraSettings.CameraDistance;
			}
			else
			{
				HandToSightDistance -= CameraSettings.CameraDistance;
			}
		}

		if (!UseFixedCameraDistance && IsValid(Firearm))
		{
			HandToSightDistance -= Firearm->GetStockLengthOfPull() / 2.0f;
		}
		SightDistance = UKismetMathLibrary::FInterpTo(SightDistance, HandToSightDistance * -1.0f, DeltaSeconds, InterpSpeed);
		SetSightTransform();

		if (RelativeToHandTransform.Equals(FinalRelativeHand))
		{
			bInterpRelativeToHand = false;
			SightDistance = HandToSightDistance * -1.0f;
			SetSightTransform();
		}
	}
	else
	{
		AnimationIndex = 0;
	}
}

void UFPSTemplateAnimInstance::SetSightTransform()
{
	/*FTransform CameraTransform = CharacterComponent->GetInUseMesh()->GetSocketTransform(CharacterComponent->GetCameraSocket(), RTS_ParentBoneSpace);
	FRotator Rotation = CameraTransform.Rotator();
	
	FTransform NewTransform = CharacterComponent->GetCameraComponent()->GetComponentTransform();
	Rotation = CharacterComponent->GetControlRotation();
	FVector AimLocation = NewTransform.GetLocation() + Rotation.Vector() * (SightDistance + 5.0f);
	NewTransform.SetLocation(AimLocation);
	
	DrawDebugSphere(GetWorld(), AimLocation, 1.0f, 16.0f, FColor::Red);

	NewTransform = UKismetMathLibrary::MakeRelativeTransform(NewTransform, CharacterComponent->GetInUseMesh()->GetComponentTransform());
	//UE_LOG(LogTemp, Warning, TEXT("Pitch: %f  Yaw: %f  Roll: %f"), Rotation.Pitch, Rotation.Yaw, Rotation.Roll);
	
	CameraTransform.SetRotation(NewTransform.Rotator().Quaternion());
	
	FVector CameraVector = NewTransform.GetLocation();*/

	FTransform CameraTransform = CharacterComponent->GetInUseMesh()->GetSocketTransform(CharacterComponent->GetCameraSocket(), RTS_ParentBoneSpace);

	FRotator NewRot = FRotator::ZeroRotator;
	NewRot.Roll += -90.0f;
	NewRot.Yaw += 90.0f;

	CameraTransform.SetRotation(NewRot.Quaternion());
	
	FVector CameraVector = CameraTransform.GetLocation();
	CameraVector.Y += SightDistance + 10.0f;

	if (!CharacterComponent->IsLocallyControlled() && IsValid(Firearm))
	{
		FVector StockOffset = Firearm->GetStockOffset() * -1.0f;
		StockOffset.Y += Firearm->GetStockLengthOfPull();
		
		CameraVector.X += StockOffset.Z * -1.0f;
		CameraVector.Z += StockOffset.X;
		CameraVector.Y += StockOffset.Y;
	}

	CameraTransform.SetLocation(CameraVector);
	
	SightLocation = CameraTransform.GetLocation();
	SightRotation = CameraTransform.Rotator();
}

void UFPSTemplateAnimInstance::SetRelativeToHand()
{
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		HeadAimingRotation = AimInterface->GetHeadRotation();
		//HeadAimingRotation = FRotator(45.0f, 0.0f, 0.0f);
		FTransform SightSocketTransform = FTransform();
		if (SprintAlpha > 0.0f)
		{
			SightSocketTransform = AimInterface->GetDefaultSightSocketTransform();
		}
		else
		{
			SightSocketTransform = AimInterface->GetSightSocketTransform();
		}
		const FTransform Hand_RTransform = CharacterComponent->GetInUseMesh()->GetSocketTransform(RightHandBone);
		FinalRelativeHand = UKismetMathLibrary::MakeRelativeTransform(SightSocketTransform, Hand_RTransform);
		
		FVector TestLoc = SightSocketTransform.GetLocation();
		TestLoc.Z += 80.0f;
		SightSocketTransform.SetLocation(TestLoc);
		
		const FTransform DefaultTransform = AimInterface->GetDefaultSightSocketTransform();
		DefaultRelativeToHand = UKismetMathLibrary::MakeRelativeTransform(DefaultTransform, Hand_RTransform);
		bInterpRelativeToHand = true;
	}
}

void UFPSTemplateAnimInstance::SetLeftHandIK()
{
	if (IsValid(Firearm))
	{
		const FTransform LeftHandIK = Firearm->GetLeftHandIKTransform();
		if (LeftHandIK.GetLocation().Equals(FVector::ZeroVector))
		{
			LeftHandIKAlpha = 0.0f; return;
		}
		LeftHandIKAlpha = 1.0f;
		
		FVector OutPosition;
		FRotator OutRotation;

		CharacterComponent->GetInUseMesh()->TransformToBoneSpace(RightHandBone, LeftHandIK.GetLocation(), LeftHandIK.Rotator(), OutPosition, OutRotation);

		LeftHandIKTransform.SetLocation(OutPosition);
		LeftHandIKTransform.SetRotation(OutRotation.Quaternion());
		LeftHandIKTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
}

void UFPSTemplateAnimInstance::InterpPortPose(float DeltaSeconds)
{
	if (Firearm)
	{
		FTransform InterpTo = FTransform(SightRotation, SightLocation, FVector::OneVector);
		switch (PortPose)
		{
		case EPortPose::High: InterpTo = Firearm->GetHighPortPose(); break;
		case EPortPose::Low: InterpTo = Firearm->GetLowPortPose(); break;
		default: InterpTo = FTransform();
		}
	
		FTransform CurrentPose = FTransform(CurrentPoseRotation, CurrentPoseLocation, FVector::OneVector);
		CurrentPose = UKismetMathLibrary::TInterpTo(CurrentPose, InterpTo, DeltaSeconds, Firearm->GetPortPoseInterpolationSpeed());
		CurrentPoseLocation = CurrentPose.GetLocation();
		CurrentPoseRotation = CurrentPose.Rotator();
		
		if (CurrentPose.Equals(InterpTo, 12.0f) && PortPose == EPortPose::None)
		{
			PortPoseAlpha = 0.0f;
		}
		
		if (!bInterpShakeCurve && Firearm->GetPerformShakeAfterPortPose() && PortPoseAlpha == 0.0f)
		{
			PlayFirearmShakeCurve();
		}
		
		if (CurrentPose.Equals(InterpTo))
		{
			bInterpPortPose = false;
			if (PortPose == EPortPose::None)
			{
				bCanPlayShakeCurve = true;
			}
		}
	}
}

void UFPSTemplateAnimInstance::SetPortPose(EPortPose Pose)
{
	if (PortPose != Pose)
	{
		PortPose = Pose;
		if (PortPose != EPortPose::None)
		{
			CurrentPoseLocation = FVector::ZeroVector;
			CurrentPoseRotation = FRotator::ZeroRotator;
			SetIsAiming(false);
		}

		RelativeToHandTransform = DefaultRelativeToHand;
		RelativeToHandLocation = RelativeToHandTransform.GetLocation();
		RelativeToHandRotation = RelativeToHandTransform.Rotator();
		PortPoseAlpha = 1.0f;
		bInterpPortPose = true;
	}
}

void UFPSTemplateAnimInstance::SetPortPoseBlend(EPortPose Pose, float Alpha)
{
	if (Firearm)
	{
		PortPose = Pose;
		PortPoseAlpha = Alpha;
	
		if (Pose == EPortPose::None)
		{
			return;
		}

		SetIsAiming(false);
	
		RelativeToHandTransform = DefaultRelativeToHand;
		RelativeToHandLocation = RelativeToHandTransform.GetLocation();
		RelativeToHandRotation = RelativeToHandTransform.Rotator();
		bInterpPortPose = true;
	}
}

bool UFPSTemplateAnimInstance::HandleFirearmCollision(EPortPose Pose, float Alpha)
{
	if (Firearm)
	{
		if (PortPose == EPortPose::None)
		{
			const FTransform ShortStockPose = Firearm->GetShortStockPose();
			const float InterpSpeed = Firearm->GetShortStockInterpSpeed();
			ShortStockPoseLocation = UKismetMathLibrary::VInterpTo(ShortStockPoseLocation, ShortStockPose.GetLocation() * Alpha, GetWorld()->DeltaTimeSeconds, InterpSpeed);
			ShortStockPoseRotation = UKismetMathLibrary::RInterpTo(ShortStockPoseRotation, ShortStockPose.Rotator() * Alpha, GetWorld()->DeltaTimeSeconds, InterpSpeed);
			if (Alpha > 0.0f)
			{
				ShortStockPoseAlpha = 1.0f;
				bInCollisionPose = true;
				SetIsAiming(false);
			}
			else
			{
				bInCollisionPose = false;
				if (CharacterComponent->IsAiming())
				{
					SetIsAiming(true);
				}
			}

			if (ShortStockPoseLocation.Equals(ShortStockPose.GetLocation(), 1.0f))
			{
				ShortStockPoseAlpha = 0.0f;
				ShortStockPoseLocation = FVector::ZeroVector;
				ShortStockPoseRotation = FRotator::ZeroRotator;
				SetPortPoseBlend(Pose, Alpha);
				return true;
			}
			return false;
		}
		else
		{
			ShortStockPoseAlpha = 0.0f;
			ShortStockPoseLocation = FVector::ZeroVector;
			ShortStockPoseRotation = FRotator::ZeroRotator;
			SetPortPoseBlend(Pose, Alpha);
			return true;
		}
	}
	return false;
}

void UFPSTemplateAnimInstance::InterpShakeCurve(float DeltaSeconds)
{
	if (Firearm)
	{
		if (const UCurveVector* Curve = Firearm->GetShakeCurve())
		{
			const float CurrentTime = (GetWorld()->GetTimeSeconds() - ShakeCurveStartTime) * Firearm->GetShakeCurveSpeed();
			const FVector Shake = Curve->GetVectorValue(CurrentTime);
			ShakeRotation = FRotator(Shake.X, Shake.Y, Shake.Z);

			if (CurrentTime > Firearm->GetShakeCurveDuration())
			{
				bInterpShakeCurve = false;
				ShakeCurveAlpha = 0.0f;
			}
		}
		else
		{
			ShakeCurveAlpha = 0.0f;
			bInterpShakeCurve = false;
		}
	}
	else
	{
		ShakeCurveAlpha = 0.0f;
		bInterpShakeCurve = false;
	}
}

void UFPSTemplateAnimInstance::PlayFirearmShakeCurve(bool ManuallyPlay)
{
	if (Firearm && bCanPlayShakeCurve)
	{
		bCanPlayShakeCurve = ManuallyPlay;
		ShakeRotation = FRotator::ZeroRotator;
		bInterpShakeCurve = true;
		ShakeCurveAlpha = 1.0f;
		ShakeCurveStartTime = GetWorld()->GetTimeSeconds();
	}
}

void UFPSTemplateAnimInstance::EnableLeftHandIK(bool Enable)
{
	if (Enable)
	{
		LeftHandIKAlpha = 1.0f;
	}
	else
	{
		LeftHandIKAlpha = 0.0f;
	}
}

void UFPSTemplateAnimInstance::InterpCameraZoom(float DeltaSeconds)
{
	float CurrentFOV = CharacterComponent->GetCameraComponent()->FieldOfView;
	float TargetFOV = CharacterComponent->GetDefaultFOV();
	float InterpSpeed = 10.0f;
	if (bIsAiming && IsValid(AimingActor))
	{
		if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
		{
			const FAimCameraSettings CameraSettings = AimInterface->GetCameraSettings();
			TargetFOV -= CameraSettings.CameraFOVZoom;
			InterpSpeed = CameraSettings.CameraFOVZoomSpeed;
		}
	}
	CurrentFOV = UKismetMathLibrary::FInterpTo(CurrentFOV, TargetFOV, DeltaSeconds, InterpSpeed);
	CharacterComponent->GetCameraComponent()->SetFieldOfView(CurrentFOV);
	if (CurrentFOV == TargetFOV)
	{
		bInterpCameraZoom = false;
	}
}

void UFPSTemplateAnimInstance::InterpAimingAlpha(float DeltaSeconds)
{
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		float InterpSpeed = AimInterpolationSpeed;
		float Multiplier = AimInterface->GetAimInterpolationMultiplier();
		Multiplier = UKismetMathLibrary::NormalizeToRange(Multiplier, -40.0f, 150.0f);
		InterpSpeed *= Multiplier;

		AimingAlpha = UKismetMathLibrary::FInterpTo(AimingAlpha, bIsAiming, DeltaSeconds, InterpSpeed);
		
		if ((bIsAiming && AimingAlpha >= 1.0f) || (!bIsAiming && AimingAlpha <= 0.0f))
		{
			bInterpAiming = false;
		}
	}
}

void UFPSTemplateAnimInstance::SetRotationLag(float DeltaSeconds)
{
	if (IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		float InterpSpeed = RotationLagResetInterpolationSpeed;
		float Multiplier = AimInterface->GetRotationLagInterpolationMultiplier();
		Multiplier = UKismetMathLibrary::NormalizeToRange(Multiplier, -40.0f, 150.0f);
		InterpSpeed *= Multiplier;
	
		const FRotator CurrentRotation = CharacterComponent->GetControlRotation();
		FRotator Rotation = UKismetMathLibrary::RInterpTo(UnmodifiedRotationLagTransform.Rotator(), CurrentRotation - OldRotation, DeltaSeconds, InterpSpeed);
		UnmodifiedRotationLagTransform.SetRotation(Rotation.Quaternion());

		float FirearmWeightMultiplier = 1.0f;
		if (IsValid(Firearm))
		{
			// Modify on HELD weapon weight	
			FirearmWeightMultiplier = UKismetMathLibrary::NormalizeToRange(Firearm->GetFirearmStats().Weight, 0.0f, 25.0f);
			FirearmWeightMultiplier = FirearmWeightMultiplier * (0.3f / Multiplier);
		}
		
		Rotation *= 3.0f;
		Rotation *= FirearmWeightMultiplier;

		float InvertMultiplier = -1.0f;
		Rotation.Roll = Rotation.Pitch;
		Rotation.Pitch = 0.0f;
		if (bInvertRotationLag)
		{
			InvertMultiplier = 1.0f;
		}
		Rotation.Yaw = FMath::Clamp(Rotation.Yaw, -7.0f, 7.0f) * InvertMultiplier;	
		Rotation.Roll = FMath::Clamp(Rotation.Roll, -3.0f, 3.0f) * -InvertMultiplier;

		RotationLagRotation = Rotation;
		RotationLagLocation = FVector(Rotation.Yaw / 4.0f, 0.0f, Rotation.Roll / 1.5);
		RotationLagTransform.SetRotation(Rotation.Quaternion());
		RotationLagTransform.SetLocation(FVector(Rotation.Yaw / 4.0f, 0.0f, Rotation.Roll / 1.5));

		OldRotation = CurrentRotation;
	}
}

void UFPSTemplateAnimInstance::SetMovementLag(float DeltaSeconds)
{
	if (const IFPSTemplate_AimInterface* AimInterface = Cast<IFPSTemplate_AimInterface>(AimingActor))
	{
		const FVector Velocity = CharacterComponent->GetMovementComponent()->Velocity;
		float RightSpeed = FVector::DotProduct(Velocity, CharacterComponent->GetActorRightVector());
		float VerticalSpeed = Velocity.Z;
		RightSpeed = UKismetMathLibrary::NormalizeToRange(RightSpeed, 0.0f, 75.0f);
		VerticalSpeed = UKismetMathLibrary::NormalizeToRange(VerticalSpeed, 0.0f, 75.0f);

		const FSwayMultipliers SwayMultipliers = AimInterface->GetSwayMultipliers();
		FRotator NewRot = MovementLagRotation;
		NewRot.Pitch = UKismetMathLibrary::FInterpTo(NewRot.Pitch, RightSpeed * SwayMultipliers.MovementRollMultiplier, DeltaSeconds, 10.0f);
		NewRot.Roll = UKismetMathLibrary::FInterpTo(NewRot.Roll, VerticalSpeed * SwayMultipliers.MovementPitchMultiplier, DeltaSeconds, 10.0f);
		MovementLagRotation = NewRot;
	}
}

void UFPSTemplateAnimInstance::InterpLeaning(float DeltaSeconds)
{
	float LeanAngle = 0.0f;
	const float NewLeanAngle = CharacterComponent->GetLeanAngle() / 2.0f;
	float CurrentLeanAngle = LeanRotation.Pitch;
	switch (CurrentLean)
	{
		case ELeaning::None : break;
		case ELeaning::Left : LeanAngle = NewLeanAngle * -1.0f; break;
		case ELeaning::Right : LeanAngle = NewLeanAngle; break;
	}

	CurrentLeanAngle = UKismetMathLibrary::FInterpTo(CurrentLeanAngle, LeanAngle, DeltaSeconds, 10.0f);
	
	LeanRotation.Pitch = CurrentLeanAngle;
	if (CurrentLeanAngle == NewLeanAngle || CurrentLeanAngle == NewLeanAngle * -1.0f)
	{
		bInterpLeaning = false;
	}
}

void UFPSTemplateAnimInstance::HandleMovementSway(float DeltaSeconds)
{	// REWRITE FOR USE WITH AIMING ACTOR AS WELL???
	if (IsValid(Firearm))
	{
		if (const UCurveVector* Curve = Firearm->GetMovementSwayCurve())
		{
			const float OldVelocityMultiplier = VelocityMultiplier;
			VelocityMultiplier = UKismetMathLibrary::NormalizeToRange(CharacterVelocity, 0.0f, CharacterComponent->GetMovementComponentSprintSpeed());
			if (VelocityMultiplier < OldVelocityMultiplier)
			{
				VelocityMultiplier = UKismetMathLibrary::FInterpTo(OldVelocityMultiplier, VelocityMultiplier, DeltaSeconds, 3.2f);
			}
			if (VelocityMultiplier < 0.25f)
			{
				VelocityMultiplier = 0.25f;
			}
			float Multiplier = 1.1f;
			if (Firearm->UseStatsForMovementSway())
			{
				const FFirearmStats Stats = Firearm->GetFirearmStats();
				Multiplier = Stats.Weight * (Stats.Weight * 3.0f / (Stats.Ergonomics * 1.5f)) + 1.0f;
				Multiplier = FMath::Clamp(Multiplier, 0.5f, 1.0f);
			}

			SwayMultiplier = UKismetMathLibrary::FInterpTo(SwayMultiplier, Firearm->GetAimSwayMultiplier(), DeltaSeconds, 2.0f);
			
			CurveTimer += (DeltaSeconds * VelocityMultiplier);
			FVector Graph = Curve->GetVectorValue(CurveTimer);
			Graph *= VelocityMultiplier * Multiplier;
			const FRotator Rotation = FRotator(Graph.Y, Graph.X, Graph.Z);
			SwayLocation = Graph * SwayMultiplier;
			SwayRotation = Rotation * SwayMultiplier;
		}
	}
}

void UFPSTemplateAnimInstance::HandleSprinting()
{
	if (CharacterComponent->GetIsSprinting() && CharacterVelocity > CharacterComponent->GetMovementComponentSprintSpeed() / 2.0f)
	{
		if (PortPose != EPortPose::None)
		{
			CharacterComponent->StopHighAndLowPortPose();
		}
		SprintAlpha = 1.0f;
		if (bSprinting != true)
		{
			bSprinting = true;
			SetIsAiming(false);
		}
		if (IsValid(Firearm))
		{
			SprintPoseLocation = Firearm->GetSprintPose().GetLocation();
			SprintPoseRotation = Firearm->GetSprintPose().Rotator();
		}
	}
	else if (bSprinting != false)
	{
		bSprinting = false;
		if (CharacterComponent->IsAiming())
		{
			SetIsAiming(true);
		}
		SprintAlpha = 0.0f;
	}
}

bool UFPSTemplateAnimInstance::IsLocallyControlled()
{
	if (CharacterComponent)
	{
		bIsLocallyControlled = CharacterComponent->IsLocallyControlled();
	}
	return bIsLocallyControlled;
}

void UFPSTemplateAnimInstance::SetIsAiming(bool IsAiming)
{
	if ((IsAiming && !bCanAim) || bInCollisionPose || bSprinting)
	{
		bIsAiming = false;
		bInterpAiming = true;
		bInterpCameraZoom = true;
		return;
	}

	if (bIsAiming != IsAiming)
	{
		bIsAiming = IsAiming;
		bInterpAiming = true;
		bInterpCameraZoom = true;
	}
}

void UFPSTemplateAnimInstance::SetLeaning(ELeaning Lean)
{
	CurrentLean = Lean;
	bInterpLeaning = true;
}

void UFPSTemplateAnimInstance::SetIsReloading(bool IsReloading, float BlendAlpha)
{
	if (IsReloading)
	{
		RotationAlpha = BlendAlpha;
		LeftHandIKAlpha = 0.0f;
	}
	else
	{
		RotationAlpha = 0.0f;
		LeftHandIKAlpha = 1.0f;
	}
}

void UFPSTemplateAnimInstance::SetCanAim(bool CanAim)
{
	bCanAim = CanAim;
}

void UFPSTemplateAnimInstance::ChangingFireMode(bool IsChanging)
{
	if (IsChanging)
	{
		RotationAlpha = 0.55f;
	}
	else
	{
		RotationAlpha = 0.0f;
	}
}

void UFPSTemplateAnimInstance::CycledSights()
{
	SetRelativeToHand();
	bInterpCameraZoom = true;
}

void UFPSTemplateAnimInstance::EnterCustomPose(const FTransform& Pose)
{
	CustomPoseLocation = Pose.GetLocation();
	CustomPoseRotation = Pose.Rotator();
	CustomPoseAlpha = 1.0f;
}

void UFPSTemplateAnimInstance::ClearCustomPose()
{
	CustomPoseAlpha = 0.0f;
}

void UFPSTemplateAnimInstance::StopMontages(float BlendOutTime)
{
	Montage_Stop(BlendOutTime);
}

void UFPSTemplateAnimInstance::SetCustomization(bool CustomizeFirearm)
{
	bCustomizingFirearm = CustomizeFirearm;
	if (bCustomizingFirearm)
	{
		LeftHandIKAlpha = 0.0f;
		AimingAlpha = 0.0f;
		PortPoseAlpha = 0.0f;
	}
	else
	{
		LeftHandIKAlpha = 1.0f;
	}
}

void UFPSTemplateAnimInstance::RecoilInterpToZero(float DeltaSeconds)
{
	FinalRecoilTransform = UKismetMathLibrary::TInterpTo(FinalRecoilTransform, FTransform(), DeltaSeconds, 8.0f); // def = 6
	RecoilLocation = FinalRecoilTransform.GetLocation();
	RecoilRotation = FinalRecoilTransform.Rotator();
	if (RecoilLocation.Equals(FVector::ZeroVector, 0.1f) && RecoilRotation.Equals(FRotator::ZeroRotator, 0.1f))
	{
		bInterpRecoil = false;
	}
}

void UFPSTemplateAnimInstance::RecoilInterpTo(float DeltaSeconds)
{
	if (Firearm)
	{
		const float CurrentTime = GetWorld()->GetTimeSeconds() - RecoilStartTime;
		const FRecoilData RecoilData = Firearm->GetRecoilData();
		if (RecoilData.RecoilLocationCurve)
		{
			const float Randomess = FMath::RandRange(RecoilData.RecoilLocationRandomness.Min, RecoilData.RecoilLocationRandomness.Max);
			RecoilLocation += (RecoilData.RecoilLocationCurve->GetVectorValue(CurrentTime) * Randomess);
			RecoilLocation *= Firearm->GetFirearmStats().RecoilMultiplier;
			FinalRecoilTransform.SetLocation(RecoilLocation);
		}
		if (RecoilData.RecoilRotationCurve)
		{
			const FVector CurveData = RecoilData.RecoilRotationCurve->GetVectorValue(CurrentTime);
			const float PitchRandom = (CurveData.X * FMath::RandRange(RecoilData.RecoilPitchRandomness.Min, RecoilData.RecoilPitchRandomness.Max)) * RecoilMultiplier;
			const float YawRandom = (CurveData.Y * FMath::RandRange(RecoilData.RecoilYawRandomness.Min, RecoilData.RecoilYawRandomness.Max)) * RecoilMultiplier;
			const float RollRandom = (CurveData.Z * FMath::RandRange(RecoilData.RecoilRollRandomness.Min, RecoilData.RecoilRollRandomness.Max)) * RecoilMultiplier;
			
			RecoilRotation += FRotator(RollRandom, PitchRandom, YawRandom);
			RecoilRotation *= Firearm->GetFirearmStats().RecoilMultiplier;
			FinalRecoilTransform.SetRotation(RecoilRotation.Quaternion());
		}
	}

	RecoilInterpToZero(DeltaSeconds);
	
	/*float Push = RecoilLocation.Y;
	FFloatSpringState SpringState;
	Push = UKismetMathLibrary::FloatSpringInterp(Push, FinalRecoilTransform.GetLocation().Y, SpringState,
		1.0f, 1.0f, DeltaSeconds, 1.0f);
	
	RecoilTransform = UKismetMathLibrary::TInterpTo(RecoilTransform, FinalRecoilTransform, DeltaSeconds, 25.0f);// def = 15
	FVector Vec = RecoilTransform.GetLocation();
	Vec.Y = Push;
	RecoilTransform.SetLocation(Vec);
	RecoilLocation = Vec;
	RecoilRotation = RecoilTransform.Rotator();*/
}

void UFPSTemplateAnimInstance::PerformRecoil(float Multiplier)
{
	if (Firearm)
	{
		bInterpRecoil = true;
		RecoilStartTime = GetWorld()->GetTimeSeconds();
		RecoilMultiplier = Multiplier;
		/*float TotalRecoil = 0.0f;
		Multiplier = FMath::Clamp(Multiplier, 0.01f, 4.0f);
		float Weight = 1.0f;
		if (Firearm->GetFirearmStats().Weight > 0.0f)
		{
			Weight = Firearm->GetFirearmStats().Weight;
		}
		float RecoilVerticalMultiplier = Firearm->GetFirearmStats().VerticalRecoil * (1 / (Weight / 2.0f));
		RecoilVerticalMultiplier = UKismetMathLibrary::NormalizeToRange(RecoilVerticalMultiplier, 0.0f, 160.0f);
		RecoilVerticalMultiplier *= Multiplier;
		TotalRecoil += RecoilVerticalMultiplier;
			
		float RecoilHorizontalMultiplier = Firearm->GetFirearmStats().HorizontalRecoil * (1 / (Weight / 2.0f));
		RecoilHorizontalMultiplier = UKismetMathLibrary::NormalizeToRange(RecoilHorizontalMultiplier, 0.0f, 260.0f);
		RecoilHorizontalMultiplier *= Multiplier;
		TotalRecoil += RecoilHorizontalMultiplier;
			
		FVector RecoilLoc = FinalRecoilTransform.GetLocation();
		RecoilLoc += FVector(0.0f, FMath::RandRange(-3.0f, -2.0f) * TotalRecoil, FMath::RandRange(0.0f, 0.5f) * TotalRecoil);

		FRotator RecoilRot = FinalRecoilTransform.GetRotation().Rotator();
		// Pitch = Roll,		Yaw = Yaw,		Roll = Pitch
		RecoilRot += FRotator(FMath::RandRange(-0.0f, 0.0f),
			FMath::RandRange(-1.0f, 2.5f) * RecoilHorizontalMultiplier,
			FMath::RandRange(-5.0f, -2.0f) * RecoilVerticalMultiplier);
		RecoilRot.Roll = FMath::Clamp(RecoilRot.Roll, -15.0f, 0.0f);

		FinalRecoilTransform.SetLocation(RecoilLoc);
		FinalRecoilTransform.SetRotation(RecoilRot.Quaternion());*/
	}
}