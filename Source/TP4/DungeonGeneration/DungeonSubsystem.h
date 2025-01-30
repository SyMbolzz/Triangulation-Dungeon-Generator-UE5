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

    /**
     * Generates a procedural dungeon with rooms and corridors
     * @param Seed - Random seed for dungeon generation
     * @param RoomClasses - Array of room types to spawn
     * @param RoomSpawned - Total number of rooms to generate
     * @param CorridorClasses - Array of corridor types to use
     * @param DungeonPosition - Center position of the dungeon
     * @param DungeonMinBounds - Minimum X,Y bounds for room placement
     * @param DrawBounds - Whether to draw debug bounds
     * @param DrawTriangulation - Whether to draw debug triangulation
     * @param DrawMST - Whether to draw minimum spanning tree
     * @param DrawCorridorLines - Whether to draw corridor debug lines
     * @return bool - Success/failure of dungeon generation
     */
    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    bool GenerateDungeon(int Seed, TArray<TSubclassOf<ARoomBase>> RoomClasses, int RoomSpawned, TArray<TSubclassOf<ACorridorBase>> CorridorClasses, FVector DungeonPosition, FVector2D DungeonMinBounds, bool DrawBounds, bool DrawTriangulation, bool DrawMST, bool DrawCorridorLines);

    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    TArray<ARoomBase*> GetRooms() { return m_Rooms; }

    UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
    TArray<ACorridorBase*> GetCorridors() { return m_Corridors; }

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

    // Debug
    bool m_DrawTriangulation;
    bool m_DrawMST;
    bool m_DrawCorridorLines;
};