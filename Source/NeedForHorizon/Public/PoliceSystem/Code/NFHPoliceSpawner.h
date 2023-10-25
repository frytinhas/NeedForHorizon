// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CarSystem/Code/NFHPlayerVehicle.h"
#include "GameFramework/Actor.h"
#include "NFHPoliceSpawner.generated.h"

UCLASS()
class NEEDFORHORIZON_API ANFHPoliceSpawner : public AActor
{
	GENERATED_BODY()

// Functions
public:	
	// Base constructor
	ANFHPoliceSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	void UpdatePoliceVehicles();

// Variables
protected:
	UPROPERTY(Category = "References", BlueprintReadOnly)
	ANFHPlayerVehicle* PlayerVehicle = nullptr;

private:
	UPROPERTY(Category = "Police Spawner Info", EditAnywhere)
	TSubclassOf<ANFHBaseVehicle> PoliceVehicleClass;

	UPROPERTY(Category = "Police Spawner Info", EditAnywhere, meta = (UIMin = 1000.f, ClampMin = 1000.f))
	float MaxPoliceCarDistance = 1500.f;
	
	UPROPERTY(Category = "Police Spawner Info", EditAnywhere)
	int MaxPoliceVehicles = 5;

	UPROPERTY(Category = "Police Spawner Info", EditAnywhere, meta = (ClampMin = 0.1f, UIMin = 0.1f))
	float DelayBetweenVerifications = 1.f;

	FTimerHandle UpdatePoliceVehiclesHandle;
};
