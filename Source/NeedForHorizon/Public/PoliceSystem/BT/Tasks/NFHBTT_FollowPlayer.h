// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "NFHBTT_FollowPlayer.generated.h"

// Forward declarations
class ANFHPoliceVehicle;
class ANFHBaseVehicle;

UCLASS()
class NEEDFORHORIZON_API UNFHBTT_FollowPlayer : public UBTTaskNode
{
	GENERATED_BODY()

// Base constructor
public:
	UNFHBTT_FollowPlayer(const FObjectInitializer& ObjectInitializer);

// Functions
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// Return true if success, and false if failed.
	bool SetPoliceThrottleInput() const;

	void SetPoliceSteeringInput() const;

// Variables
	UPROPERTY()
	ANFHPoliceVehicle* PoliceCarReference;

	UPROPERTY(Category = "Info", EditAnywhere)
	FBlackboardKeySelector TargetKey;

	UPROPERTY()
	ANFHBaseVehicle* Target;
};
