// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLPouringEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Owl/SLOwlExperimentStatics.h"

// Constructor with initialization
FSLPouringEvent::FSLPouringEvent(const FString& InId, float InStart, float InEnd, uint64 InPairId,
	USLBaseIndividual* InIndividual1,
	USLBaseIndividual* InIndividual2) :
	ISLEvent(InId, InStart, InEnd),
	PairId(InPairId),
	Individual1(InIndividual1), 
	Individual2(InIndividual2)
{
}

// Constructor initialization without end time
FSLPouringEvent::FSLPouringEvent(const FString& InId, float InStart, uint64 InPairId,
	USLBaseIndividual* InIndividual1,
	USLBaseIndividual* InIndividual2) :
	ISLEvent(InId, InStart),
	PairId(InPairId),
	Individual1(InIndividual1),
	Individual2(InIndividual2)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLPouringEvent::ToOwlNode() const
{
	// Create the Pouring event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "PouringSituation");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", StartTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", EndTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInEpisodeProperty("log", EpisodeId));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLPouringEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
{
	// Add timepoint individuals
	// We know that the document is of type FOwlExperiment,
	// we cannot use the safer dynamic_cast because RTTI is not enabled by default
	// if (FOwlEvents* EventsDoc = dynamic_cast<FOwlEvents*>(OutDoc))
	FSLOwlExperiment* EventsDoc = static_cast<FSLOwlExperiment*>(OutDoc);
	EventsDoc->RegisterTimepoint(StartTime);
	EventsDoc->RegisterTimepoint(EndTime);
	EventsDoc->RegisterObject(Individual1);
	EventsDoc->RegisterObject(Individual2);
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLPouringEvent::Context() const
{
	return FString::Printf(TEXT("Pouring - %lld"), PairId);
}

// Get the tooltip data
FString FSLPouringEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'O1\',\'%s\',\'Id\',\'%s\',\'O2\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*Individual1->GetClassValue(), *Individual1->GetIdValue(), *Individual2->GetClassValue(), *Individual2->GetIdValue(), *Id);
}

// Get the data as string
FString FSLPouringEvent::ToString() const
{
	return FString::Printf(TEXT("Individual1:[%s] Individual2:[%s] PairId:%lld"),
		*Individual1->GetInfo(), *Individual2->GetInfo(), PairId);
}
/* End ISLEvent interface */