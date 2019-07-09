// Fill out your copyright notice in the Description page of Project Settings.


#include "SeekerComponent.h"
#include "EngineGlobals.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "WantedComponent.h"
#include "EngineUtils.h"
#include "Engine.h"

// Sets default values for this component's properties
USeekerComponent::USeekerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	MaxSeekDistance = 5000.f;
	PeripheralVisionAngle = 90.f;
	PeripheralVisionCosine = FMath::Cos(FMath::DegreesToRadians(PeripheralVisionAngle));

	SensingInterval = 5.f;

	bSeeActors = true;
	bWantsInitializeComponent = true;
	bAutoActivate = false;
}

void USeekerComponent::InitializeComponent()
{
	UE_LOG(LogTemp, Warning, TEXT("Initialize Component"));
	Super::InitializeComponent();
	SetPeripheralVisionAngle(PeripheralVisionAngle);

	OnTimer();
}

void USeekerComponent::SetSensingInterval(const float NewSensingInterval)
{
	UE_LOG(LogTemp, Warning, TEXT("Set Sensing Interval"));
	if (SensingInterval != NewSensingInterval && NewSensingInterval > 0.f)
	{
		SensingInterval = NewSensingInterval;

		AActor* const Owner = GetOwner();
		if (IsValid(Owner))
		{
			float CurrentElapsed = Owner->GetWorldTimerManager().GetTimerElapsed(TimerHandle_OnTimer);
			CurrentElapsed = FMath::Max(0.f, CurrentElapsed);

			if (CurrentElapsed < SensingInterval)
			{
				// Extend lifetime by remaining time.
				SetTimer(SensingInterval - CurrentElapsed);
			}
			else if (CurrentElapsed > SensingInterval)
			{
				// Basically fire next update, because time has already expired.
				// Don't want to fire immediately in case an update tries to change the interval, looping endlessly.
				SetTimer(KINDA_SMALL_NUMBER);
			}
		}
	}
}

void USeekerComponent::SetPeripheralVisionAngle(const float NewPeripheralVisionAngle)
{
	UE_LOG(LogTemp, Warning, TEXT("Set Peripheral Vision Angle"));
	PeripheralVisionAngle = NewPeripheralVisionAngle;
	PeripheralVisionCosine = FMath::Cos(FMath::DegreesToRadians(PeripheralVisionAngle));
}

float USeekerComponent::GetPeripheralVisionAngle() const
{
	UE_LOG(LogTemp, Warning, TEXT("Get Peripheral Vision Angle"));
	return PeripheralVisionAngle;
}

float USeekerComponent::GetPeripheralVisionCosine() const
{
	UE_LOG(LogTemp, Warning, TEXT("Get Peripheral Vision Cosine"));
	return PeripheralVisionCosine;
}

void USeekerComponent::SetTimer(const float TimeInterval)
{
	UE_LOG(LogTemp, Warning, TEXT("Set Update Timer"));
	// Only necessary to update if we are the server
	AActor* const Owner = GetOwner();
	if (IsValid(Owner))
	{
		Owner->GetWorldTimerManager().SetTimer(TimerHandle_OnTimer, this, &USeekerComponent::OnTimer, TimeInterval, false);
	}
}

AController* USeekerComponent::GetSensorController() const
{
	UE_LOG(LogTemp, Warning, TEXT("Get Sensor Controller"));
	AActor* SensorActor = GetOwner();
	AController* Controller = Cast<AController>(SensorActor);

	if (IsValid(Controller))
	{
		return Controller;
	}
	else
	{
		APawn* Pawn = Cast<APawn>(SensorActor);
		if (IsValid(Pawn) && IsValid(Pawn->Controller))
		{
			return Pawn->Controller;
		}
	}

	return NULL;
}

bool USeekerComponent::HasLineOfSightTo(const AActor* Other) const
{
	UE_LOG(LogTemp, Warning, TEXT("Has Line Of Sight To"));
	AController* SensorController = GetSensorController();
	if (SensorController == NULL)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed check"));
		return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("Actor"));
	return SensorController->LineOfSightTo(Other, FVector::ZeroVector, true);
}

void USeekerComponent::OnTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("Run Timer"));
	AActor* const Owner = GetOwner();
	if (!IsValid(Owner) || !IsValid(Owner->GetWorld()))
	{
		return;
	}

	UpdateAISensing();

	SetTimer(SensingInterval);
};

void USeekerComponent::UpdateAISensing()
{
	UE_LOG(LogTemp, Warning, TEXT("Update AI Sensing"));
	AActor* const Owner = GetOwner();

	check(IsValid(Owner));
	check(IsValid(Owner->GetWorld()));

	UE_LOG(LogTemp, Warning, TEXT("Remove detection from actors"));
	for (AActor* Actor : ActorsArray)
	{
		Actor->FindComponentByClass<UWantedComponent>()->SetIsDetected(false);
		Actor->FindComponentByClass<UWantedComponent>()->SetIsClosest(false);
		Actor->FindComponentByClass<UWantedComponent>()->SetIsFurthest(false);
	}

	UE_LOG(LogTemp, Warning, TEXT("Clear Contener"));
	ActorsArray.Empty();

	UE_LOG(LogTemp, Warning, TEXT("Detecting Actors"));
	for (FActorIterator Iterator(GetWorld()); Iterator; ++Iterator)
	{
		AActor* Actor = *Iterator;

		if (IsValid(Actor))
		{		
			if (Actor != GetOwner())
			{
				if (Actor->FindComponentByClass<UWantedComponent>())
				{
					SenseActor(*Actor);
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Detected: %d"), ActorsArray.Num());

	SetActorsPositions(Owner);
}

void USeekerComponent::SenseActor(AActor& Actor)
{
	UE_LOG(LogTemp, Warning, TEXT("Sense Actor"));
	// Visibility checks
	bool bHasSeenActor = false;
	bool bHasFailedLineOfSightCheck = false;

	if (bSeeActors && ShouldCheckVisibilityOf(&Actor))
	{
		if (CouldSeeActor(&Actor, true))
		{
			if (HasLineOfSightTo(&Actor))
			{
				BroadcastOnSeeActor(Actor);
				bHasSeenActor = true;

				UE_LOG(LogTemp, Warning, TEXT("Set Actor detection"));
				Actor.FindComponentByClass<UWantedComponent>()->SetIsDetected(true);

				UE_LOG(LogTemp, Warning, TEXT("Add Actor to Contener"));
				ActorsArray.Add(&Actor);
			}
			else
			{
				bHasFailedLineOfSightCheck = true;
			}
		}
	}
}

AActor* USeekerComponent::GetSensorActor() const
{
	UE_LOG(LogTemp, Warning, TEXT("Get Sensor"));
	AActor* SensorActor = GetOwner();
	AController* Controller = Cast<AController>(SensorActor);

	if (!IsValid(SensorActor))
	{
		return NULL;
	}

	return SensorActor;
}

bool USeekerComponent::CouldSeeActor(const AActor* Other, bool bMaySkipChecks) const
{
	UE_LOG(LogTemp, Warning, TEXT("Could See Actor"));
	if (!Other)
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldn't see - not other actor"));
		return false;
	}

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldn't see - no owner"));
		return false;
	}

	FVector const OtherLoc = Other->GetActorLocation();
	FVector const SensorLoc = GetSensorLocation();
	FVector const SelfToOther = OtherLoc - SensorLoc;

	// check max sight distance
	float const SelfToOtherDistSquared = SelfToOther.SizeSquared();
	if (SelfToOtherDistSquared > FMath::Square(MaxSeekDistance))
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldn't see - too far"));
		return false;
	}

	// may skip if more than some fraction of maxdist away (longer time to acquire)
	if (bMaySkipChecks && (FMath::Square(FMath::FRand()) * SelfToOtherDistSquared > FMath::Square(0.4f * MaxSeekDistance)))
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldn't see - too long to acquire"));
		return false;
	}

	// 	UE_LOG(LogPath, Warning, TEXT("DistanceToOtherSquared = %f, SightRadiusSquared: %f"), SelfToOtherDistSquared, FMath::Square(MaxSeekDistance));

		// check field of view
	FVector const SelfToOtherDir = SelfToOther.GetSafeNormal();
	FVector const MyFacingDir = GetSensorRotation().Vector();

	// 	UE_LOG(LogPath, Warning, TEXT("DotProductFacing: %f, PeripheralVisionCosine: %f"), SelfToOtherDir | MyFacingDir, PeripheralVisionCosine);
	UE_LOG(LogTemp, Warning, TEXT("Actor can be seen"));
	return ((SelfToOtherDir | MyFacingDir) >= PeripheralVisionCosine);
}

FVector USeekerComponent::GetSensorLocation() const
{
	UE_LOG(LogTemp, Warning, TEXT("Get Sensor Location"));
	FVector SensorLocation(FVector::ZeroVector);
	const AActor* SensorActor = GetSensorActor();

	if (SensorActor != NULL)
	{
		FRotator ViewRotation;
		SensorActor->GetActorEyesViewPoint(SensorLocation, ViewRotation);
	}

	return SensorLocation;
}

FRotator USeekerComponent::GetSensorRotation() const
{
	UE_LOG(LogTemp, Warning, TEXT("Get Sensor Rotation"));
	FRotator SensorRotation(FRotator::ZeroRotator);
	const AActor* SensorActor = GetOwner();

	if (SensorActor != NULL)
	{
		SensorRotation = SensorActor->GetActorRotation();
	}

	return SensorRotation;
}

bool USeekerComponent::ShouldCheckVisibilityOf(AActor* Actor) const
{
	UE_LOG(LogTemp, Warning, TEXT("Should Check Visibility"));
	if (!bSeeActors)
	{
		return false;
	}

	if (Actor->bHidden)
	{
		return false;
	}

	return true;
}

void USeekerComponent::BroadcastOnSeeActor(AActor& Actor)
{
	UE_LOG(LogTemp, Warning, TEXT("Broadcast OnSeeActor"));
	OnSeeActor.Broadcast(&Actor);
}

void USeekerComponent::SetActorsPositions(AActor* Owner)
{
	if (ActorsArray.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sort Container"));
		ActorsArray.HeapSort([Owner](const AActor& A, const AActor& B)
			{
				return A.GetDistanceTo(Owner) < B.GetDistanceTo(Owner);
			});

		UE_LOG(LogTemp, Warning, TEXT("Set Closest and Farthest"));

		ActorsArray[0]->FindComponentByClass<UWantedComponent>()->SetIsClosest(true);
		ActorsArray[ActorsArray.Num() - 1]->FindComponentByClass<UWantedComponent>()->SetIsFurthest(true);
	
		for (AActor* Actor : ActorsArray)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("%s - Distance to seeker: %f - Number %d to seeker"), (*Actor->GetName()), Actor->GetDistanceTo(Owner), ActorsArray.IndexOfByKey(Actor) + 1));
			}
		}
	}
}