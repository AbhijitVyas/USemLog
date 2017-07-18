// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLRawDataLogger.h"
#include "SLEventDataLogger.h"
#include "SLRuntimeManager.generated.h"

UCLASS()
class SEMLOG_API ASLRuntimeManager : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLRuntimeManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Get raw data logger
	USLRawDataLogger* GetRawDataLogger() { return RawDataLogger; };

	// Add finished event
	bool AddFinishedEvent(
		const FString Namespace,
		const FString Class,
		const FString Id,		
		const float StartTime,
		const float EndTime,
		const TArray<FOwlTriple>& Properties = TArray<FOwlTriple>());
	
	// Add finished event, Name = Class + Id
	bool AddFinishedEvent(
		const FString Namespace,
		const FString Name,
		const float StartTime,
		const float EndTime,
		const TArray<FOwlTriple>& Properties = TArray<FOwlTriple>());

	// Add finished event, FullName = Namespace + Class + Id
	bool AddFinishedEvent(
		const FString FullName,
		const float StartTime,
		const float EndTime,
		const TArray<FOwlTriple>& Properties = TArray<FOwlTriple>());

	// Start an event
	bool StartEvent(
		const FString Namespace,
		const FString Class,
		const FString Id,
		const float StartTime,
		const TArray<FOwlTriple>& Properties = TArray<FOwlTriple>());

	// Start an event, Name = Class + Id
	bool StartEvent(
		const FString Namespace,
		const FString Name,
		const float StartTime,
		const TArray<FOwlTriple>& Properties = TArray<FOwlTriple>());

	// Start an event, FullName = Namespace + Class + Id
	bool StartEvent(
		const FString FullName,
		const float StartTime,
		const TArray<FOwlTriple>& Properties = TArray<FOwlTriple>());

	// Start an event
	bool FinishEvent(
		const FString Namespace,
		const FString Class,
		const FString Id,
		const float EndTime,
		const TArray<FOwlTriple>& Properties = TArray<FOwlTriple>());

	// Start an event, Name = Class + Id
	bool FinishEvent(
		const FString Namespace,
		const FString Name,
		const float EndTime,
		const TArray<FOwlTriple>& Properties = TArray<FOwlTriple>());

	// Start an event, FullName = Namespace + Class + Id
	bool FinishEvent(
		const FString FullName,
		const float EndTime,
		const TArray<FOwlTriple>& Properties = TArray<FOwlTriple>());

private:
	// Episode Id (be default will be auto generated)
	UPROPERTY(EditAnywhere, Category = "SL")
	FString EpisodeId;

	// Log directory
	UPROPERTY(EditAnywhere, Category = "SL")
	FString LogDirectory;

	// Log raw data
	UPROPERTY(EditAnywhere, Category = "SL")
	bool bLogRawData;

	// Update rate in seconds (if 0.f or smaller than the Tick's DeltaTime, it will update every tick)
	UPROPERTY(EditAnywhere, Category = "SL", meta = (editcondition = "bLogRawData"), meta = (ClampMin = 0))
	float RawDataUpdateRate;

	// Write data to file
	UPROPERTY(EditAnywhere, Category = "SL", meta = (editcondition = "bLogRawData"))
	bool bWriteRawDataToFile;

	// Publish raw data as event
	UPROPERTY(EditAnywhere, Category = "SL", meta = (editcondition = "bLogRawData"))
	bool bBroadcastRawData;

	// Raw data logger
	UPROPERTY(EditAnywhere, Category = "SL", meta = (editcondition = "bLogRawData"))
	USLRawDataLogger* RawDataLogger;

	// Log semantic events
	UPROPERTY(EditAnywhere, Category = "SL")
	bool bLogEventData;
	
	// Event data logger
	UPROPERTY(EditAnywhere, Category = "SL", meta = (editcondition = "bLogEventData"))
	USLEventDataLogger* EventDataLogger;

	// Amount of time passed since last update (used for the raw data update rate)
	float TimePassedSinceLastUpdate;
};
