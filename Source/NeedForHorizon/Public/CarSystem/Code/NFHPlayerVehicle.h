// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CarSystem/Code/NFHBaseVehicle.h"
#include "NiagaraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "PhysicsEngine/PhysicsThrusterComponent.h"
#include "NFHPlayerVehicle.generated.h"

// Event Dispatchers declaration
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActiveThrustersDelegate, bool, bThrustActive);

// Class declaration
UCLASS()
class NEEDFORHORIZON_API ANFHPlayerVehicle : public ANFHBaseVehicle
{
	GENERATED_BODY()

	ANFHPlayerVehicle();

// Components
protected:
	UPROPERTY(Category = "Systems|Car System|Thruster System", EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UNiagaraComponent> ThrustersEffect;

	UPROPERTY(Category = "Systems|Car System|Thruster System", EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UPhysicsThrusterComponent> ThrusterComponent;

// Variables
	/* Max thruster capacity.
	Use "GetMaxThrustersCapacity" method to get this in Blueprints or in others .h/.cpp). */
	UPROPERTY(Category = "Systems|Car System|Thruster System", EditDefaultsOnly)
	float MaxThrusterCapacity = 100.f;

	// If checked, it recharges the thrusters instantly when starting. If unchecked, it recharges the thrusters at normal speed when starting.
	UPROPERTY(Category = "Systems|Car System|Thruster System", EditDefaultsOnly)
	bool bRechargeTheThrustersImmediately = true;

	/* Current thruster capacity. (If you are in Blueprints, know you only can edit this in C++.
	Use "GetThrustersCapacity" method to get this in Blueprints or in others .h/.cpp). */
	UPROPERTY(Category = "Systems|Car System|Thruster System", VisibleDefaultsOnly)
	float ThrusterCapacity = 0.f;

	// Velocity to decrease thruster capacity after activate.
	UPROPERTY(Category = "Systems|Car System|Thruster System", EditDefaultsOnly)
	float VelocityToDecreaseThrusterCapacity = 100.f;

	// Velocity to increase thruster capacity after deactivate.
	UPROPERTY(Category = "Systems|Car System|Thruster System", EditDefaultsOnly)
	float VelocityToIncreaseThrusterCapacity = 100.f;

	/* The VelocityMultiplierToIncreaseThruserCapacity has a few factors, it is responsible for increasing the recharge speed of
	the thrusters if some factors are valid, such as not having ThrusterInput or being at a low/stopped speed. */
	UPROPERTY(Category = "Systems|Car System|Thruster System", EditDefaultsOnly)
	float VelocityMultiplierToIncreaseThruserCapacity = 3.f;

private:
	FTimerHandle DecreaseThrusterTimerHandle, IncreaseThrusterTimerHandle;

// Event Dispatchers variables
public:
	// Whenever the thrusters have any changes, this is called.
	UPROPERTY(BlueprintAssignable, Category = "Systems|Car System|Thruster System")
	FActiveThrustersDelegate OnActiveThrusters;

// Functions
	// UFunctions
		virtual void BeginPlay() override;

		virtual void ExplodeVehicle() override;
	
		// Get max thruster capacity.
		UFUNCTION(Category = "Systems|Car System|Thruster System", BlueprintCallable, BlueprintPure)
		FORCEINLINE float GetMaxThrustersCapacity() const
		{
			return MaxThrusterCapacity;
		}

		// Get current thruster capacity
		UFUNCTION(Category = "Systems|Car System|Thruster System", BlueprintCallable, BlueprintPure)
		FORCEINLINE float GetThrustersCapacity() const
		{
			return ThrusterCapacity;
		}

		// Verify if thrusters are enableds.
		UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Systems|Car System|Thruster System")
		FORCEINLINE bool ThrustersEnabled() const
		{
			//? Verify if have thrusters
			return ThrusterComponent->IsActive();
		}

		// Activate thrusters (Just use this function to modify thrusters, modifying thrusters by other methods can result in many bugs)
		UFUNCTION(BlueprintCallable, Category = "Systems|Car System|Thruster System")
		void ActivateThrusters(const bool bEnable);

	// CPP Functions
		void DecreaseThrusterCapacity();

		void IncreaseThrusterCapacity();
};
