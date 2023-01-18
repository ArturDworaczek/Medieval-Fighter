// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MedievalFighterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimInstance.h"

//////////////////////////////////////////////////////////////////////////
// AMedievalFighterCharacter
//////////////////////////////////////////////////////////////////////////
AMedievalFighterCharacter::AMedievalFighterCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 350.f;
	GetCharacterMovement()->AirControl = 0.01f;
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	DefaultSpeed = GetCharacterMovement()->MaxWalkSpeed;
	SpeedIncrease = 50.0f;
	SpeedDecrease = 50.0f;

	// Health system
	Health = 100.0f;

	// Create a first person camera
	FPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FPCamera->SetupAttachment(GetMesh(), TEXT("Head"));
	FPCamera->SetRelativeLocation(FVector(1.0f, 20.0f, 3.0f));
	FPCamera->SetRelativeRotation(FRotator(0.0f, 90.0f, -90.0f));

	
	// Create first person
	GetMesh()->SetOnlyOwnerSee(true);
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	FPWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon Mesh"));
	FPWeaponMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
	FPWeaponMesh->SetOnlyOwnerSee(true);
	FPWeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));

	// Create third person
	TPMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Mesh"));
	TPMesh->SetupAttachment(GetRootComponent());
	TPMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -97.0f));
	TPMesh->SetRelativeRotation(FRotator(0.0f, -90.000717f, 0.0f));
	TPMesh->SetOwnerNoSee(true);
	TPMesh->SetCollisionProfileName(TEXT("BlockAll"));

	TPWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Third Person Weapon Mesh"));
	TPWeaponMesh->SetupAttachment(TPMesh, TEXT("hand_r"));
	TPWeaponMesh->SetOwnerNoSee(true);
	TPWeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	TPWeaponMesh->OnComponentBeginOverlap.AddDynamic(this, &AMedievalFighterCharacter::WeaponMeshOverlap);

	// Load weapon meshes
	ConstructorHelpers::FObjectFinder<UStaticMesh>DaggerAssetFound(TEXT("StaticMesh'/Game/Player/WeaponPlaceholders/KnifeViking.KnifeViking'"));
	DaggerAsset = DaggerAssetFound.Object;
	ConstructorHelpers::FObjectFinder<UAnimMontage>DaggerAttackMontageFound(TEXT("AnimMontage'/Game/Player/Mannequin/Animations/Dagger/FPP_Dag_AttackLSlash_Montage.FPP_Dag_AttackLSlash_Montage'"));
	DaggerAttackMontage = DaggerAttackMontageFound.Object;
	ConstructorHelpers::FObjectFinder<UAnimMontage>DaggerDamageMontageFound(TEXT("AnimMontage'/Game/Player/Mannequin/Animations/Dagger/FPP_Dag_HitC_Montage.FPP_Dag_HitC_Montage'"));
	DaggerDamageMontage = DaggerDamageMontageFound.Object;

	ConstructorHelpers::FObjectFinder<UStaticMesh>HalberdAssetFound(TEXT("StaticMesh'/Game/Player/WeaponPlaceholders/BerdyszViking.BerdyszViking'"));
	HalberdAsset = HalberdAssetFound.Object;
	ConstructorHelpers::FObjectFinder<UAnimMontage>HalberdAttackMontageFound(TEXT("AnimMontage'/Game/Player/Mannequin/Animations/Halberd/FPP_Halb_Attack_D2_Montage.FPP_Halb_Attack_D2_Montage'"));
	HalberdAttackMontage = HalberdAttackMontageFound.Object;
	ConstructorHelpers::FObjectFinder<UAnimMontage>HalberdDamageMontageFound(TEXT("AnimMontage'/Game/Player/Mannequin/Animations/Halberd/FPP_Halb_Hit1_Montage.FPP_Halb_Hit1_Montage'"));
	HalberdDamageMontage = HalberdDamageMontageFound.Object;

	ConstructorHelpers::FObjectFinder<UStaticMesh>LongSwordAssetFound(TEXT("StaticMesh'/Game/Player/WeaponPlaceholders/Longsword.Longsword'"));
	LongSwordAsset = LongSwordAssetFound.Object;
	ConstructorHelpers::FObjectFinder<UAnimMontage>LongSwordAttackMontageFound(TEXT("AnimMontage'/Game/Player/Mannequin/Animations/Longsword/FPP_Longs_Attack_R_Montage.FPP_Longs_Attack_R_Montage'"));
	LongSwordAttackMontage = LongSwordAttackMontageFound.Object;
	ConstructorHelpers::FObjectFinder<UAnimMontage>LongSwordDamageMontageFound(TEXT("AnimMontage'/Game/Player/Mannequin/Animations/Longsword/FPP_Longs_Hit1_Montage.FPP_Longs_Hit1_Montage'"));
	LongSwordDamageMontage = LongSwordDamageMontageFound.Object;

	ConstructorHelpers::FObjectFinder<UStaticMesh>SpearAssetFound(TEXT("StaticMesh'/Game/Player/WeaponPlaceholders/SpearViking.SpearViking'"));
	SpearAsset = SpearAssetFound.Object;
	ConstructorHelpers::FObjectFinder<UAnimMontage>SpearAttackMontageFound(TEXT("AnimMontage'/Game/Player/Mannequin/Animations/Spear/FPPSpear_Attack1_Montage.FPPSpear_Attack1_Montage'"));
	SpearAttackMontage = SpearAttackMontageFound.Object;
	ConstructorHelpers::FObjectFinder<UAnimMontage>SpearDamageMontageFound(TEXT("AnimMontage'/Game/Player/Mannequin/Animations/Spear/FPPSpear_Hit1_Montage.FPPSpear_Hit1_Montage'"));
	SpearDamageMontage = SpearDamageMontageFound.Object;
}

//////////////////////////////////////////////////////////////////////////
// Multiplayer Variable Replication
//////////////////////////////////////////////////////////////////////////
void AMedievalFighterCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(AMedievalFighterCharacter, bAttacking);
	DOREPLIFETIME(AMedievalFighterCharacter, bSprinting);
	DOREPLIFETIME(AMedievalFighterCharacter, DefaultSpeed);
	DOREPLIFETIME(AMedievalFighterCharacter, HitPlayersArray);
	DOREPLIFETIME(AMedievalFighterCharacter, Health);
}

//////////////////////////////////////////////////////////////////////////
// Input
//////////////////////////////////////////////////////////////////////////
void AMedievalFighterCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Action Inputs
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMedievalFighterCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMedievalFighterCharacter::StopSprinting);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMedievalFighterCharacter::Attack);

	// Axis Inputs
	PlayerInputComponent->BindAxis("MoveForward", this, &AMedievalFighterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMedievalFighterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

//////////////////////////////////////////////////////////////////////////
// Axis Inputs
//////////////////////////////////////////////////////////////////////////
void AMedievalFighterCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}
void AMedievalFighterCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

//////////////////////////////////////////////////////////////////////////
// Multiplayer Gameplay
//////////////////////////////////////////////////////////////////////////
void AMedievalFighterCharacter::SetWeapon(EWeapons WeaponToSet)
{
	if (!bAttacking) {
		SetWeaponServer(WeaponToSet);

		switch (WeaponToSet) {
			case EWeapons::W_NoWeapon:
			{
				FPWeaponMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
				FPWeaponMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
				FPWeaponMesh->SetStaticMesh(NULL);
				FPWeaponMesh->SetVisibility(false, false);
			}
				break;
			case EWeapons::W_Dagger:
			{
				FPWeaponMesh->SetRelativeLocation(FVector(-8.781104f, 4.826352f, 0.126374f));
				FPWeaponMesh->SetRelativeRotation(FRotator(4.980621f, 81.317833f, 119.621643f));
				FPWeaponMesh->SetStaticMesh(DaggerAsset);
				FPWeaponMesh->SetVisibility(true, false);
			}
				break;
			case EWeapons::W_Halberd:
			{
				FPWeaponMesh->SetRelativeLocation(FVector(12.110796f, 5.220458f, -28.740444f));
				FPWeaponMesh->SetRelativeRotation(FRotator(-9.99996f, -97.999985f, -124.999985f));
				FPWeaponMesh->SetStaticMesh(HalberdAsset);
				FPWeaponMesh->SetVisibility(true, false);
			}
				break;
			case EWeapons::W_Longsword:
			{
				FPWeaponMesh->SetRelativeLocation(FVector(-8.573628f, 5.381995f, 0.508609f));
				FPWeaponMesh->SetRelativeRotation(FRotator(0.000067f, 75.0f, -47.0f));
				FPWeaponMesh->SetStaticMesh(LongSwordAsset);
				FPWeaponMesh->SetVisibility(true, false);
			}
				break;
			case EWeapons::W_Spear:
			{
				FPWeaponMesh->SetRelativeLocation(FVector(-12.062593f, 5.173286f, 1.960684f));
				FPWeaponMesh->SetRelativeRotation(FRotator(2.0f, 82.0f, -43.0f));
				FPWeaponMesh->SetStaticMesh(SpearAsset);
				FPWeaponMesh->SetVisibility(true, false);
			}
				break;
			default:
				FPWeaponMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
				FPWeaponMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
				FPWeaponMesh->SetStaticMesh(NULL);
				FPWeaponMesh->SetVisibility(false, false);
		}
	}
}
void AMedievalFighterCharacter::SetWeaponServer_Implementation(EWeapons WeaponToSet)
{
	SetWeaponMulticast(WeaponToSet);
}
bool AMedievalFighterCharacter::SetWeaponServer_Validate(EWeapons WeaponToSet)
{
	return true;
}
void AMedievalFighterCharacter::SetWeaponMulticast_Implementation(EWeapons WeaponToSet)
{
	ActiveWeapon = WeaponToSet;

	switch (WeaponToSet) {
		case EWeapons::W_NoWeapon:
		{
			TPWeaponMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			TPWeaponMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
			TPWeaponMesh->SetStaticMesh(NULL);
			TPWeaponMesh->SetVisibility(false, false);
		}
			break;
		case EWeapons::W_Dagger:
		{
			TPWeaponMesh->SetRelativeLocation(FVector(-8.781104f, 4.826352f, 0.126374f));
			TPWeaponMesh->SetRelativeRotation(FRotator(4.980621f, 81.317833f, 119.621643f));
			TPWeaponMesh->SetStaticMesh(DaggerAsset);
			TPWeaponMesh->SetVisibility(true, false);
		}
			break;
		case EWeapons::W_Halberd:
		{
			TPWeaponMesh->SetRelativeLocation(FVector(12.110796f, 5.220458f, -28.740444f));
			TPWeaponMesh->SetRelativeRotation(FRotator(-9.99996f, -97.999985f, -124.999985f));
			TPWeaponMesh->SetStaticMesh(HalberdAsset);
			TPWeaponMesh->SetVisibility(true, false);
		}
			break;
		case EWeapons::W_Longsword:
		{
			TPWeaponMesh->SetRelativeLocation(FVector(-8.573628f, 5.381995f, 0.508609f));
			TPWeaponMesh->SetRelativeRotation(FRotator(0.000067f, 75.0f, -47.0f));
			TPWeaponMesh->SetStaticMesh(LongSwordAsset);
			TPWeaponMesh->SetVisibility(true, false);
		}
			break;
		case EWeapons::W_Spear:
		{
			TPWeaponMesh->SetRelativeLocation(FVector(-12.062593f, 5.173286f, 1.960684f));
			TPWeaponMesh->SetRelativeRotation(FRotator(2.0f, 82.0f, -43.0f));
			TPWeaponMesh->SetStaticMesh(SpearAsset);
			TPWeaponMesh->SetVisibility(true, false);
		}
			break;
		default:
			TPWeaponMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			TPWeaponMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
			TPWeaponMesh->SetStaticMesh(NULL);
			TPWeaponMesh->SetVisibility(false, false);
	}
}
//////////////////////////////////////////////////////////////////////////
// Attack
//////////////////////////////////////////////////////////////////////////
void AMedievalFighterCharacter::Attack()
{
	if (!bAttacking) {
		AttackServer();

		switch (ActiveWeapon) {
			case EWeapons::W_Dagger:
			{
				GetMesh()->GetAnimInstance()->Montage_Play(DaggerAttackMontage);
			}
				break;
			case EWeapons::W_Halberd:
			{
				GetMesh()->GetAnimInstance()->Montage_Play(HalberdAttackMontage);
			}
				break;
			case EWeapons::W_Longsword:
			{
				GetMesh()->GetAnimInstance()->Montage_Play(LongSwordAttackMontage);
			}
				break;
			case EWeapons::W_Spear:
			{
				GetMesh()->GetAnimInstance()->Montage_Play(SpearAttackMontage);
			}
				break;
		}
	}
}
void AMedievalFighterCharacter::AttackServer_Implementation() 
{
	AttackMulticast();
}
bool AMedievalFighterCharacter::AttackServer_Validate() 
{
	if (GetLocalRole() == ROLE_Authority && !bAttacking)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void AMedievalFighterCharacter::AttackMulticast_Implementation() 
{
	TPWeaponMesh->SetCollisionProfileName(TEXT("OverlapAll"));
	bAttacking = true;

	switch (ActiveWeapon) {
		case EWeapons::W_Dagger:
		{
			TPMesh->GetAnimInstance()->Montage_Play(DaggerAttackMontage);
		}
			break;
		case EWeapons::W_Halberd:
		{
			TPMesh->GetAnimInstance()->Montage_Play(HalberdAttackMontage);
		}
			break;
		case EWeapons::W_Longsword:
		{
			TPMesh->GetAnimInstance()->Montage_Play(LongSwordAttackMontage);
		}
			break;
		case EWeapons::W_Spear:
		{
			TPMesh->GetAnimInstance()->Montage_Play(SpearAttackMontage);
		}
			break;
	}
}
void AMedievalFighterCharacter::AttackResetServer_Implementation()
{
	AttackResetMulticast();
}
bool AMedievalFighterCharacter::AttackResetServer_Validate()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void AMedievalFighterCharacter::AttackResetMulticast_Implementation()
{
	bAttacking = false;
	TPWeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	HitPlayersArray.Empty();
}
//////////////////////////////////////////////////////////////////////////
// Damage System
//////////////////////////////////////////////////////////////////////////
void AMedievalFighterCharacter::WeaponMeshOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMedievalFighterCharacter* HitPlayer = Cast<AMedievalFighterCharacter>(OtherActor);

	if (HitPlayer != nullptr) {
		if (OtherActor != this) {
			HitPlayerServer(HitPlayer);
		}
	}
}
void AMedievalFighterCharacter::HitPlayerServer_Implementation(AMedievalFighterCharacter* PlayerHit)
{
	HitPlayerMulticast(PlayerHit);
}
bool AMedievalFighterCharacter::HitPlayerServer_Validate(AMedievalFighterCharacter* PlayerHit)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void AMedievalFighterCharacter::HitPlayerMulticast_Implementation(AMedievalFighterCharacter* PlayerHit)
{
	if (HitPlayersArray.Num() > 0) {
		for (int32 Index = 0; Index < HitPlayersArray.Num(); Index++) {
			if (PlayerHit == HitPlayersArray[Index]) {
				break;
			}
			else if (Index >= HitPlayersArray.Num()) {
				HitPlayersArray.Add(PlayerHit);
				PlayerHit->TakeDamage(25.0f);
			}
		}
	}
	else {
		HitPlayersArray.Add(PlayerHit);
		PlayerHit->TakeDamage(25.0f);
	}
}
void AMedievalFighterCharacter::TakeDamage(float Damage)
{
	Health -= Damage;

	switch (ActiveWeapon) {
		case EWeapons::W_Dagger:
		{
			GetMesh()->GetAnimInstance()->Montage_Play(DaggerDamageMontage);
			TPMesh->GetAnimInstance()->Montage_Play(DaggerDamageMontage);
		}
			break;
		case EWeapons::W_Halberd:
		{
			GetMesh()->GetAnimInstance()->Montage_Play(HalberdDamageMontage);
			TPMesh->GetAnimInstance()->Montage_Play(HalberdDamageMontage);
		}
			break;
		case EWeapons::W_Longsword:
		{
			GetMesh()->GetAnimInstance()->Montage_Play(LongSwordDamageMontage);
			TPMesh->GetAnimInstance()->Montage_Play(LongSwordDamageMontage);
		}
			break;
		case EWeapons::W_Spear:
		{
			GetMesh()->GetAnimInstance()->Montage_Play(SpearDamageMontage);
			TPMesh->GetAnimInstance()->Montage_Play(SpearDamageMontage);
		}
			break;
	}
}
//////////////////////////////////////////////////////////////////////////
// Movement
//////////////////////////////////////////////////////////////////////////
// Sprinting Functions
void AMedievalFighterCharacter::Sprint()
{
	float Speed = FVector::DotProduct(GetVelocity(), GetActorRotation().Vector());
	if (Speed > 0.0f)
	{
		SetbSprinting(true);
		GetWorld()->GetTimerManager().ClearTimer(DeSprintTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(InSprintTimerHandle, this, &AMedievalFighterCharacter::IncreaseSpeed, 0.1f, true);
	}
}
void AMedievalFighterCharacter::StopSprinting()
{
	SetbSprinting(false);
	GetWorld()->GetTimerManager().ClearTimer(InSprintTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(DeSprintTimerHandle, this, &AMedievalFighterCharacter::DecreaseSpeed, 0.2f, true);
}
// Set default speed (Multicast)
void AMedievalFighterCharacter::SetDefaultSpeedM_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
}
// Set sprinting bool (Server)
void AMedievalFighterCharacter::SetbSprinting_Implementation(bool ToSet)
{
	bSprinting = ToSet;
}
bool AMedievalFighterCharacter::SetbSprinting_Validate(bool ToSet)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		return true;
	}
	else
	{
		return false;
	}
}
// Increase speed (Server/Multicast)
void AMedievalFighterCharacter::IncreaseSpeed_Implementation()
{
	float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
	float Speed = FVector::DotProduct(GetVelocity(), GetActorRotation().Vector());

	if (Speed > 50.0f)
	{
		if ((CurrentSpeed < 600.0f) && (bSprinting == true))
		{
			GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed + SpeedIncrease;
			IncreaseSpeedM(CurrentSpeed);
		}
	}
	else
	{
		StopSprinting();
	}
}
bool AMedievalFighterCharacter::IncreaseSpeed_Validate()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void AMedievalFighterCharacter::IncreaseSpeedM_Implementation(float CurrentSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed + SpeedIncrease;
}
// Decrease speed (Server/Multicast)
void AMedievalFighterCharacter::DecreaseSpeed_Implementation()
{
	float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;

	if ((CurrentSpeed > 300.0f) && (bSprinting == false))
	{
		GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed - SpeedDecrease;
		DecreaseSpeedM(CurrentSpeed);

		float Speed = FVector::DotProduct(GetVelocity(), GetActorRotation().Vector());
		if (Speed < 50.0f)
		{
			GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
			SetDefaultSpeedM();
		}
	}
}
bool AMedievalFighterCharacter::DecreaseSpeed_Validate()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void AMedievalFighterCharacter::DecreaseSpeedM_Implementation(float CurrentSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed - SpeedDecrease;
}