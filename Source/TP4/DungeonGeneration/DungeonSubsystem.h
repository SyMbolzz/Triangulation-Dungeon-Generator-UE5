#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RoomBase.h"
#include "DungeonSubsystem.generated.h"

UCLASS()
class TP4_API UDungeonSubsystem : public UGameInstanceSubsystem 
{
    GENERATED_BODY()

public:

    // Main dungeon generation function
    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    bool GenerateDungeon(int Seed, TArray<TSubclassOf<ARoomBase>> RoomClasses, int RoomNumber, FVector DungeonPosition, FVector2D DungeonBounds);

private:

    // Core generation steps
    void CreateRooms(TArray<TSubclassOf<ARoomBase>> RoomClasses, int RoomNumber, FVector DungeonPosition, FVector2D DungeonBounds);

    bool SpawnRoom(TSubclassOf<ARoomBase> RoomClass, FVector DungeonPosition, FVector2D DungeonBounds);

    FVector GetActorExtent(AActor* Actor);

    TArray<TPair<FVector2D, FVector2D>> GenerateCorridors(const TArray<TPair<FVector2D, FVector2D>>& MST);

    // Data
    TArray<ARoomBase*> Rooms;
};