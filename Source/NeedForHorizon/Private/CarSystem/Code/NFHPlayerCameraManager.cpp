// Fill out your copyright notice in the Description page of Project Settings.

#include "CarSystem/Code/NFHPlayerCameraManager.h"
#include "CarSystem/Code/NFHPlayerVehicle.h"

ANFHPlayerCameraManager::ANFHPlayerCameraManager()
{

}

void ANFHPlayerCameraManager::BeginPlay()
{
	Super::BeginPlay();
	// Cast to player vehicle by player pawn
	PlayerPawnRef = Cast<ANFHPlayerVehicle>(GetOwningPlayerController()->GetPawn());
	// If player vehicle is valid, bind event to on active thrusters
	if(PlayerPawnRef)
	{
		PlayerPawnRef->OnActiveThrusters.AddDynamic(this, &ANFHPlayerCameraManager::OnPlayerActiveThrusters);
	}
}

// Implementation if not have blueprint implementation
void ANFHPlayerCameraManager::OnPlayerActiveThrusters_Implementation(const bool bEnable)
{
	bThrustersEnabled = bEnable;
}
