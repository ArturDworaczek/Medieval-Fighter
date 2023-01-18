// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MedievalFighterCharacter.generated.h"

UENUM(BlueprintType)
enum EWeapons
{
	W_NoWeapon			UMETA(DisplayName = "No Weapon"),
	W_Dagger			UMETA(DisplayName = "Dagger"),
	W_Halberd			UMETA(DisplayName = "Halberd"),
	W_Longsword			UMETA(DisplayName = "Longsword"),
	W_Spear				UMETA(DisplayName = "Spear"),
	W_Sword_and_Shield	UMETA(DisplayName = "Sword and Shield")
};

UCLASS(config=Game)
class AMedievalFighterCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "First Person", meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FPCamera;
	/** First person weapon mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "First Person", meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* FPWeaponMesh;

	/** Third person skeletal mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Third Person", meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* TPMesh;
	/** Third person weapon mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Third Person", meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* TPWeaponMesh;
public:
	AMedievalFighterCharacter();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Gameplay", meta = (AllowPrivateAccess = "true"))
		TEnumAsByte<EWeapons> ActiveWeapon;
protected:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Timer handles
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FTimerHandle JumpTimerHandle;
	FTimerHandle InSprintTimerHandle;
	FTimerHandle DeSprintTimerHandle;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Mesh
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	UStaticMesh* DaggerAsset;
	UAnimMontage* DaggerAttackMontage;
	UAnimMontage* DaggerDamageMontage;

	UStaticMesh* HalberdAsset;
	UAnimMontage* HalberdAttackMontage;
	UAnimMontage* HalberdDamageMontage;

	UStaticMesh* LongSwordAsset;
	UAnimMontage* LongSwordAttackMontage;
	UAnimMontage* LongSwordDamageMontage;

	UStaticMesh* SpearAsset;
	UAnimMontage* SpearAttackMontage;
	UAnimMontage* SpearDamageMontage;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// NOT REPLICATED VARIABLES
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
		float SpeedIncrease;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
		float SpeedDecrease;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// REPLICATED VARIABLES
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Multiplayer Combat")
		float Health;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Multiplayer Movement")
		float DefaultSpeed;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Multiplayer Movement")
		bool bSprinting;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Multiplayer Movement")
		bool bAttacking;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Multiplayer Combat")
		TArray<AMedievalFighterCharacter*> HitPlayersArray;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Axis Inputs
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/** Called for forwards/backward input */
	void MoveForward(float Value);
	/** Called for side to side input */
	void MoveRight(float Value);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Action Inputs
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/** Sprinting */
	/** Called To Sprint */
	void Sprint();
	/** Called To Stop Sprinting */
	void StopSprinting();

	/** Called to set weapon */
	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gameplay")
		void SetWeapon(EWeapons WeaponToSet);

	/** Called To Attack */
	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gameplay")
		void Attack();
	/** Called when weapon mesh is overlapped */
	UFUNCTION(Category = "Combat")
		void WeaponMeshOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Takes damage */
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void TakeDamage(float Damage);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Network
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/** Attack (Server) */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Multiplayer Gameplay")
		void AttackServer();
		void AttackServer_Implementation();
		bool AttackServer_Validate();
	/** Attack (Multicast) */
	UFUNCTION(NetMulticast, Reliable, Category = "Multiplayer Gameplay")
		void AttackMulticast();
		void AttackMulticast_Implementation();
	/** Attack Reset (Server) */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Multiplayer Gameplay")
		void AttackResetServer();
		void AttackResetServer_Implementation();
		bool AttackResetServer_Validate();
	/** Attack Reset (Multicast) */
	UFUNCTION(NetMulticast, Reliable, Category = "Multiplayer Gameplay")
		void AttackResetMulticast();
		void AttackResetMulticast_Implementation();
	/** Hit Player (Server) */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Multiplayer Gameplay")
		void HitPlayerServer(AMedievalFighterCharacter* PlayerHit);
		void HitPlayerServer_Implementation(AMedievalFighterCharacter* PlayerHit);
		bool HitPlayerServer_Validate(AMedievalFighterCharacter* PlayerHit);
	/** Hit Player (Multicast) */
	UFUNCTION(NetMulticast, Reliable, Category = "Multiplayer Gameplay")
		void HitPlayerMulticast(AMedievalFighterCharacter* PlayerHit);
		void HitPlayerMulticast_Implementation(AMedievalFighterCharacter* PlayerHit);

	/** Set weapon (Server) */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Multiplayer Gameplay")
		void SetWeaponServer(EWeapons WeaponToSet);
		void SetWeaponServer_Implementation(EWeapons WeaponToSet);
		bool SetWeaponServer_Validate(EWeapons WeaponToSet);
	/** Set weapon (Multicast) */
	UFUNCTION(NetMulticast, Reliable, Category = "Multiplayer Gameplay")
		void SetWeaponMulticast(EWeapons WeaponToSet);
		void SetWeaponMulticast_Implementation(EWeapons WeaponToSet);

	/** Set sprinting bool (Server) */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Multiplayer Movement Functions")
		void SetbSprinting(bool ToSet);
		void SetbSprinting_Implementation(bool ToSet);
		bool SetbSprinting_Validate(bool ToSet);
	/** Increase speed (Server) */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Multiplayer Movement Functions")
		void IncreaseSpeed();
		void IncreaseSpeed_Implementation();
		bool IncreaseSpeed_Validate();
	/** Increase speed (Server) */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Multiplayer Movement Functions")
		void DecreaseSpeed();
		void DecreaseSpeed_Implementation();
		bool DecreaseSpeed_Validate();
	/** Set default speed (Multicast) */
	UFUNCTION(Client, Reliable, Category = "Multiplayer Movement Functions")
		void SetDefaultSpeedM();
		void SetDefaultSpeedM_Implementation();
	/** Increase speed (Multicast) */
	UFUNCTION(NetMulticast, Reliable, Category = "Multiplayer Movement Functions")
		void IncreaseSpeedM(float CurrentSpeed);
		void IncreaseSpeedM_Implementation(float CurrentSpeed);
	/** Decrease speed (Multicast) */
	UFUNCTION(NetMulticast, Reliable, Category = "Multiplayer Movement Functions")
		void DecreaseSpeedM(float CurrentSpeed);
		void DecreaseSpeedM_Implementation(float CurrentSpeed);
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns first person camera subobject **/
	FORCEINLINE class UCameraComponent* GetFPCamera() const { return FPCamera; }
	/** Returns first person weapon mesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetFPWeaponMesh() const { return FPWeaponMesh; }
	/** Returns third person weapon mesh subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetTPMesh() const { return TPMesh; }
	/** Returns third person weapon mesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetTPWeaponMesh() const { return TPWeaponMesh; }
};

