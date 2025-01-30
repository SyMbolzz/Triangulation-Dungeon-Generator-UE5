#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "RoomBase.generated.h"

UCLASS()
class TP4_API ARoomBase : public AActor
{
	GENERATED_BODY()
	
public:
	ARoomBase();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UBoxComponent* RoomExtent;

};
