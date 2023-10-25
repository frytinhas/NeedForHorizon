// Fill out your copyright notice in the Description page of Project Settings.


#include "PoliceSystem/BT/Tasks/NFHBTT_FollowPlayer.h"
#include "CarSystem/Code/NFHPoliceVehicle.h"
#include "CarSystem/Code/NFHBaseVehicle.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UNFHBTT_FollowPlayer::UNFHBTT_FollowPlayer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Follow Player";
	bTickIntervals = false;
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UNFHBTT_FollowPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    //* Get police car reference
    PoliceCarReference = Cast<ANFHPoliceVehicle>(OwnerComp.GetAIOwner()->GetPawn());
    //* Get player car reference
    Target = Cast<ANFHBaseVehicle>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));
    //? Verify if car movement is valid
    if (PoliceCarReference->NFHVehicleComponent && Target->NFHVehicleComponent)
    {
        // Finish task if valid
        return EBTNodeResult::InProgress;
    }
    else
    {
        // Finish task if invalid
        return EBTNodeResult::Failed;
    }
}

void UNFHBTT_FollowPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    //? Execute "SetPoliceThrottleInput" and verify if the function has success
    if (SetPoliceThrottleInput())
    {
        // "SetPoliceCarSteeringInput" if "SetPoliceThrottleInput" was success
        SetPoliceSteeringInput();
        //* UKismetSystemLibrary::PrintString(this, TEXT("True"));
        // End task with success
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
    else
    {
        // Set police car steering input in 0.f if "SetPoliceThrottleInput" was unsuccessful
        PoliceCarReference->NFHVehicleComponent->SetSteeringInput(0.f);
        //* UKismetSystemLibrary::PrintString(this, TEXT("False"));
        // End task with failed
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    }
}

bool UNFHBTT_FollowPlayer::SetPoliceThrottleInput() const
{
    // Get player movement component
    const UChaosWheeledVehicleMovementComponent* PlayerMovementComponent = Target->NFHVehicleComponent;    
    //? Verify if player has movement component
    if (PlayerMovementComponent)
    {
        // Verify if police car is near of player car
        if (PoliceCarReference->GetDistanceTo(Target) > 50.f || Target->GetCarHealth() != 0.f)
        {
            // Declare variables to exec line trace
            const TArray<AActor*> ActorsToIgnore = { PoliceCarReference };
            FHitResult HitResult;
            //? Line trace to verify if not have nothing in forward of the car
            if (!UKismetSystemLibrary::LineTraceSingle(this, PoliceCarReference->GetMesh()->GetSocketLocation("CarForward"),
            PoliceCarReference->GetMesh()->GetSocketRotation("CarForward").Vector() * 80.f +
            PoliceCarReference->GetMesh()->GetSocketLocation("CarForward"),
            TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true) ||
            FVector::Distance(Target->GetMesh()->GetSocketLocation(Target->NFHVehicleComponent->WheelSetups[0].BoneName), PoliceCarReference->GetActorLocation()) <
            FVector::Distance(Target->GetMesh()->GetSocketLocation(Target->NFHVehicleComponent->WheelSetups[3].BoneName), PoliceCarReference->GetActorLocation()))
            {
                // Get throttle input
                // Get current police car velocity
                const float PoliceCarVelocity = FMath::Abs(PoliceCarReference->NFHVehicleComponent->GetForwardSpeedMPH());
                // Get current player car velocity
                const float PlayerCarVelocity = FMath::Abs(PlayerMovementComponent->GetForwardSpeedMPH());
                // Get maximum speed that the car can reach
                const float MaxVelocity = PlayerCarVelocity > 40.f ? PlayerCarVelocity + 15.f : FMath::Clamp(Target->GetDistanceTo(PoliceCarReference), 15.f, 400.f);
                //? Verify if current police car velocity is > current car velocity
                const bool bALessB = PoliceCarVelocity < MaxVelocity;
                //* Get throttle input to apply (Verifying if if current police car velocity is > current car velocity, true = 1.f and false = 0.f)
                const float ThrottleInput = bALessB ? 1.f : 0.f;

                // Set brake input
                PoliceCarReference->NFHVehicleComponent->SetBrakeInput(0.f);
            
                // Set throttle input
                PoliceCarReference->NFHVehicleComponent->SetThrottleInput(ThrottleInput);

                // End function and warning if success or not
                return bALessB;
            }
            // Execute brake input
            PoliceCarReference->NFHVehicleComponent->SetBrakeInput(1.f);
            return true;
        }
        // Set brake input
        PoliceCarReference->NFHVehicleComponent->SetBrakeInput(0.f);
        
        // Set throttle input
        PoliceCarReference->NFHVehicleComponent->SetThrottleInput(0.f);

        // End function and warning if success or not
        return false;
    }
    // Set throttle input in 0.f
    PoliceCarReference->NFHVehicleComponent->SetThrottleInput(0.f);

    // End function with false result
    return false;
}

void UNFHBTT_FollowPlayer::SetPoliceSteeringInput() const
{
    // Get current player location
    const FVector TargetLocation = Target->GetActorLocation();

    //* Set steering amount
        // Get distance between player and wheels (FR and FL)
        const float DistanceBetweenPlayerAndFL = FVector::Dist(TargetLocation, PoliceCarReference->GetMesh()->GetSocketLocation(PoliceCarReference->NFHVehicleComponent->WheelSetups[0].BoneName));
        const float DistanceBetweenPlayerAndFR = FVector::Dist(TargetLocation, PoliceCarReference->GetMesh()->GetSocketLocation(PoliceCarReference->NFHVehicleComponent->WheelSetups[1].BoneName));
        //! Get target steering (this value is very important)
        const float TargetSteering = PoliceCarReference->NFHVehicleComponent->GetBrakeInput() == 0.f ?
            (DistanceBetweenPlayerAndFL > DistanceBetweenPlayerAndFR ? 1.f : -1.f) : DistanceBetweenPlayerAndFL > DistanceBetweenPlayerAndFR ? -1.f : 1.f;
        // Get steering quantity to apply
        const float SteeringAmount = FMath::Abs(DistanceBetweenPlayerAndFL - DistanceBetweenPlayerAndFR) < 20.f ? 0.f : 1.f;  

    // Set steering input
    PoliceCarReference->NFHVehicleComponent->SetSteeringInput(TargetSteering * SteeringAmount);
}