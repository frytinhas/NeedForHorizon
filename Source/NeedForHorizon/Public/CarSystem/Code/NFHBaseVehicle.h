// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "NFHBaseVehicle.generated.h"

//* Forward declarations
class UChaosWheeledVehicleMovementComponent;

UCLASS()
class NEEDFORHORIZON_API ANFHBaseVehicle : public AWheeledVehiclePawn
{
	GENERATED_BODY()

// Components
public:
	UPROPERTY(Category = "Vehicle", BlueprintReadOnly)
		TObjectPtr<UChaosWheeledVehicleMovementComponent> NFHVehicleComponent;

protected:
	UPROPERTY(Category = "Vehicle", EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAudioComponent> EngineSound;

// Variables
public:
	// Get the car player controller
	UPROPERTY(Category = "Player Info", BlueprintReadOnly)
	APlayerController* CarPlayerController;
	
protected:
	// Current max health (Use "GetMaxCarHealth" method to get this in Blueprints or in others .h/.cpp)
	UPROPERTY(Category = "Systems|Car System|Durability System", EditDefaultsOnly)
	float MaxCarHealth = 500.f;

	// Car health (Use "GetCarHealth" method to get this in Blueprints or in others .h/.cpp)
	UPROPERTY(Category = "Systems|Car System|Durability System", VisibleDefaultsOnly)
	float CarHealth;

private:
	// Actors hited variables
	UPROPERTY()
	TArray<AActor*> ActorsHited;

	FTimerHandle ActorsHitedTimerHandle;
	
	
// Functions
public:
	ANFHBaseVehicle();

private:
	virtual float TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator,
		AActor* DamageCauser) override;

	// UFunctions
public:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void ExplodeVehicle();
	
	// Get current car health
	UFUNCTION(Category = "Systems|Car System|Durability System", BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetCarHealth() const
	{
		return CarHealth;
	}

	// Get max car health
	UFUNCTION(Category = "Systems|Car System|Durability System", BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetMaxCarHealth() const
	{
		return MaxCarHealth;
	}

	// Override "IsPlayerControlled" function
	virtual bool IsPlayerControlled() const override;
	
	// On car hit in others things.
	UFUNCTION(Category = "Systems|Car System|Durability System", BlueprintNativeEvent)
	void OnCarHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
};
