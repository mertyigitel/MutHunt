// Copyright 2021, Dakota Dawe, All rights reserved


#include "Components/FPSTemplate_CharacterComponent.h"
#include "FPSTemplateAnimInstance.h"
#include "Actors/FPSTemplateFirearm.h"
#include "Misc/FPSTemplateStatics.h"
#include "Interfaces/FPSTemplate_AimInterface.h"

#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UFPSTemplate_CharacterComponent::UFPSTemplate_CharacterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	
	FirearmCollisionChannel = ECC_GameTraceChannel2;

	bAutoSetOwnerOnEquipFirearmAndAimingActor = true;
	
	bIsInitialized = false;
	bLocallyControlled = false;
	
	MovementComponentSprintSpeed = 350.0f;
	bForceIntoSprintPose = false;
	MaxLookUpAngle = 80.0f;
	MaxLookDownAngle = 80.0f;
	DefaultLeanAngle = 35.0f;
	MaxCrouchDepth = 30.0f;
	CrouchDepthIncrement = 2.5f;
	LeanAngle = DefaultLeanAngle;
	DefaultCameraFOV = 90.0f;
	
	CameraSocket = FName("Camera");
	CameraSocketParentBone = FName("head");
	RightHandAxis = EAxis::Type::Y;
	
	MaxFirearmAttachmentAttempts = 5;
	FirearmReAttachmentAttemptInterval = 0.5f;
	AttachmentAttempt = 0;

	bUseLeftHandIK = true;

	bFreeLook = false;

	MaxLookLeftRight = 50.0f;
	MaxLookUpDown = 35.0f;

	LookUpDownOffset = 0.0f;

	FirearmCollisionDistanceCheck = 12.0f;

	ControlYaw = 0.0f;

	bIsThirdPersonDefault = false;
	bInThirdPerson = bIsThirdPersonDefault;

	CrouchHeight = 0.0f;

	DefaultLeanSpeed = 10.0f;
	LeanSpeed = DefaultLeanSpeed;
	IncrementalLeanAngle = 0.0f;
	bIsIncrementalLeaning = false;
	IncrementalLeanAmount = 2.5;
}

void UFPSTemplate_CharacterComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bUseFirearmCollision)
	{
		PrimaryComponentTick.TickInterval = 1.0f / 60.0f; // tick firearm collision at 60fps
	}
	else
	{
		SetComponentTickEnabled(false);
	}
}

void UFPSTemplate_CharacterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME(UFPSTemplate_CharacterComponent, CurrentFirearm);
	DOREPLIFETIME(UFPSTemplate_CharacterComponent, AimingActor);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, LeanAngle, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, LeanSpeed, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, IncrementalLeanAngle, COND_SkipOwner);
	DOREPLIFETIME(UFPSTemplate_CharacterComponent, CrouchHeight);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, bUseLeftHandIK, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, bIsAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, CurrentLean, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, bIsSprinting, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, PortPose, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, bFreeLook, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UFPSTemplate_CharacterComponent, ControlYaw, COND_SkipOwner);
}

void UFPSTemplate_CharacterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (HasAuthority())
	{
		ControlYaw = GetControlRotation().Yaw;
	}
	
	if (bUseFirearmCollision && FPCameraComponent && CurrentFirearm && AnimationInstance.IsValid() && PortPose == EPortPose::None)
	{
		FVector Start = FPCameraComponent->GetComponentLocation();
		FVector Muzzle = CurrentFirearm->GetMuzzleSocketTransform().GetLocation();
		FRotator Rotation = FPCameraComponent->GetComponentRotation();

		if (!bIsAiming)
		{
			Rotation.Yaw += 8.0f;
			Rotation.Pitch -= 2.0f;
		}
		FVector End = Start + Rotation.Vector() * 150.0f;
		float DistanceToMuzzle = FVector::Distance(Start, Muzzle);

		FHitResult HitResult;		
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetOwner());

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, FirearmCollisionChannel, Params))
		{
			float HitDistance = HitResult.Distance - DistanceToMuzzle;

			EPortPose Pose = EPortPose::High;
			float Normalized = 0.0f;
			if (HitDistance < FirearmCollisionDistanceCheck)
			{
				HitDistance *= -1.0f;
				Normalized = UKismetMathLibrary::NormalizeToRange(HitDistance, FirearmCollisionDistanceCheck * -1.0f, -5.0f);
				Normalized = FMath::Clamp(Normalized,0.0f, 1.0f);

				float Pitch = GetBaseAimRotation().Pitch;
				if (IsLocallyControlled())
				{
					if (Pitch < 0.0f)
					{
						Pose = EPortPose::Low;
					}
				}
				else
				{
					if (Pitch > 89.0f)
					{
						Pose = EPortPose::Low;
					}
				}

				bFirearmCollisionHitting = AnimationInstance->HandleFirearmCollision(Pose, Normalized);
			}
			else if (HitDistance > FirearmCollisionDistanceCheck)
			{
				Normalized = 0.0f;
				Pose = EPortPose::None;
				bFirearmCollisionHitting = AnimationInstance->HandleFirearmCollision(Pose, Normalized);
			}
		}
		else
		{
			bFirearmCollisionHitting = AnimationInstance->HandleFirearmCollision(EPortPose::None, 0.0f);
		}
	}
}

void UFPSTemplate_CharacterComponent::Init(UCameraComponent* CameraComponent, bool bAutoAttach, USkeletalMeshComponent* FirstPersonMesh, USkeletalMeshComponent* ThirdPersonMesh)
{
	LeanAngle = DefaultLeanAngle;
	LeanSpeed = DefaultLeanSpeed;
	FPCameraComponent = CameraComponent;
	FPMesh = FirstPersonMesh;
	TPMesh = ThirdPersonMesh;

	OwningPawn = GetOwner<APawn>();

	if (OwningPawn.IsValid())
	{
		if (OwningPawn->IsLocallyControlled())
		{
			AnimationInstance = Cast<UFPSTemplateAnimInstance>(FPMesh->GetAnimInstance());
			bLocallyControlled = true;
		}
		else
		{
			AnimationInstance = Cast<UFPSTemplateAnimInstance>(TPMesh->GetAnimInstance());
		}
	}

	if (AnimationInstance.IsValid())
	{
		AnimationInstance->SetCharacterComponent(this);
	}

	if (FPCameraComponent)
	{
		DefaultCameraFOV = FPCameraComponent->FieldOfView;
		if (IsLocallyControlled())
		{
			if (bAutoAttach)
			{
				FPCameraComponent->AttachToComponent(GetInUseMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CameraSocket);
			}
			
			GetInUseMesh()->SetOwnerNoSee(false);
			if (const APlayerController* PC = GetOwner<APawn>()->GetController<APlayerController>())
			{
				if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
				{
					CameraManager->ViewPitchMax = MaxLookUpAngle;
					CameraManager->ViewPitchMin = MaxLookDownAngle * -1.0f;
				}
			}
		}
	}

	if (USkeletalMeshComponent* InUseMesh = GetInUseMesh())
	{
		StartMeshLocation = InUseMesh->GetRelativeLocation();
		StartMeshLocation.Z -= CrouchHeight;
	}

	bInThirdPerson = bIsThirdPersonDefault;
	
	bIsInitialized = true;
}


bool UFPSTemplate_CharacterComponent::Server_SetPortPose_Validate(EPortPose NewPortPose)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetPortPose_Implementation(EPortPose NewPortPose)
{
	PortPose = NewPortPose;
	OnRep_PortPose();
}

void UFPSTemplate_CharacterComponent::SetHighPortPose(bool bSkipNone)
{
	if (!bFirearmCollisionHitting)
	{
		bHighPort = true;
		if (PortPose == EPortPose::Low && !bSkipNone)
		{
			PortPose = EPortPose::None;
		}
		else
		{
			PortPose = EPortPose::High;
		}
		
		OnRep_PortPose();
		if (!HasAuthority())
		{
			Server_SetPortPose(PortPose);
		}
	}
}

void UFPSTemplate_CharacterComponent::SetLowPortPose(bool bSkipNone)
{
	if (!bFirearmCollisionHitting)
	{
		bLowPort = true;
		if (PortPose == EPortPose::High && !bSkipNone)
		{
			PortPose = EPortPose::None;
		}
		else
		{
			PortPose = EPortPose::Low;
		}
		
		OnRep_PortPose();
		if (!HasAuthority())
		{
			Server_SetPortPose(PortPose);
		}
	}
}

void UFPSTemplate_CharacterComponent::StopLowPortPose()
{
	if (!bFirearmCollisionHitting)
	{
		bLowPort = false;
		if (bHighPort)
		{
			PortPose = EPortPose::High;
		}
		else
		{
			PortPose = EPortPose::None;
		}
		
		OnRep_PortPose();
		if (!HasAuthority())
		{
			Server_SetPortPose(PortPose);
		}
	}
}

void UFPSTemplate_CharacterComponent::StopHighPortPose()
{
	if (!bFirearmCollisionHitting)
	{
		bHighPort = false;
		if (bLowPort)
		{
			PortPose = EPortPose::Low;
		}
		else
		{
			PortPose = EPortPose::None;
		}
		
		OnRep_PortPose();
		if (HasAuthority())
		{
			Server_SetPortPose(PortPose);
		}
	}
}

void UFPSTemplate_CharacterComponent::StopHighAndLowPortPose()
{
	if (!bFirearmCollisionHitting)
	{
		bHighPort = false;
		bLowPort = false;
		PortPose = EPortPose::None;
		OnRep_PortPose();
		if (HasAuthority())
		{
			Server_SetPortPose(PortPose);
		}
	}
}

void UFPSTemplate_CharacterComponent::OnRep_FreeLook()
{
	SetFreeLook(bFreeLook);
}

void UFPSTemplate_CharacterComponent::SetFreeLook(bool FreeLook)
{
	if (OwningPawn.IsValid() && AnimationInstance.IsValid())
	{
		bFreeLook = FreeLook;
		AnimationInstance->SetFreeLook(bFreeLook);
		OwningPawn->bUseControllerRotationYaw = !bFreeLook;
		if (bFreeLook)
		{
			LookUpDownOffset = OwningPawn->GetControlRotation().Pitch;
		}
	}
}

bool UFPSTemplate_CharacterComponent::ValidTurn(float AxisValue)
{
	if (bFreeLook && OwningPawn.IsValid())
	{
		const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(OwningPawn->GetActorRotation(), GetControlRotation());
		if (Delta.Yaw > MaxLookLeftRight && AxisValue < 0.0f)
		{
			return false;
		}
		if (Delta.Yaw < -MaxLookLeftRight && AxisValue > 0.0f)
		{
			return false;
		}
	}
	return true;
}

bool UFPSTemplate_CharacterComponent::ValidLookUp(float AxisValue)
{
	if (bFreeLook && OwningPawn.IsValid())
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(OwningPawn->GetActorRotation(), ControlRotation);
		
		if (Delta.Pitch > MaxLookUpDown && AxisValue > 0.0f)
		{
			return false;
		}
		if (Delta.Pitch < -MaxLookUpDown && AxisValue < 0.0f)
		{
			return false;
		}
	}
	return true;
}

void UFPSTemplate_CharacterComponent::SetControlRotation(FRotator NewControlRotation)
{
	if (OwningPawn.IsValid() && OwningPawn->Controller)
	{
		OwningPawn->Controller->SetControlRotation(NewControlRotation);
	}
}

void UFPSTemplate_CharacterComponent::OnRep_CrouchHeight()
{
	/*if (USkeletalMeshComponent* Mesh = GetInUseMesh())
	{
		FVector MeshLocation = StartMeshLocation;
		MeshLocation.Z += CrouchHeight;
		GetAnimationInstance()->InterpolateCrouch(StartMeshLocation, CrouchHeight);
	}*/
	GetAnimationInstance()->InterpolateCrouch(StartMeshLocation, CrouchHeight);
}

bool UFPSTemplate_CharacterComponent::Server_CrouchHeight_Validate(float NewHeight)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_CrouchHeight_Implementation(float NewHeight)
{
	if (NewHeight >= -MaxCrouchDepth && NewHeight <= 0.0f)
	{
		CrouchHeight = NewHeight;
		if (CrouchHeight < -MaxCrouchDepth)
		{
			CrouchHeight = -MaxCrouchDepth;
		}
		else if (CrouchHeight > 0.0f)
		{
			CrouchHeight = 0.0f;
		}
		OnRep_CrouchHeight();
	}
}

void UFPSTemplate_CharacterComponent::CrouchIncrement(bool Down)
{
	if (Down)
	{
		CrouchHeight -= CrouchDepthIncrement;
		if (CrouchHeight < -MaxCrouchDepth)
		{
			CrouchHeight = -MaxCrouchDepth;
		}
	}
	else
	{
		CrouchHeight += CrouchDepthIncrement;
		if (CrouchHeight > 0.0f)
		{
			CrouchHeight = 0.0f;
		}
	}
	Server_CrouchHeight(CrouchHeight);
	OnRep_CrouchHeight();
}

void UFPSTemplate_CharacterComponent::GoToCrouch()
{
	CrouchHeight = -MaxCrouchDepth;
	Server_CrouchHeight(CrouchHeight);
	OnRep_CrouchHeight();
}

void UFPSTemplate_CharacterComponent::GoToStanding()
{
	CrouchHeight = 0.0f;
	Server_CrouchHeight(CrouchHeight);
	OnRep_CrouchHeight();
}

AFPSTemplate_SightBase* UFPSTemplate_CharacterComponent::GetCurrentSight() const
{
	if (IsValid(CurrentFirearm))
	{
		return CurrentFirearm->GetCurrentSight();
	}
	return nullptr;
}

void UFPSTemplate_CharacterComponent::OnRep_PortPose() const
{
	if (AnimationInstance.IsValid())
	{
		AnimationInstance->SetPortPose(PortPose);
	}
}

void UFPSTemplate_CharacterComponent::OnRep_IsAiming()
{
	if (AnimationInstance.IsValid())
	{
		if (bIsAiming)
		{
			StopHighAndLowPortPose();
		}
		AnimationInstance->SetIsAiming(bIsAiming);
	}
}

/*void UFPSTemplate_CharacterComponent::OnRep_CurrentFirearm()
{
	if (IsValid(CurrentFirearm))
	{
		CurrentFirearm->SetCharacterComponent(this);
		CurrentFirearm->SetupStabilizerComponent();
		CurrentFirearm->Equip();
		SetAimingActor(CurrentFirearm);
		AttachItem(CurrentFirearm, CurrentFirearm->GetFirearmGripSocket());
	}
}*/

void UFPSTemplate_CharacterComponent::AttachItem(AActor* Actor, const FName SocketName)
{
	if (IsValid(Actor))
	{
		if (USkeletalMeshComponent* AttachToMesh = GetInUseMesh())
		{
			Actor->AttachToComponent(AttachToMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
			if (AFPSTemplateFirearm* Firearm = Cast<AFPSTemplateFirearm>(Actor))
			{
				Firearm->SetCharacterComponent(this);
				const FTransform Offset = Firearm->GetGripOffset();
				Actor->SetActorRelativeTransform(Offset);
			}
			AttachmentAttempt = 0;
		}
		else
		{
			if (++AttachmentAttempt <= MaxFirearmAttachmentAttempts)
			{
				FTimerHandle TTempHandle;
				FTimerDelegate TimerDelegate;
				TimerDelegate.BindUFunction(this, FName("AttachItem"), Actor, SocketName);
				GetWorld()->GetTimerManager().SetTimer(TTempHandle, TimerDelegate, FirearmReAttachmentAttemptInterval, false);
			}
		}
	}
}

bool UFPSTemplate_CharacterComponent::Server_ClearCurrentFirearm_Validate()
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_ClearCurrentFirearm_Implementation()
{
	ClearCurrentFirearm();
}

void UFPSTemplate_CharacterComponent::ClearCurrentFirearm()
{
	CurrentFirearm = nullptr;
	AimingActor = nullptr;
	if (!HasAuthority())
	{
		Server_ClearCurrentFirearm();
	}
}

void UFPSTemplate_CharacterComponent::EquipFirearm(AFPSTemplateFirearm* Firearm)
{
	if (IsValid(CurrentFirearm) && !IsValid(Firearm))
	{
		CurrentFirearm->SetCharacterComponent(nullptr);
	}
	if (bAutoSetOwnerOnEquipFirearmAndAimingActor && HasAuthority() && GetOwner() && Firearm)
	{
		Firearm->SetOwner(GetOwner());
	}
	CurrentFirearm = Firearm;
	AimingActor = CurrentFirearm;
	OnRep_AimingActor();
}

void UFPSTemplate_CharacterComponent::SetAimingActor(AActor* Actor)
{
	if (bAutoSetOwnerOnEquipFirearmAndAimingActor && HasAuthority() && GetOwner() && Actor)
	{
		Actor->SetOwner(GetOwner());
	}
	AimingActor = Actor;
	OnRep_AimingActor();
}

void UFPSTemplate_CharacterComponent::OnRep_AimingActor()
{
	if (IsValid(AimingActor))
	{
		if (AFPSTemplateFirearm* Firearm = Cast<AFPSTemplateFirearm>(AimingActor))
		{
			CurrentFirearm = Firearm;
			CurrentFirearm->SetCharacterComponent(this);
			CurrentFirearm->SetupStabilizerComponent();
			CurrentFirearm->Equip();
			//SetAimingActor(CurrentFirearm);
			//AttachItem(CurrentFirearm, CurrentFirearm->GetFirearmGripSocket());
		}
		if (AimingActor->Implements<UFPSTemplate_AimInterface>())
		{
			AttachItem(AimingActor, IFPSTemplate_AimInterface::Execute_GetGripSocketName(AimingActor));
		}
	}
	else
	{
		CurrentFirearm = nullptr;
	}
}

void UFPSTemplate_CharacterComponent::OnRep_Lean()
{
	bIsIncrementalLeaning = false;

	if (AnimationInstance.IsValid())
	{
		AnimationInstance->SetLeaning(CurrentLean);
	}
}

void UFPSTemplate_CharacterComponent::OnRep_UseLeftHandIK() const
{
	if (AnimationInstance.IsValid())
	{
		AnimationInstance->EnableLeftHandIK(bUseLeftHandIK);
	}
}

USkeletalMeshComponent* UFPSTemplate_CharacterComponent::GetInUseMesh() const
{
	if (OwningPawn.IsValid())
	{
		if (OwningPawn->IsLocallyControlled())
		{
			return FPMesh;
		}
		return TPMesh;
	}

	return TPMesh;
}

UPawnMovementComponent* UFPSTemplate_CharacterComponent::GetMovementComponent() const
{
	if (OwningPawn.IsValid())
	{
		return OwningPawn->GetMovementComponent();
	}
	return nullptr;
}

FRotator UFPSTemplate_CharacterComponent::GetBaseAimRotation() const
{
	if (OwningPawn.IsValid())
	{
		FRotator Rotation = OwningPawn->GetBaseAimRotation();
		Rotation.Yaw = ControlYaw;
		return Rotation;
	}
	return FRotator::ZeroRotator;
}

FRotator UFPSTemplate_CharacterComponent::GetControlRotation() const
{
	if (OwningPawn.IsValid())
	{
		return OwningPawn->GetControlRotation();
	}
	return FRotator::ZeroRotator;
}

const FName& UFPSTemplate_CharacterComponent::GetCameraSocket() const
{
	if (bUseParentSocketForAiming)
	{
		return CameraSocketParentBone;
	}
	return CameraSocket;
}

bool UFPSTemplate_CharacterComponent::Server_SetLeanAngle_Validate(float NewLeanAngle)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetLeanAngle_Implementation(float NewLeanAngle)
{
	SetLeanAngle(NewLeanAngle);
}

void UFPSTemplate_CharacterComponent::SetLeanAngle(float NewLeanAngle)
{
	LeanAngle = NewLeanAngle >= 0.0f ? LeanAngle = NewLeanAngle : LeanAngle = DefaultLeanAngle;
	OnRep_Lean();
	if (!HasAuthority())
	{
		Server_SetLeanAngle(LeanAngle);
	}
}

void UFPSTemplate_CharacterComponent::ResetLeanAngle()
{
	LeanAngle = DefaultLeanAngle;
	bIsIncrementalLeaning = false;
	IncrementalLeanAngle = 0.0f;
	OnRep_Lean();
	if (!HasAuthority())
	{
		Server_SetLeanAngle(LeanAngle);
	}
}

bool UFPSTemplate_CharacterComponent::Server_SetLeanSpeed_Validate(float NewLeanSpeed)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetLeanSpeed_Implementation(float NewLeanSpeed)
{
	SetLeanSpeed(NewLeanSpeed);
}

void UFPSTemplate_CharacterComponent::SetLeanSpeed(float NewLeanSpeed)
{
	LeanSpeed = NewLeanSpeed > 0.0f ? LeanSpeed = NewLeanSpeed : LeanSpeed = DefaultLeanSpeed;
	if (!HasAuthority())
	{
		Server_SetLeanSpeed(LeanSpeed);
	}
}

void UFPSTemplate_CharacterComponent::ResetLeanSpeed()
{
	LeanSpeed = DefaultLeanSpeed;
	if (!HasAuthority())
	{
		Server_SetLeanSpeed(LeanSpeed);
	}
}

void UFPSTemplate_CharacterComponent::OnRep_IncrementalLeanAngle()
{
	if (AnimationInstance.IsValid())
	{
		bIsIncrementalLeaning = LeanAngle != 0.0f;
		
		AnimationInstance->SetLeaning(CurrentLean);
	}
}

bool UFPSTemplate_CharacterComponent::Server_SetLeanIncremental_Validate(float LeanAmount)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetLeanIncremental_Implementation(float LeanAmount)
{
	IncrementalLeanAngle = LeanAmount;
	
	if (IncrementalLeanAngle > LeanAngle)
	{
		IncrementalLeanAngle = LeanAngle;
	}
	else if (IncrementalLeanAngle < -LeanAngle)
	{
		IncrementalLeanAngle = -LeanAngle;
	}
	
	OnRep_IncrementalLeanAngle();
}

void UFPSTemplate_CharacterComponent::IncrementalLean(ELeaning LeanDirection)
{
	if (!bIsIncrementalLeaning)
	{
		IncrementalLeanAngle = 0.0f;
	}
	switch (LeanDirection)
	{
	case ELeaning::Left: IncrementalLeanAngle -= IncrementalLeanAmount;  break;
	case ELeaning::Right: IncrementalLeanAngle += IncrementalLeanAmount; break;
	case ELeaning::None: ResetLeanAngle();
	}
	
	if (IncrementalLeanAngle > LeanAngle)
	{
		IncrementalLeanAngle = LeanAngle;
	}
	else if (IncrementalLeanAngle < -LeanAngle)
	{
		IncrementalLeanAngle = -LeanAngle;
	}
	
	OnRep_IncrementalLeanAngle();
	if (!HasAuthority())
	{
		Server_SetLeanIncremental(IncrementalLeanAngle);
	}
}

bool UFPSTemplate_CharacterComponent::Server_SetSprinting_Validate(bool IsSprinting)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetSprinting_Implementation(bool IsSprinting)
{
	bIsSprinting = IsSprinting;
}

void UFPSTemplate_CharacterComponent::SetSprinting(bool Sprinting)
{
	bIsSprinting = Sprinting;
	if (!HasAuthority())
	{
		Server_SetSprinting(bIsSprinting);
	}
}

FVector UFPSTemplate_CharacterComponent::GetActorForwardVector() const
{
	if (OwningPawn.IsValid())
	{
		return OwningPawn->GetActorForwardVector();
	}
	return FVector::ZeroVector;
}

FVector UFPSTemplate_CharacterComponent::GetActorRightVector() const
{
	if (OwningPawn.IsValid())
	{
		return OwningPawn->GetActorRightVector();
	}
	return FVector::ZeroVector;
}

void UFPSTemplate_CharacterComponent::RagdollCharacter()
{
	UFPSTemplateStatics::Ragdoll(FPMesh);
	UFPSTemplateStatics::Ragdoll(TPMesh);
}

void UFPSTemplate_CharacterComponent::RagdollCharacterWithForce(const FVector ImpactLocation, const float ImpactForce)
{
	UFPSTemplateStatics::RagdollWithImpact(FPMesh, ImpactLocation, ImpactForce);
	UFPSTemplateStatics::RagdollWithImpact(TPMesh, ImpactLocation, ImpactForce);
}

void UFPSTemplate_CharacterComponent::SetThirdPersonView(bool bThirdPerson)
{
	bInThirdPerson = bThirdPerson;
}

void UFPSTemplate_CharacterComponent::LeanLeft()
{
	bLeanLeftDown = true;
	if (CurrentLean == ELeaning::Right)
	{
		CurrentLean = ELeaning::None;
	}
	else
	{
		CurrentLean = ELeaning::Left;
	}
	bIsIncrementalLeaning = false;
	//IncrementalLeanAngle = 0.0f;
	if (AnimationInstance.IsValid())
	{
		AnimationInstance->SetLeaning(CurrentLean);
	}
	if (!HasAuthority())
	{
		Server_SetLean(CurrentLean);
	}
}

void UFPSTemplate_CharacterComponent::LeanRight()
{
	bLeanRightDown = true;
	if (CurrentLean == ELeaning::Left)
	{
		CurrentLean = ELeaning::None;
	}
	else
	{
		CurrentLean = ELeaning::Right;
	}
	bIsIncrementalLeaning = false;
	//IncrementalLeanAngle = 0.0f;
	if (AnimationInstance.IsValid())
	{
		AnimationInstance->SetLeaning(CurrentLean);
	}
	if (!HasAuthority())
	{
		Server_SetLean(CurrentLean);
	}
}

void UFPSTemplate_CharacterComponent::StopLeanLeft()
{
	bLeanLeftDown = false;
	if (bLeanRightDown)
	{
		CurrentLean = ELeaning::Right;
	}
	else
	{
		CurrentLean = ELeaning::None;
	}
	
	if (AnimationInstance.IsValid())
	{
		AnimationInstance->SetLeaning(CurrentLean);
	}
	if (!HasAuthority())
	{
		Server_SetLean(CurrentLean);
	}
}

void UFPSTemplate_CharacterComponent::StopLeanRight()
{
	bLeanRightDown = false;
	if (bLeanLeftDown)
	{
		CurrentLean = ELeaning::Left;
	}
	else
	{
		CurrentLean = ELeaning::None;
	}
	
	if (AnimationInstance.IsValid())
	{
		AnimationInstance->SetLeaning(CurrentLean);
	}
	if (!HasAuthority())
	{
		Server_SetLean(CurrentLean);
	}
}

bool UFPSTemplate_CharacterComponent::Server_SetLean_Validate(ELeaning Lean)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetLean_Implementation(ELeaning Lean)
{
	CurrentLean = Lean;
	OnRep_Lean();
}

bool UFPSTemplate_CharacterComponent::Server_SetUseLeftHandIK_Validate(bool bUse)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetUseLeftHandIK_Implementation(bool bUse)
{
	SetUseLeftHandIK(bUse);
}

void UFPSTemplate_CharacterComponent::SetUseLeftHandIK(bool bUse)
{
	if (bUseLeftHandIK != bUse)
	{
		bUseLeftHandIK = bUse;
		OnRep_UseLeftHandIK();
		if (!HasAuthority())
		{
			Server_SetUseLeftHandIK(bUseLeftHandIK);
		}
	}
}

void UFPSTemplate_CharacterComponent::PlayCameraShake(TSubclassOf<UCameraShakeBase> CameraShake, float Scale) const
{
	if (OwningPawn.IsValid() && CameraShake)
	{
		if (APlayerController* PC = OwningPawn->GetController<APlayerController>())
		{
			PC->ClientStartCameraShake(CameraShake, Scale);
		}
	}
}

float UFPSTemplate_CharacterComponent::GetMagnificationSensitivity() const
{
	if (bIsAiming)
	{
		if (AimingActor->Implements<UFPSTemplate_AimInterface>())
		{
			return 1.0f / IFPSTemplate_AimInterface::Execute_GetCurrentMagnification(AimingActor);
		}
	}
	return 1.0f;
}

float UFPSTemplate_CharacterComponent::GetMagnificationSensitivityStartValue(float StartAtMagnification) const
{
	if (bIsAiming)
	{
		if (AimingActor->Implements<UFPSTemplate_AimInterface>())
		{
			if (IFPSTemplate_AimInterface::Execute_GetCurrentMagnification(AimingActor) > StartAtMagnification)
			{
				return 1.0f / IFPSTemplate_AimInterface::Execute_GetCurrentMagnification(AimingActor);
			}
		}
	}
	return 1.0f;
}

void UFPSTemplate_CharacterComponent::StartAiming()
{
	if (bFirearmCollisionHitting || !IsValid(AimingActor))
	{
		return;
	}

	StopHighAndLowPortPose();
	
	bIsAiming = true;
	if (AimingActor->Implements<UFPSTemplate_AimInterface>())
	{
		if (IsValid(CurrentFirearm))
		{
			CurrentFirearm->ActivateCurrentSight(true);
		}
		else
		{
			IFPSTemplate_AimInterface::Execute_DisableRenderTargets(AimingActor, false);
		}
	}
	if (AnimationInstance.IsValid())
	{
		AnimationInstance->SetIsAiming(bIsAiming);
	}
	if (!HasAuthority())
	{
		Server_SetAiming(bIsAiming);
	}
}

void UFPSTemplate_CharacterComponent::StopAiming()
{
	if (IsValid(AimingActor))
	{
		bIsAiming = false;
		if (AimingActor->Implements<UFPSTemplate_AimInterface>())
		{
			if (IsValid(CurrentFirearm))
			{
				CurrentFirearm->ActivateCurrentSight(false);
			}
			else
			{
				IFPSTemplate_AimInterface::Execute_DisableRenderTargets(AimingActor, true);
			}
		}
		if (AnimationInstance.IsValid())
		{
			AnimationInstance->SetIsAiming(bIsAiming);
		}
		if (!HasAuthority())
		{
			Server_SetAiming(bIsAiming);
		}
	}
}

bool UFPSTemplate_CharacterComponent::Server_SetAiming_Validate(bool IsAiming)
{
	return true;
}

void UFPSTemplate_CharacterComponent::Server_SetAiming_Implementation(bool IsAiming)
{
	bIsAiming = IsAiming;
	OnRep_IsAiming();
}
