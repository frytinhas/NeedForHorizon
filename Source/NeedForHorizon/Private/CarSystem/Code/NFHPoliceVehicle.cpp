// Fill out your copyright notice in the Description page of Project Settings.


#include "CarSystem/Code/NFHPoliceVehicle.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

ANFHPoliceVehicle::ANFHPoliceVehicle()
{
	// Define basic AI variables (I recommend not changing them)
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ANFHPoliceAIController::StaticClass();
}

void ANFHPoliceVehicle::BeginPlay()
{
	Super::BeginPlay();
	// Define police ai controller
	PoliceAIController = Cast<ANFHPoliceAIController>(UAIBlueprintHelperLibrary::GetAIController(this));
}

