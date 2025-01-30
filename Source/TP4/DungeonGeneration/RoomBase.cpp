#include "RoomBase.h"

ARoomBase::ARoomBase()
{
	// Create and attach the box collision component
	RoomExtent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	RoomExtent->SetupAttachment(RootComponent);
	RoomExtent->SetBoxExtent(FVector(500.f, 500.f, 200.f));
	RoomExtent->SetLineThickness(5.f);

    RoomExtent->SetSimulatePhysics(true);
    RoomExtent->SetLinearDamping(10.f);
    RoomExtent->SetEnableGravity(false);
    RoomExtent->BodyInstance.bLockZTranslation = true;
    RoomExtent->BodyInstance.bLockXRotation = true;
    RoomExtent->BodyInstance.bLockYRotation = true;
    RoomExtent->BodyInstance.bLockZRotation = true;
    RoomExtent->SetCollisionProfileName(FName("PhysicsActor"), true);
    RoomExtent->SetUseCCD(true);

    SetTickGroup(ETickingGroup::TG_PrePhysics);
	PrimaryActorTick.bCanEverTick = true;
}

void ARoomBase::BeginPlay()
{
	Super::BeginPlay();
}

void ARoomBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
