// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLSlideEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Owl/SLOwlExperimentStatics.h"

// Constructor with initialization
FSLSlideEvent::FSLSlideEvent(const FString& InId, const float InStart, const float InEnd, const uint64 InPairId,
	USLBaseIndividual* InManipulator, USLBaseIndividual* InIndividual) :
	ISLEvent(InId, InStart, InEnd), PairId(InPairId), Manipulator(InManipulator), Individual(InIndividual)
{
}

// Constructor initialization without end time
FSLSlideEvent::FSLSlideEvent(const FString& InId, const float InStart, const uint64 InPairId,
	USLBaseIndividual* InIndividual, USLBaseIndividual* InManipulator) :
	ISLEvent(InId, InStart), PairId(InPairId), Manipulator(InManipulator), Individual(InIndividual)
{
}

/* Begin ISLEvent interface */
// Get an owl representation of the event
FSLOwlNode FSLSlideEvent::ToOwlNode() const
{
	// Create the Slide event node
	FSLOwlNode EventIndividual = FSLOwlExperimentStatics::CreateEventIndividual(
		"log", Id, "SlidingSituation");
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateStartTimeProperty("log", StartTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateEndTimeProperty("log", EndTime));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreatePerformedByProperty("log", Manipulator->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateObjectActedOnProperty("log", Individual->GetIdValue()));
	EventIndividual.AddChildNode(FSLOwlExperimentStatics::CreateInEpisodeProperty("log", EpisodeId));
	return EventIndividual;
}

// Add the owl representation of the event to the owl document
void FSLSlideEvent::AddToOwlDoc(FSLOwlDoc* OutDoc)
{
	// Add timepoint individuals
	// We know that the document is of type FOwlExperiment,
	// we cannot use the safer dynamic_cast because RTTI is not enabled by default
	// if (FOwlEvents* EventsDoc = dynamic_cast<FOwlEvents*>(OutDoc))
	FSLOwlExperiment* EventsDoc = static_cast<FSLOwlExperiment*>(OutDoc);
	EventsDoc->RegisterTimepoint(StartTime);
	EventsDoc->RegisterTimepoint(EndTime);
	EventsDoc->RegisterObject(Manipulator);
	EventsDoc->RegisterObject(Individual);
	OutDoc->AddIndividual(ToOwlNode());
}

// Get event context data as string (ToString equivalent)
FString FSLSlideEvent::Context() const
{
	return FString::Printf(TEXT("Slide - %lld"), PairId);
}

// Get the tooltip data
FString FSLSlideEvent::Tooltip() const
{
	return FString::Printf(TEXT("\'O1\',\'%s\',\'Id\',\'%s\',\'O2\',\'%s\',\'Id\',\'%s\',\'Id\',\'%s\'"),
		*Manipulator->GetClassValue(), *Manipulator->GetIdValue(), *Individual->GetClassValue(), *Individual->GetIdValue(), *Id);
}

// Get the data as string
FString FSLSlideEvent::ToString() const
{
	return FString::Printf(TEXT("Individual:[%s] Manipulator:[%s] PairId:%lld"),
		*Manipulator->GetInfo(), *Individual->GetInfo(), PairId);
}

FString FSLSlideEvent::RESTCallToKnowRob(FSLKRRestClient* InFSLKRRestClient) const
{
	// Call REST method to create sub actions on KnowRob side.
	// somaClassName:somaIndividualName is sent at the moment for objects participated
	FString ObjectsPartcipated = TEXT("[")
		+ Individual->GetClassValue() + TEXT(":") + Individual->GetParentActor()->GetActorLabel()
		+ TEXT(",")
		+ Manipulator->GetClassValue() + TEXT(":") + Manipulator->GetParentActor()->GetActorLabel()
		+ TEXT("]");


	FString SubActionType = TEXT("soma:'Slide'");
	FString TaskType = TEXT("soma:'Sliding'");

	FString AdditionalEventInfo = TEXT("");
	InFSLKRRestClient->SendCreateSubActionRequest(SubActionType, TaskType,
		ObjectsPartcipated, AdditionalEventInfo, double(StartTime), double(EndTime));

	return TEXT("Succeed!");
}
/* End ISLEvent interface */