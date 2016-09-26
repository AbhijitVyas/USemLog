// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "SLEventsExporter.h"
#include "SLDrawerStates.generated.h"

/**
*  Actor that logs drawer states
*/
UCLASS()
class SEMLOG_API ASLDrawerStates : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLDrawerStates();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Close drawers
	void CloseDrawers();

	// Check drawer states
	void CheckDrawerStates();

	// Log state
	void LogState(AActor* Furniture, const FString State);

	// Drawer state check update rate (seconds)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float UpdateRate;

	// Array of constraints to watch
	TArray<UPhysicsConstraintComponent*> Constraints;

	// Furniture state timer handle
	FTimerHandle FurnitureStateTimerHandle;

	// Close furniture timer handle (call after a delay, apply impulse to close drawers)
	FTimerHandle CloseFurnitureTimerHandle;

	// Drawer actor initial position
	TMap<AActor*, FVector> DrawerToInitLocMap;

	// Door actor min and max position
	TMap<AActor*, TPair<float, float> > DoorToMinMaxMap;

	// Actor to state map
	TMap<AActor*, FString> FurnitureToStateMap;

	// Pointer to the semantic events exporter
	FSLEventsExporter* SemEventsExporter;
};
