// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "SLPouringParticleAgentClass.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

// Sets default values
ASLPouringParticleAgentClass::ASLPouringParticleAgentClass()
{ 
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = mesh;

}

// Called when the game starts or when spawned
void ASLPouringParticleAgentClass::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASLPouringParticleAgentClass::StaticClass(), FoundActors);

	UE_LOG(LogTemp, Log, TEXT("This is total number of found actors of type ASLPouringParticleAgentClass: %d"), FoundActors.Num());

	// get all actors of ASLPouringParticleAgentClass that are on overlap begine with the plane with source container
	// for each actor, start recording location once it starts overlaping plane in source container
	// get all actors of ASLPouringParticleAgentClass that are on overlap begine with the plane with destination container
	// for each actor, record its location once it stops movement.
	// count how many actors are still in destination container
	// count how many actors are spilled down.



	// TODO0: Modify the SLParticleIndividual such that it can log its info to the semanticmap.owl file: DONE
	// TODO0: Work on toggle masks option as well so that each individual particle can have unique color: Not imp as of now
	// TODO0:create custom collision channel for particles such that they do not collide with each other: I have done it but not sure if it is correctly implemented
	// TODO0: Work on creating new maps with different source and dest container and then put all particles inside source container(make pouring scenario ready)
	// TODO0: Work on different shapes collisions in Blender. https://www.youtube.com/watch?v=q1iF08P8hC0&ab_channel=C%3A%5CInsertNameHere : Tried but not working as expected. The resulted mesh does not let any particles drop inside.
	// TODO1: get all particles with given class ASLPouringParticleAgentClass in c++
	// TODO2: for each particle, check if it collides with the invisible surface of the container on top, ignore its own collision with other particles
	// TODO3: if it leaves the container that means it is now in pouring mode: Invisible layer for collision
	// TODO4: log its location every second(10 times) into tf file
	// TODO5: check if particle collides with destination container's imaginary surface so that we can say that it is inside dest container.
	
}

// Called every frame
void ASLPouringParticleAgentClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

