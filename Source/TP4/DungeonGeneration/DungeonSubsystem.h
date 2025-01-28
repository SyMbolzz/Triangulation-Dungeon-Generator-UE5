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

    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    TArray<ARoomBase*> GetRooms() { return m_Rooms; }

private:

    // Core generation steps
    TArray<ARoomBase*> CreateRooms(const TArray<TSubclassOf<ARoomBase>>& RoomClasses, const int& RoomNumber, const FVector& DungeonPosition, const FVector2D& DungeonBounds);

    void OnAllRoomsSleep();

    void RemoveOverlapedRooms(TArray<ARoomBase*>& Rooms);
    
    TArray<TPair<FVector2D, FVector2D>> GenerateCorridors(const TArray<TPair<FVector2D, FVector2D>>& MST);

    void RemoveRoomsNotInCorridors(TArray<ARoomBase*>& Rooms, TArray<TPair<FVector2D, FVector2D>> Corridors);
    
    //Helper functions
    TArray<FVector2D> GetPoints(const TArray<ARoomBase*>& Rooms);

    void CheckAllRoomsSleeping();

    // Data
    TArray<ARoomBase*> m_Rooms;

    FTimerHandle SleepCheckHandle;
    FTimerHandle SafetyHandle;

    // Debug
    float DungeonHeight;
};