#include "RoomBase.h"

ARoomBase::ARoomBase()
{
    //RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Create and attach the box collision component
	RoomExtent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	RoomExtent->SetupAttachment(RootComponent);
	RoomExtent->SetBoxExtent(FVector(500.f, 500.f, 200.f));
	RoomExtent->SetLineThickness(5.f);

    //// Create the physics constraint
    //PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
    //PhysicsConstraint->SetupAttachment(RoomExtent);

    //// Lock the Z-axis
    //PhysicsConstraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Free, 0.0f); // Allow X movement
    //PhysicsConstraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Free, 0.0f); // Allow Y movement
    //PhysicsConstraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.0f); // Lock Z movement

    //// Disable angular motion
    //PhysicsConstraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
    //PhysicsConstraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
    //PhysicsConstraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.0f);

    //// Set the constraint's connected components
    //PhysicsConstraint->SetConstrainedComponents(RoomExtent, NAME_None, nullptr, NAME_None);

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

    //TArray<AActor*> OverlappingActors;
    //RoomExtent->GetOverlappingActors(OverlappingActors, ARoomBase::StaticClass());
    //if (OverlappingActors.IsEmpty())
    //{
    //    RoomExtent->SetSimulatePhysics(false);
    //    RoomExtent->SetCollisionProfileName(FName("NoCollision"));
    //}
}
