// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "NFHPlayerCameraManager.generated.h"

class ANFHPlayerVehicle;

UCLASS()
class NEEDFORHORIZON_API ANFHPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	ANFHPlayerCameraManager();

public:
// Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
	ANFHPlayerVehicle* PlayerPawnRef;

	UPROPERTY(VisibleDefaultsOnly ,BlueprintReadOnly, Category = "Systems|Car System|Thruster System")
	bool bThrustersEnabled;

// Functions
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, Category = "Systems|Car System|Thruster System")
	void OnPlayerActiveThrusters(const bool bEnable);
};
