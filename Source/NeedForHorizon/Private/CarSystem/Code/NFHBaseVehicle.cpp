// Fill out your copyright notice in the Description page of Project Settings.


#include "CarSystem/Code/NFHBaseVehicle.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "NiagaraDataInterfaceEmitterBinding.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Kismet/KismetMathLibrary.h"

ANFHBaseVehicle::ANFHBaseVehicle()
{
	// Construct components
		// Cast to UChaosWheeledVehicleMovementComponent
		NFHVehicleComponent = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

		// Construct sound component
		EngineSound = CreateDefaultSubobject<UAudioComponent>("Engine Sound");
		EngineSound->SetupAttachment(GetMesh(), "COMSocket");
	
	// Mesh config
		// Set vehicle simulate physics
		GetMesh()->SetSimulatePhysics(true);

	// Vehicle component initial variables
		// Set vehicle mass
		NFHVehicleComponent->Mass = 1500.f;
		NFHVehicleComponent->bEnableCenterOfMassOverride = true;

		// Setup Wheels
		FChaosWheelSetup ConstructorWheelConfig;
		ConstructorWheelConfig.BoneName = "FL";
		NFHVehicleComponent->WheelSetups.Add(ConstructorWheelConfig);
		ConstructorWheelConfig.BoneName = "FR";
		NFHVehicleComponent->WheelSetups.Add(ConstructorWheelConfig);
		ConstructorWheelConfig.BoneName = "RL";
		NFHVehicleComponent->WheelSetups.Add(ConstructorWheelConfig);
		ConstructorWheelConfig.BoneName = "RR";
		NFHVehicleComponent->WheelSetups.Add(ConstructorWheelConfig);

	// Bind event to on car mesh hit other component
		GetMesh()->OnComponentHit.AddDynamic(this, &ANFHBaseVehicle::OnCarHit);
}

float ANFHBaseVehicle::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	// Clamp value
	CarHealth = FMath::Clamp(CarHealth - ActualDamage, 0.f, MaxCarHealth);
	
	//? Verify if car explode
	if (CarHealth == 0.f)
	{
		ExplodeVehicle();
	}
	
	return ActualDamage;
}

void ANFHBaseVehicle::BeginPlay()
{
	Super::BeginPlay();
	//? Destroy car if mesh is not valid
	if (!GetMesh())
	{
		K2_DestroyActor();
	}
	// Set car player controller (if have one)
	CarPlayerController = Cast<APlayerController>(GetController());
	// Set car health
	CarHealth = MaxCarHealth;
}

void ANFHBaseVehicle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Set current RPM in engine audio component
	EngineSound->SetFloatParameter("CurrentRPM", NFHVehicleComponent->GetForwardSpeedMPH());
}

void ANFHBaseVehicle::ExplodeVehicle()
{
	// Disable engine torque
	NFHVehicleComponent->SetMaxEngineTorque(0.f);
	// Launch car to up
	GetMesh()->AddImpulseToAllBodiesBelow(GetActorUpVector() * 1000.f);
}

bool ANFHBaseVehicle::IsPlayerControlled() const
{
	return IsValid(CarPlayerController);
}

void ANFHBaseVehicle::OnCarHit_Implementation(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!ActorsHited.Contains(OtherActor))
	{
		// Apply damage to the car that crashed
		UGameplayStatics::ApplyDamage(OtherActor, IsPlayerControlled() ? NFHVehicleComponent->GetForwardSpeedMPH() * 2.f :
			FMath::Clamp(NFHVehicleComponent->GetForwardSpeedMPH(),1.f, 150.f),nullptr, this, UDamageType::StaticClass());
		// Apply damage to this car
		UGameplayStatics::ApplyDamage(this, IsPlayerControlled() ? NFHVehicleComponent->GetForwardSpeedMPH() / 1.5f :
			NFHVehicleComponent->GetForwardSpeedMPH() * 1.5f,nullptr, nullptr, UDamageType::StaticClass());
		// Add other actor to actors hited list
		ActorsHited.AddUnique(OtherActor);
		// Set timer to clear hited actors
		GetWorldTimerManager().SetTimer(ActorsHitedTimerHandle,
	[this]
		{
			ActorsHited.Empty();
		},
		2.f, false);
	}
}
