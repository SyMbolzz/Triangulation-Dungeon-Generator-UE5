#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RoomBase.h"
#include "CorridorBase.h"
#include "DungeonSubsystem.generated.h"

UCLASS()
class TP4_API UDungeonSubsystem : public UGameInstanceSubsystem 
{
    GENERATED_BODY()

public:

    // Main dungeon generation function
    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    bool GenerateDungeon(int Seed, TArray<TSubclassOf<ARoomBase>> RoomClasses, int RoomNumber, TArray<TSubclassOf<ACorridorBase>> CorridorClasses, FVector DungeonPosition, FVector2D DungeonMinBounds);

    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    TArray<ARoomBase*> GetRooms() { return m_Rooms; }

private:

    // Core generation steps
    TArray<ARoomBase*> CreateRooms(const TArray<TSubclassOf<ARoomBase>>& RoomClasses, const int& RoomNumber, const FVector& DungeonPosition, const FVector2D& DungeonBounds);

    void OnAllRoomsSleep();

    void RemoveOverlapedRooms(TArray<ARoomBase*>& Rooms);
    
    TArray<TPair<FVector2D, FVector2D>> GenerateCorridorLines(const TArray<TPair<FVector2D, FVector2D>>& MST);

    void RemoveRoomsNotInCorridorLines(TArray<ARoomBase*>& Rooms, TArray<TPair<FVector2D, FVector2D>> CorridorLines);

    TArray<ACorridorBase*> CreateCorridors(TArray<TPair<FVector2D, FVector2D>> CorridorLines);
    
    //Helper functions
    TArray<FVector2D> GetPoints(const TArray<ARoomBase*>& Rooms);

    void CheckAllRoomsSleeping();

    // Data
    TArray<ARoomBase*> m_Rooms;
    TArray<TSubclassOf<ACorridorBase>> m_CorridorClasses;
    TArray<ACorridorBase*> m_Corridors;

    FTimerHandle SleepCheckHandle;
    FTimerHandle SafetyHandle;

    float DungeonHeight;
};