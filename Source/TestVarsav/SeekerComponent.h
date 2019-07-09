// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Components/ActorComponent.h"
#include "SeekerComponent.generated.h"

class AController;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSeeActorDelegate, AActor*, Actor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTVARSAV_API USeekerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USeekerComponent();

	virtual void InitializeComponent() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float MaxSeekDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI)
		float SensingInterval;

	TArray<AActor*> ActorsArray;

protected:
	/** How far to the side AI can see, in degrees. Use SetPeripheralVisionAngle to change the value at runtime. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI)
		float PeripheralVisionAngle;

	/** Cosine of limits of peripheral vision. Computed from PeripheralVisionAngle. */
	UPROPERTY()
		float PeripheralVisionCosine;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = AI)
		virtual void SetSensingInterval(const float NewSensingInterval);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = AI)
		virtual void SetPeripheralVisionAngle(const float NewPeripheralVisionAngle);

	UFUNCTION(BlueprintCallable, Category = AI)
		float GetPeripheralVisionAngle() const;

	UFUNCTION(BlueprintCallable, Category = AI)
		float GetPeripheralVisionCosine() const;

	/**
	 * Chance of seeing other pawn decreases with increasing distance or angle in peripheral vision
	 * @param bMaySkipChecks if true allows checks to be sometimes skipped if target is far away (used by periodic automatic visibility checks)
	 * @return true if the specified pawn Other is potentially visible (within peripheral vision, etc.) - still need to do LineOfSightTo() check to see if actually visible.
	 */
	bool CouldSeeActor(const AActor* Other, bool bMaySkipChecks = false) const;

	/**
	 * Check line to other actor.
	 * @param Other is the actor whose visibility is being checked.
	 * @param ViewPoint is eye position visibility is being checked from.
	 * @return true if controller's pawn can see Other actor.
	 */
	bool HasLineOfSightTo(const AActor* Other) const;

	/** Get position where hearing/seeing occurs (i.e. ear/eye position).  If we ever need different positions for hearing/seeing, we'll deal with that then! */
	FVector GetSensorLocation() const;

	/**  Get the rotation of this sensor. We need this for the sight component */
	FRotator GetSensorRotation() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		uint32 bSeeActors : 1;

	bool ShouldCheckVisibilityOf(AActor* Actor) const;

protected:
	/** See if there are interesting sounds and sights that we want to detect, and respond to them if so. */
	void SenseActor(AActor& Actor);

	AActor* GetSensorActor() const;	// Get the actor used as the actual sensor location is derived from this actor.

	AController* GetSensorController() const; // Get the controller of the sensor actor.

	/** Update function called on timer intervals. */
	void OnTimer();

	/** Handle for efficient management of OnTimer timer */
	FTimerHandle TimerHandle_OnTimer;

	/** Modify the timer to fire in TimeDelay seconds. A value <= 0 disables the timer. */
	void SetTimer(const float TimeDelay);

	/** Calls SensePawn on any Pawns that we are allowed to sense. */
	void UpdateAISensing();

public:
	/** Delegate to execute when we see a Pawn. */
	UPROPERTY(BlueprintAssignable)
		FSeeActorDelegate OnSeeActor;

protected:
	void BroadcastOnSeeActor(AActor& Actor);

	UFUNCTION()
		void SetActorsPositions(AActor* Owner);
};
