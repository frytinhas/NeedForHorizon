// Fill out your copyright notice in the Description page of Project Settings.


#include "CarSystem/Code/NFHPlayerVehicle.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

ANFHPlayerVehicle::ANFHPlayerVehicle()
{
// Construct physics thruster
	ThrusterComponent = CreateDefaultSubobject<UPhysicsThrusterComponent>("ThrusterComponent");
	// Attach thruster component in mesh
	ThrusterComponent->SetupAttachment(GetMesh(), "ThrustersEffect");
	// Define Thruster Component variables
	ThrusterComponent->ThrustStrength = 800000.f;

// Construct niagara thruster
	ThrustersEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ThrustersEffect"));
	ThrustersEffect->bAutoActivate = false;
	// Attach thrusters in mesh
	ThrustersEffect->SetupAttachment(GetMesh(), "ThrustersEffect");
}

void ANFHPlayerVehicle::BeginPlay()
{
	Super::BeginPlay();
	
	//? Verify if need apply thruster capacity immediately
	if (bRechargeTheThrustersImmediately)
	{	
		// Set thruster capacity in max
		ThrusterCapacity = MaxThrusterCapacity;
	}
	else
	{
		//* Start increase thruster capacity
		IncreaseThrusterCapacity();
	}
}

void ANFHPlayerVehicle::ExplodeVehicle()
{
	Super::ExplodeVehicle();
	
	//* Disable player input
	DisableInput(CarPlayerController);
	ActivateThrusters(false);
}


void ANFHPlayerVehicle::ActivateThrusters(const bool bEnable)
{
	//? Verify if need enable or disable thrusters
	if (bEnable)
	{
		//? Verify if have thruster capacity to enable thrusters
		if (ThrusterCapacity > 35.f)
		{
		  // Enable thruster
			ThrusterComponent->Activate();
			//! Force car to accelerate
			NFHVehicleComponent->SetThrottleInput(1.f);
			//! Force car to stop brake
			NFHVehicleComponent->SetBrakeInput(0.f);
			//* Stop increase thruster capacity
			GetWorldTimerManager().ClearTimer(IncreaseThrusterTimerHandle);
			//* Start decrease thruster capacity
			DecreaseThrusterCapacity();
		}
		//* End function if not have thruster capacity
		else
		{
			return;
		}
	}
	else
	{
	  // Disable thruster
		ThrusterComponent->Deactivate();
		NFHVehicleComponent->SetThrottleInput(0.f);
		//* Stop decrease thruster capacity
		GetWorldTimerManager().ClearTimer(DecreaseThrusterTimerHandle);
		//* Start increase thruster capacity
		IncreaseThrusterCapacity();
	}

		// Reset thrusters effect
		ThrustersEffect->ResetSystem();
		// Set if is visible or not
		ThrustersEffect->SetVisibility(bEnable);
		//* Call OnActiveThrusters event dispatcher to warning others algorithms
		OnActiveThrusters.Broadcast(bEnable);
}

void ANFHPlayerVehicle::DecreaseThrusterCapacity()
{
	//? Verify if thruster capacity is > 0
	if(ThrusterCapacity > 0.f)
	{
		// Decrease thruster capacity with smoothing
		ThrusterCapacity = UKismetMathLibrary::FInterpTo_Constant(ThrusterCapacity, 0.f, UGameplayStatics::GetWorldDeltaSeconds(this), VelocityToDecreaseThrusterCapacity);
		//! Repeat function after 0.05s
		GetWorldTimerManager().SetTimer(DecreaseThrusterTimerHandle, this, &ANFHPlayerVehicle::DecreaseThrusterCapacity, 0.05f);
	}
	else
	{
		//* Deactive thrusters
		ActivateThrusters(false);
	}
}

void ANFHPlayerVehicle::IncreaseThrusterCapacity()
{
	//? Verify if thruster capacity is not equal max thruster capacity
	if(ThrusterCapacity != MaxThrusterCapacity)
	{
		// Get speed to increase thruster capacity
		const float IncreaseMultiplier = (NFHVehicleComponent->GetThrottleInput()) == 0.f ? VelocityMultiplierToIncreaseThruserCapacity : VelocityMultiplierToIncreaseThruserCapacity / 1.5f;
		const float IncreaseSpeed = (NFHVehicleComponent->GetForwardSpeed()) > 10.f ? VelocityToIncreaseThrusterCapacity * IncreaseMultiplier / 1.5f : 
		VelocityToIncreaseThrusterCapacity * IncreaseMultiplier;

		// Increase thruster capacity with smoothing
		ThrusterCapacity = UKismetMathLibrary::FInterpTo_Constant(ThrusterCapacity, MaxThrusterCapacity, UGameplayStatics::GetWorldDeltaSeconds(this), IncreaseSpeed);
		//! Repeat function after 0.05s
		GetWorldTimerManager().SetTimer(IncreaseThrusterTimerHandle, this, &ANFHPlayerVehicle::IncreaseThrusterCapacity, 0.05f, false);
	}
}
