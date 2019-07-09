// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WantedComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTVARSAV_API UWantedComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWantedComponent();

protected:
	UPROPERTY()
		bool isDetected;

	UPROPERTY()
		bool isClosest;

	UPROPERTY()
		bool isFurthest;

public:
	UFUNCTION()
		void SetIsDetected(bool detection);

	UFUNCTION()
		bool GetIsDetected();

	UFUNCTION()
		void SetIsClosest(bool position);

	UFUNCTION()
		bool GetIsClosest();

	UFUNCTION()
		void SetIsFurthest(bool position);

	UFUNCTION()
		bool GetIsFurthest();
};
