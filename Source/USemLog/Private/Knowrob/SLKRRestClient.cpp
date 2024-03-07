#include "Knowrob/SLKRRestClient.h"

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "Containers/Queue.h"
#include <string>


FSLKRRestClient::FSLKRRestClient()
{
    GameParticipant = TEXT("");
}

FSLKRRestClient::~FSLKRRestClient()
{
}

void FSLKRRestClient::Init(const FString& InHost, const FString& InPort, const FString& InProtocol, const FString& GameUser) {

    URL = InHost + TEXT(":") + InPort + TEXT("/knowrob/api/v1.0/");
    GameParticipant = GameUser;
    IsConnected();

}

bool FSLKRRestClient::IsConnected() {
    return true;
}

void FSLKRRestClient::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
}

FString FSLKRRestClient::getEpisodeIri() {
    return EpisodeIri;
}


FString FSLKRRestClient::getActionIri() {
    return ActionIri;
}


bool FSLKRRestClient::SendCreateEpisodeRequest() {
    
    bool returnStatus;
    // create a GET request
    FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    // final URL
    FString FINAL_URL = URL + TEXT("create_episode");
    Request->SetURL(FINAL_URL);
    Request->SetVerb((TEXT("POST")));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // get system time when game starts and send it to rest call for episode start
    FDateTime timeUtc = FDateTime::UtcNow();
    int64 GameStartTime = timeUtc.ToUnixTimestamp();
    UE_LOG(LogTemp, Display, TEXT("Game Start unix time without utc: %lld"), GameStartTime); // log time
    // store start time for later use with logging of sub actions
    SetGameStartUnixTime(GameStartTime);

    FString Query = FString::Printf(TEXT("{ \"game_participant\": \"%s\", \"game_start_time\": \"%lf\"}"), *GameParticipant, double(GameStartTime));
    Request->SetContentAsString(*Query);
    Request->OnProcessRequestComplete().BindLambda(
        // Here, we "capture" the 'this' pointer (the "&"), so our lambda can call this
        // class's methods in the callback.
        [&](
            FHttpRequestPtr Request,
            FHttpResponsePtr Response,
            bool connectedSuccessfully) mutable {

                if (connectedSuccessfully) {
                    returnStatus = true;
                    UE_LOG(LogTemp, Display, TEXT("Successfully connected to the server"));
                    // We should have a JSON response - attempt to process it.
                    FString ResponseJsonString = *Response->GetContentAsString(); //Json converted to FString
                    UE_LOG(LogTemp, Display, TEXT("Response %s"), *ResponseJsonString); // log the response as string
                    
                    // Parse Json Message Here
                    TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(ResponseJsonString);
                    TSharedPtr< FJsonObject > JsonObject;
                    bool DeserializeState = FJsonSerializer::Deserialize(Reader, JsonObject);
                    if (!DeserializeState)
                    {
                        UE_LOG(LogTemp, Error, TEXT("[%s] Deserialization Error. Message Contents: %s"),
                            *FString(__FUNCTION__), *ResponseJsonString);
                        return;
                    }
                    else {
                        if (JsonObject != NULL) {
                            //Get the value of the json object by field name
                            ActionIri = JsonObject->GetStringField("Action");
                            EpisodeIri = JsonObject->GetStringField("Episode");
                        }
                    }
                }
                else {
                    returnStatus = false;
                    switch (Request->GetStatus()) {
                    case EHttpRequestStatus::Failed_ConnectionError:
                        UE_LOG(LogTemp, Error, TEXT("Connection failed."));
                    default:
                        UE_LOG(LogTemp, Error, TEXT("Request failed."));
                    }
                }
        });
    Request->ProcessRequest();
    return returnStatus;
}


EHttpRequestStatus::Type FSLKRRestClient::SendFinishEpisodeRequest() {
    // create a GET request
    FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    // final URL
    FString FINAL_URL = URL + TEXT("finish_episode");
    Request->SetURL(FINAL_URL);
    Request->SetVerb((TEXT("POST")));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    // get system time when game ends and send it via rest api
    FDateTime timeUtc = FDateTime::UtcNow();
    int64 GameStopTime = timeUtc.ToUnixTimestamp();

    GameStopUnixTime = double(GameStopTime);
    if (GameStopUnixTime < GameStartUnixTime) {
        UE_LOG(LogTemp, Error, TEXT("Game End Time: %lf can not be less than Start Time: %lf"), double(GameStopUnixTime), double(GameStartUnixTime)); // log time
    }
    UE_LOG(LogTemp, Display, TEXT("game end time: %lld"), GameStopUnixTime); // log time
    FString Query = FString::Printf(TEXT("{ \"episode_iri\": \"%s\", \"game_end_time\": \"%lf\"}"), *EpisodeIri, GameStopUnixTime);
    Request->SetContentAsString(*Query);
    Request->ProcessRequest();
    EHttpRequestStatus::Type Status = Request->GetStatus();
    return Status;
}

void FSLKRRestClient::SendCreateSubActionRequest(FString SubActionType, FString TaskType, 
    FString ObjectsPartcipated, FString AdditionalEventInfo, double StartTime, double EndTime) {
    // create a GET request
    FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    // final URL
    FString FINAL_URL = URL + TEXT("add_subaction_with_task");
    Request->SetURL(FINAL_URL);
    Request->SetVerb((TEXT("POST")));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // adjust start and stop time for sub action with game start in unix format
    double  EventStartTime = GetGameStartUnixTime() + StartTime;
    double  EventEndTime = GetGameStartUnixTime() + EndTime;

    UE_LOG(LogTemp, Display, TEXT("Sub Event %s with start time : %lld and stop time: %lld"), *SubActionType, EventStartTime, EventEndTime); // log time

    FString Query = FString::Printf(TEXT("{ \"parent_action_iri\": \"%s\", \"sub_action_type\": \"%s\", \"task_type\": \"%s\", \"start_time\": \"%lf\", \"end_time\": \"%lf\", \"objects_participated\": \"%s\", \"additional_event_info\": \"%s\", \"game_participant\": \"%s\"}"),
        *ActionIri, *SubActionType, *TaskType, EventStartTime, EventEndTime, *ObjectsPartcipated, *AdditionalEventInfo, *GameParticipant);
    Request->SetContentAsString(*Query);
    Request->ProcessRequest();
    const TCHAR* Status = EHttpRequestStatus::ToString(Request->GetStatus());
    UE_LOG(LogTemp, Warning, TEXT("response type: %s"), Status);
}

void FSLKRRestClient::SendPouringAdditionalRequest(FString SubActionType, FString MaxPouringAngle, FString MinPouringAngle, 
    FString SourceContainer, FString DestinationContainer, FString PouringPose) {
    // create a GET request
    FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    // final URL
    FString FINAL_URL = URL + TEXT("add_additional_pouring_information");
    Request->SetURL(FINAL_URL);
    Request->SetVerb((TEXT("POST")));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));


    FString Query = FString::Printf(TEXT("{ \"parent_action_iri\": \"%s\", \"sub_action_type\": \"%s\", \"max_pouring_angle\": \"%s\", \"min_pouring_angle\": \"%s\", \"source_container\": \"%s\", \"destination_container\": \"%s\", \"pouring_pose\": \"%s\"}"),
        *ActionIri, *SubActionType, *MaxPouringAngle, *MinPouringAngle, *SourceContainer, *DestinationContainer, *PouringPose);
    Request->SetContentAsString(*Query);
    Request->ProcessRequest();
    const TCHAR* Status = EHttpRequestStatus::ToString(Request->GetStatus());
    UE_LOG(LogTemp, Warning, TEXT("response type: %s"), Status);
}

void FSLKRRestClient::SendRequest(FString RequestContent) {
   
    FHttpModule& httpModule = FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> pRequest = httpModule.CreateRequest();

    pRequest->SetVerb(TEXT("POST"));
    pRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    FString Query = FString::Printf(TEXT("{ \"query\": \"%s\", \"maxSolutionCount\" : 1}"), *RequestContent);
    pRequest->SetContentAsString(*Query);

    pRequest->SetURL(URL);

    pRequest->OnProcessRequestComplete().BindLambda(
        // Here, we "capture" the 'this' pointer (the "&"), so our lambda can call this
        // class's methods in the callback.
        [&](
            FHttpRequestPtr pRequest,
            FHttpResponsePtr pResponse,
            bool connectedSuccessfully) mutable {

                if (connectedSuccessfully) {

                    // We should have a JSON response - attempt to process it.
                   // ProcessSpaceTrackResponse(pResponse->GetContentAsString());
                    UE_LOG(LogTemp, Warning, TEXT("Success"));
                    ProcessKnowrobResponse(pResponse->GetContentAsString());
                }
                else {
                    switch (pRequest->GetStatus()) {
                    case EHttpRequestStatus::Failed_ConnectionError:
                        UE_LOG(LogTemp, Error, TEXT("Connection failed."));
                    default:
                        UE_LOG(LogTemp, Error, TEXT("Request failed."));
                    }
                }
        });

    pRequest->ProcessRequest();
}

void FSLKRRestClient::ProcessKnowrobResponse(const FString& ResponseContent) {
    
}

double FSLKRRestClient::GetGameStartUnixTime() {
    return GameStartUnixTime;
}

void FSLKRRestClient::SetGameStartUnixTime(double StartTime) {
    GameStartUnixTime = StartTime;
}