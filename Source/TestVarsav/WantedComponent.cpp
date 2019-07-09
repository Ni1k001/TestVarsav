// Fill out your copyright notice in the Description page of Project Settings.


#include "WantedComponent.h"

// Sets default values for this component's properties
UWantedComponent::UWantedComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	isDetected = false;
	isClosest = false;
	isFurthest = false;
}

void UWantedComponent::SetIsDetected(bool detection)
{
	isDetected = detection;
}

bool UWantedComponent::GetIsDetected()
{
	return isDetected;
}

void UWantedComponent::SetIsClosest(bool position)
{
	isClosest = position;

	if (isClosest)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - I'm the closest"), *(GetOwner()->GetName()));
	}
}

bool UWantedComponent::GetIsClosest()
{
	return isClosest;
}

void UWantedComponent::SetIsFurthest(bool position)
{
	isFurthest = position;

	if (isFurthest)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - I'm the furthest"), *(GetOwner()->GetName()));
	}
}

bool UWantedComponent::GetIsFurthest()
{
	return isFurthest;
}