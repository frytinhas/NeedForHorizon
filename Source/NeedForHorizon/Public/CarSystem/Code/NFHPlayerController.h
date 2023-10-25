// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NFHPlayerController.generated.h"

/**
 *
 */
UCLASS()
class NEEDFORHORIZON_API ANFHPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ANFHPlayerController();

	virtual void BeginPlay() override;
};
