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
    TArray<ARoomBase*> CreateRooms(const TArray<TSubclassOf<ARoomBase>>& RoomClasses, const int& RoomNumber, const FVector& DungeonPosition, const FVector2D& DungeonBounds);

    //Helper functions
    FVector GetActorExtent(AActor* Actor);

    TArray<FVector2D> GetPoints(const TArray<ARoomBase*>& Rooms);

    TArray<TPair<FVector2D, FVector2D>> GenerateCorridors(const TArray<TPair<FVector2D, FVector2D>>& MST);

    void CheckAllRoomsSleeping();

    void OnAllRoomsSleep();

    // Data
    TArray<ARoomBase*> m_Rooms;

    FTimerHandle SleepCheckHandle;
    FTimerHandle SafetyHandle;

    // Debug
    float DungeonHeight;
};