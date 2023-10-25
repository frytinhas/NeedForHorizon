// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CarSystem/Code/NFHBaseVehicle.h"
#include "PoliceSystem/Code/NFHPoliceAIController.h"
#include "NFHPoliceVehicle.generated.h"

/**
 * 
 */
UCLASS()
class NEEDFORHORIZON_API ANFHPoliceVehicle : public ANFHBaseVehicle
{
	GENERATED_BODY()

	ANFHPoliceVehicle();

// Variables
public:
	UPROPERTY(BlueprintReadOnly, Category = "AI|Police System")
	ANFHPoliceAIController* PoliceAIController = nullptr;

// Functions
	virtual void BeginPlay() override;
};
