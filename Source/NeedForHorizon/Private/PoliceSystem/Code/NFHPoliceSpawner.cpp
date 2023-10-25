// Fill out your copyright notice in the Description page of Project Settings.


#include "PoliceSystem/Code/NFHPoliceSpawner.h"
#include "NavigationSystem.h"
#include "CarSystem/Code/NFHPoliceVehicle.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavMesh/NavMeshBoundsVolume.h"

DECLARE_LOG_CATEGORY_EXTERN(PoliceSpawner, Log, All);

DEFINE_LOG_CATEGORY(PoliceSpawner);

// Sets default values
ANFHPoliceSpawner::ANFHPoliceSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ANFHPoliceSpawner::BeginPlay()
{
	Super::BeginPlay();
	// Get player vehicle
	PlayerVehicle = Cast<ANFHPlayerVehicle>(UGameplayStatics::GetPlayerPawn(this, 0));
	//? Verify if player vehicle and nav mesh is valid, if not valid destroy actor
	if (!PlayerVehicle || !UGameplayStatics::GetActorOfClass(this, ANavMeshBoundsVolume::StaticClass()))
	{
		UE_LOG(PoliceSpawner, Warning, TEXT("Player vehicle or nav mesh don't exist in map"));
		K2_DestroyActor();
	}
	
	// Update police vehicles
	UpdatePoliceVehicles();
}

// Called every frame
void ANFHPoliceSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANFHPoliceSpawner::UpdatePoliceVehicles()
{
	//? Verify if police vehicle class is null and throw a warning (exception is donkey thing)
	if (PoliceVehicleClass == nullptr)
	{
		UE_LOG(PoliceSpawner, Warning, TEXT("Police vehicle class is null"));
		return;
	}
	
	// Get all police vehicles in scene
	TArray<AActor*> AllPoliceVehiclesArray;
	UGameplayStatics::GetAllActorsOfClass(this, ANFHPoliceVehicle::StaticClass(), AllPoliceVehiclesArray);
	//? Verify if has 1 or > police vehicles
	if (AllPoliceVehiclesArray.Num() > 0)
	{
		// Declare filtered police vehicles variable
		TArray<int> FilteredPoliceVehicles;
		// Filter just police vehicles working only
		for (AActor* CurrentActor : AllPoliceVehiclesArray)
		{
			//? If distance between police car and player vehicle is > max distance and car health is not equal 0, destroy police car
			if (CurrentActor->GetDistanceTo(PlayerVehicle) > MaxPoliceCarDistance || Cast<ANFHPoliceVehicle>(CurrentActor)->GetCarHealth() == 0.f)
			{
				UE_LOG(PoliceSpawner, Display, TEXT("Police vehicle %s was destroyed, Distance is: %f, and Health is: %d"), *CurrentActor->GetName(),
					CurrentActor->GetDistanceTo(PlayerVehicle), int(Cast<ANFHPoliceVehicle>(CurrentActor)->GetCarHealth()));
				CurrentActor->K2_DestroyActor();
			}
			else
			{
				// Add to filtered police vehicles
				FilteredPoliceVehicles.Add(0);
			}
		}
	
		//? Verify if have low police vehicles number
		if (FilteredPoliceVehicles.Num() < MaxPoliceVehicles || FilteredPoliceVehicles.IsEmpty())
		{
			// Declare spawn variables
			FVector LocationToSpawn;
			const FRotator RotationToSpawn = UKismetMathLibrary::FindLookAtRotation(LocationToSpawn, PlayerVehicle->GetActorLocation());

			//? Verify if has valid random location to spawn and set "LocationToSpawn" value
			if (UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this, PlayerVehicle->GetActorLocation(), LocationToSpawn,
				MaxPoliceCarDistance - 500.f) && LocationToSpawn != FVector(0.f, 0.f, 0.f))
			{
				// Spawn actor and display info
				GetWorld()->SpawnActor<ANFHPoliceVehicle>(ANFHPoliceVehicle::StaticClass(), LocationToSpawn + FVector(0.f, 0.f, 2.f),
					FRotator(0.f, RotationToSpawn.Yaw, 0.f));
				UE_LOG(PoliceSpawner, Display, TEXT("Spawn with success police vehicle in location: %s"), *LocationToSpawn.ToString());
			}
			else
			{
				// Warning to log if not found random point to spawn
				UE_LOG(PoliceSpawner, Warning, TEXT("Failed to spawn police car"));
			}
		}
		else {/* Warning to log if not spawn by max police car num */ UE_LOG(PoliceSpawner, Display, TEXT("Max police car num"));}
	}
	else
	{
		// Declare spawn variables
		FVector LocationToSpawn;
		const FRotator RotationToSpawn = UKismetMathLibrary::FindLookAtRotation(LocationToSpawn, PlayerVehicle->GetActorLocation());

		//? Verify if has valid random location to spawn and set "LocationToSpawn" value
		if (UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this, PlayerVehicle->GetActorLocation(), LocationToSpawn,
			MaxPoliceCarDistance - 500.f) && LocationToSpawn != FVector(0.f, 0.f, 0.f))
		{
			// Spawn actor and display info
			GetWorld()->SpawnActor<ANFHPoliceVehicle>(ANFHPoliceVehicle::StaticClass(), LocationToSpawn + FVector(0.f, 0.f, 2.f),
				FRotator(0.f, RotationToSpawn.Yaw, 0.f));
			UE_LOG(PoliceSpawner, Display, TEXT("Spawn with success police vehicle in location: %s"), *LocationToSpawn.ToString());
		}
		else
		{
			// Warning to log if not found random point to spawn
			UE_LOG(PoliceSpawner, Warning, TEXT("Failed to spawn police car"));
		}
	}
	// Repeat function after delay
	GetWorldTimerManager().SetTimer(UpdatePoliceVehiclesHandle, this, &ANFHPoliceSpawner::UpdatePoliceVehicles, DelayBetweenVerifications);
}

