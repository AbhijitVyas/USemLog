// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Events/SLContactEventHandler.h"
#include "Monitors/SLContactMonitorInterface.h"
#include "Events/SLContactEvent.h"
#include "Events/SLSupportedByEvent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Utils/SLUuid.h"
#include "Individuals/Type/SLParticleIndividual.h"


// Set parent
void FSLContactEventHandler::Init(UObject* InParent)
{
	if (!bIsInit)
	{
		// Check if parent is of right type
		Parent = Cast<ISLContactMonitorInterface>(InParent);
		if (Parent)
		{
			bIsInit = true;
		}
	}
}

// Bind to input delegates
void FSLContactEventHandler::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if (Parent)
		{
			Parent->OnBeginSLContact.AddRaw(this, &FSLContactEventHandler::OnSLOverlapBegin);
			Parent->OnEndSLContact.AddRaw(this, &FSLContactEventHandler::OnSLOverlapEnd);

			Parent->OnBeginSLSupportedBy.AddRaw(this, &FSLContactEventHandler::OnSLSupportedByBegin);
			Parent->OnEndSLSupportedBy.AddRaw(this, &FSLContactEventHandler::OnSLSupportedByEnd);
		}

		// Mark as started
		bIsStarted = true;
	}
}

// Terminate listener, finish and publish remaining events
void FSLContactEventHandler::Finish(float EndTime, bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Let parent first publish any pending (delayed) events
		if(!Parent->IsFinished())
		{
			Parent->Finish();
		}
		
		// End and broadcast all started events
		FinishAllEvents(EndTime);

		// TODO use dynamic delegates to be able to unbind from them
		// https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Delegates/Dynamic
		// this would mean that the handler will need to inherit from UObject

		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Start new contact event
void FSLContactEventHandler::AddNewContactEvent(const FSLContactResult& InResult)
{
	// Start a semantic contact event
	TSharedPtr<FSLContactEvent> Event = MakeShareable(new FSLContactEvent(
		FSLUuid::NewGuidInBase64Url(), InResult.Time,
		FSLUuid::PairEncodeCantor(InResult.Self->GetUniqueID(), InResult.Other->GetUniqueID()),
		InResult.Self, InResult.Other));
	Event->EpisodeId = EpisodeId;
	// Add event to the pending contacts array
	StartedContactEvents.Emplace(Event);

	// Start a semantic Pouring event, if particles are involved in contact
	AddNewPouringEvent(InResult);
}

// Publish finished event
bool FSLContactEventHandler::FinishContactEvent(USLBaseIndividual* InOther, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedContactEvents.CreateIterator()); EventItr; ++EventItr)
	{
		PouringEndTime = EndTime;
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Individual2 == InOther)
		{
			// Set the event end time
			(*EventItr)->EndTime = EndTime;

			// Avoid publishing short events
			if (((*EventItr)->EndTime - (*EventItr)->StartTime) > ContactEventMin)
			{
				OnSemanticEvent.ExecuteIfBound(*EventItr);
			}
			
			// find out how many particles are ending overlap with source container
			if ((*EventItr)->Individual2->GetClass() == USLParticleIndividual::StaticClass()) {
				particlesOverlapEnded = particlesOverlapEnded + 1;
				UE_LOG(LogTemp, Warning, TEXT("%d total particles overlapped.."),
					particlesOverlapEnded);
			}

			// Remove event from the pending list
			EventItr.RemoveCurrent();

			// if for the last contact event, the end time is the pouring endtime
			if (PouringEndTime < EndTime) {
				PouringEndTime = EndTime;
			}

			return true;
		}
	}
	return false;
}

// Start new supported by event
void FSLContactEventHandler::AddNewSupportedByEvent(USLBaseIndividual* Supported, USLBaseIndividual* Supporting, float StartTime, const uint64 EventPairId)
{
	// Start a supported by event
	TSharedPtr<FSLSupportedByEvent> Event = MakeShareable(new FSLSupportedByEvent(
		FSLUuid::NewGuidInBase64Url(), StartTime, EventPairId, Supported, Supporting));
	Event->EpisodeId = EpisodeId;
	// Add event to the pending array
	StartedSupportedByEvents.Emplace(Event);
}

// Finish then publish the event
bool FSLContactEventHandler::FinishSupportedByEvent(const uint64 InPairId, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedSupportedByEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->PairId == InPairId)
		{
			// Ignore short events
			if (EndTime - (*EventItr)->StartTime > SupportedByEventMin)
			{
				// Set end time and publish event
				(*EventItr)->EndTime = EndTime;
				OnSemanticEvent.ExecuteIfBound(*EventItr);
			}
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending contact events (this usually is called at end play)
void FSLContactEventHandler::FinishAllEvents(float EndTime)
{
	// Finish contact events
	for (auto& Ev : StartedContactEvents)
	{
		// Ignore short events
		if (EndTime - Ev->StartTime > ContactEventMin)
		{
			// Set end time and publish event
			Ev->EndTime = EndTime;
			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}
	StartedContactEvents.Empty();

	// Finish supported by events
	for (auto& Ev : StartedSupportedByEvents)
	{
		// Ignore short events
		if ((EndTime - Ev->StartTime) > SupportedByEventMin)
		{
			// Set end time and publish event
			Ev->EndTime = EndTime;
			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}
	StartedSupportedByEvents.Empty();

	// Finish pouring events
	for (auto& Ev : StartedPouringEvents)
	{
		// Ignore short events
		if (PouringEndTime - Ev->StartTime > PouringEventMin)
		{
			// Set end time and publish event
			Ev->EndTime = PouringEndTime;
			OnSemanticEvent.ExecuteIfBound(Ev);
		}
	}
	StartedPouringEvents.Empty();
}

// Start new Pouring event
void FSLContactEventHandler::AddNewPouringEvent(const FSLContactResult& InResult)
{
	// Start a semantic Pouring event

	// TODO: gather how many particles show up.. then compare them with two different types of containers..

	if (InResult.Other->GetClass() == USLParticleIndividual::StaticClass() && StartedPouringEvents.Num() == 0) {
		// if there is already CurrPouredIndividual object then pouring is initiated
		TSharedPtr<FSLPouringEvent> Event = MakeShareable(new FSLPouringEvent(
			FSLUuid::NewGuidInBase64Url(), InResult.Time,
			FSLUuid::PairEncodeCantor(InResult.Self->GetUniqueID(), InResult.Other->GetUniqueID()),
			InResult.Self, InResult.Other));
		Event->EpisodeId = EpisodeId;
		// Add event to the pending Pourings array
		StartedPouringEvents.Emplace(Event);
	}

}

// Publish finished event
bool FSLContactEventHandler::FinishPouringEvent(USLBaseIndividual* InOther, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedPouringEvents.CreateIterator()); EventItr; ++EventItr)
	{
		// It is enough to compare against the other id when searching
		if ((*EventItr)->Individual2 == InOther)
		{
			// Set the event end time
			(*EventItr)->EndTime = EndTime;

			// Avoid publishing short events
			if (((*EventItr)->EndTime - (*EventItr)->StartTime) > PouringEventMin)
			{
				OnSemanticEvent.ExecuteIfBound(*EventItr);
			}

			// Remove event from the pending list
			EventItr.RemoveCurrent();

			return true;
		}
	}
	return false;
}

// Event called when a semantic overlap event begins
void FSLContactEventHandler::OnSLOverlapBegin(const FSLContactResult& InResult)
{
	AddNewContactEvent(InResult);
}

// Event called when a semantic overlap event ends
void FSLContactEventHandler::OnSLOverlapEnd(USLBaseIndividual* Self, USLBaseIndividual* Other, float Time)
{
	FinishContactEvent(Other, Time);
}

// Event called when a supported by event begins
void FSLContactEventHandler::OnSLSupportedByBegin(USLBaseIndividual* Supported, USLBaseIndividual* Supporting, float StartTime, const uint64 PairId)
{
	AddNewSupportedByEvent(Supported, Supporting, StartTime, PairId);
}

// Event called when a 'possible' supported by event ends
void FSLContactEventHandler::OnSLSupportedByEnd(const uint64 PairId1, const uint64 PairId2, float EndTime)
{
	if(!FinishSupportedByEvent(PairId1, EndTime))
	{
		FinishSupportedByEvent(PairId2, EndTime);
	}
}
