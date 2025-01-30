#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CorridorBase.generated.h"

UCLASS()
class TP4_API ACorridorBase : public AActor
{
	GENERATED_BODY()
	
public:
	ACorridorBase();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

};
