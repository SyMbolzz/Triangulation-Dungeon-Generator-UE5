#include "DungeonSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Algo/RandomShuffle.h"
#include "Triangulation.h"
#include "MinSpanTree.h"

/**
 * Main entry point for dungeon generation
 * Handles the complete process from room spawning to corridor creation
 */
bool UDungeonSubsystem::GenerateDungeon(int Seed, TArray<TSubclassOf<ARoomBase>> RoomClasses, int RoomSpawned, TArray<TSubclassOf<ACorridorBase>> CorridorClasses, FVector DungeonPosition, FVector2D DungeonMinBounds, bool DrawBounds, bool DrawTriangulation, bool DrawMST, bool DrawCorridorLines)
{
    // Validate input
    if (RoomClasses.IsEmpty() || CorridorClasses.IsEmpty() || RoomSpawned <= 0
        || DungeonMinBounds.X < 0 || DungeonMinBounds.Y < 0)
    { 
        return false;
    }

    // Initialize random number generator with seed for reproducible results
    FMath::RandInit(Seed);

    // Store parameters for later use
    DungeonHeight = DungeonPosition.Z;
    m_CorridorClasses = CorridorClasses;
    m_DrawTriangulation = DrawTriangulation;
    m_DrawMST = DrawMST;
    m_DrawCorridorLines = DrawCorridorLines;

    // Create initial room layout
    m_Rooms = CreateRooms(RoomClasses, RoomSpawned, DungeonPosition, DungeonMinBounds);

    // Set up timer to check when physics simulation is complete
    GetWorld()->GetTimerManager().SetTimer(SleepCheckHandle, this, &UDungeonSubsystem::CheckAllRoomsSleeping, 0.05f, true);
    
    // Safety timer in case physics simulation doesn't settle
    GetWorld()->GetTimerManager().SetTimer(SafetyHandle, [this]()
        {
            // Is called after 5 seconds if the rooms are still not sleeping
            if (SleepCheckHandle.IsValid())
            {
                SleepCheckHandle.Invalidate();
                OnAllRoomsSleep();
            }
        }, 5.0f, false);


    // Draw dungeon boundaries if requested
    if (DrawBounds)
    {
        DrawDebugBox(GetWorld(), DungeonPosition, FVector(DungeonMinBounds, 0.f), FColor::Red, true, -1.f, 0, 1.f);
    }
    
    return true;
}

/**
 * Creates initial room layouts using physics simulation
 * Rooms are spawned with random positions and orientations
 * Physics simulation is used to resolve overlaps
 */
TArray<ARoomBase*> UDungeonSubsystem::CreateRooms(const TArray<TSubclassOf<ARoomBase>>& RoomClasses, const int& RoomNumber, const FVector& DungeonPosition, const FVector2D& DungeonBounds)
{
    TArray<ARoomBase*> SpawnedRooms;
    // Define possible room rotations (0, 90, 180, 270 degrees)
    TArray<float> PossibleAngles = { 0.f, 90.f, 180.f, 270.f };

    // First pass: Ensure at least one of each room type is spawned
    Algo::RandomShuffle(RoomClasses);
    for (int i = 0; i < FMath::Min(RoomNumber, RoomClasses.Num()); i++)
    {  
        // Randomly rotate the room
        FRotator Rotation = FRotator(0.f, PossibleAngles[FMath::RandRange(0, PossibleAngles.Num() - 1)],0.f);
        
        // Spawn room at random position within bounds
        ARoomBase* SpawnedRoom = GetWorld()->SpawnActor<ARoomBase>(RoomClasses[i],
            FVector(DungeonPosition.X + FMath::FRandRange(-DungeonBounds.X, DungeonBounds.X),
                    DungeonPosition.Y + FMath::FRandRange(-DungeonBounds.Y, DungeonBounds.Y),
                    DungeonPosition.Z), Rotation);

        SpawnedRooms.Add(SpawnedRoom);
    }

    // Second pass: Fill remaining rooms with random types
    for (int i = 0; i < RoomNumber - RoomClasses.Num(); i++)
    {
        FRotator Rotation = FRotator(0.f, PossibleAngles[FMath::RandRange(0, PossibleAngles.Num() - 1)], 0.f);
        ARoomBase* SpawnedRoom = GetWorld()->SpawnActor<ARoomBase>(RoomClasses[FMath::RandRange(0, RoomClasses.Num() - 1)],
            FVector(DungeonPosition.X + FMath::FRandRange(-DungeonBounds.X, DungeonBounds.X),
                    DungeonPosition.Y + FMath::FRandRange(-DungeonBounds.Y, DungeonBounds.Y),
                    DungeonPosition.Z), Rotation);

        SpawnedRooms.Add(SpawnedRoom);
    }

    return SpawnedRooms;
}

/**
 * Called when all rooms have finished physics simulation
 * Handles the main dungeon generation steps:
 * 1. Removes overlapping rooms
 * 2. Gets key points for triangulation
 * 3. Creates Delaunay triangulation
 * 4. Generates minimum spanning tree
 * 5. Creates corridor layout
 * 6. Spawns final corridors
 */
void UDungeonSubsystem::OnAllRoomsSleep()
{
    // Clean up rooms that ended up overlapping
    RemoveOverlapedRooms(m_Rooms);

    // Get key points for triangulation from room positions
    TArray<FVector2D> Points = GetPoints(m_Rooms);

    // Generate Delaunay triangulation
    TArray<STriangle> Triangles = UTriangulation::GenerateTriangulation(Points);

    // Create minimum spanning tree from triangulation
    TArray<TPair<FVector2D, FVector2D>> MST = UMinSpanTree::GenerateMST(Triangles);

    // Generate L-shaped corridor paths
    TArray<TPair<FVector2D, FVector2D>> CorridorLines = GenerateCorridorLines(MST);

    // Remove rooms that aren't connected by corridors
    RemoveRoomsNotInCorridorLines(m_Rooms, CorridorLines);

    // Create actual corridor actors
    m_Corridors = CreateCorridors(CorridorLines);

    // Disable room collision after generation
    for (ARoomBase* Room : m_Rooms)
    {
        Room->RoomExtent->SetCollisionProfileName(FName("NoCollision"));
    }

    // Draw debug visualization if requested
    if (m_DrawTriangulation)
    {
        // Draw triangulation lines
        for (const auto& Triangle : Triangles)
        {
            FColor Color = FColor::Red;
            DrawDebugLine(GetWorld(), FVector(Triangle.A, DungeonHeight + 100.f), FVector(Triangle.B, DungeonHeight + 100.f), Color, true, -1.f, 0, 5.f);
            DrawDebugLine(GetWorld(), FVector(Triangle.B, DungeonHeight + 100.f), FVector(Triangle.C, DungeonHeight + 100.f), Color, true, -1.f, 0, 5.f);
            DrawDebugLine(GetWorld(), FVector(Triangle.C, DungeonHeight + 100.f), FVector(Triangle.A, DungeonHeight + 100.f), Color, true, -1.f, 0, 5.f);
        }
    }

    if (m_DrawMST)
    {
        // Draw minimum spanning tree edges
        for (const auto& Edge : MST)
        {
            DrawDebugLine(GetWorld(), FVector(Edge.Key, DungeonHeight + 200.f), FVector(Edge.Value, DungeonHeight + 200.f), FColor::Green, true, -1.f, 0, 20.f);
        }
    }

    if (m_DrawCorridorLines)
    {
        // Draw final corridor paths
        for (const auto& Corridor : CorridorLines)
        {
            DrawDebugLine(GetWorld(), FVector(Corridor.Key, DungeonHeight + 300.f), FVector(Corridor.Value, DungeonHeight + 300.f), FColor::Blue, true, -1.f, 0, 20.f);
        }
    }
}

/**
 * Removes rooms that overlap with each other
 * Uses physics collision detection to find overlaps
 * @param Rooms - Array of rooms to check and clean up
 */
void UDungeonSubsystem::RemoveOverlapedRooms(TArray<ARoomBase*>& Rooms)
{
    TArray<ARoomBase*> RoomsToRemove;

    // Check each room for validity and overlaps
    for (ARoomBase* Room : Rooms)
    {
        if (!IsValid(Room))
        {
            RoomsToRemove.Add(Room);
            continue;
        }

        // Disable physics and set up overlap checking
        Room->RoomExtent->SetSimulatePhysics(false);
        Room->RoomExtent->SetCollisionProfileName(FName("OverlapAll"));

        // Check for overlapping rooms
        TSet<AActor*> OverlapingActors;
        Room->RoomExtent->GetOverlappingActors(OverlapingActors, ARoomBase::StaticClass());

        // Destroy overlapping rooms
        for (AActor* OverlapingActor : OverlapingActors)
        {
            if (OverlapingActor != Room)
            {
                OverlapingActor->Destroy();
            }
        }
    }

    // Remove invalid rooms from array
    for (ARoomBase* RoomToRemove : RoomsToRemove)
    {
        Rooms.Remove(RoomToRemove);
    }
}

/**
 * Selects key points for dungeon layout
 * Takes a subset of room positions to use as nodes
 * @param Rooms - Array of all dungeon rooms
 * @return Array of 2D points for triangulation
 */
TArray<FVector2D> UDungeonSubsystem::GetPoints(const TArray<ARoomBase*>& Rooms)
{
    TArray<ARoomBase*> RoomsCopy = Rooms;
    Algo::RandomShuffle(RoomsCopy);

    TArray<FVector2D> Points;

    // Select a subset of rooms (at least 4, up to 1/4 of total rooms)
    int32 NumPointsToGet = FMath::Max(4, RoomsCopy.Num() / 4);
    NumPointsToGet = FMath::Min(NumPointsToGet, RoomsCopy.Num());

    // Get positions from selected rooms
    for (int32 i = 0; i < NumPointsToGet; i++)
    {
        Points.Add(FVector2D(RoomsCopy[i]->GetActorLocation()));
    }

    return Points;
}

/**
 * Generates L-shaped corridor paths between rooms
 * Creates corridors by choosing random intermediate points
 * @param MST - Minimum spanning tree edges
 * @return Array of corridor line segments
 */
TArray<TPair<FVector2D, FVector2D>> UDungeonSubsystem::GenerateCorridorLines(const TArray<TPair<FVector2D, FVector2D>>& MST)
{
    TArray<TPair<FVector2D, FVector2D>> Corridors;

    // Create L-shaped paths for each MST edge
    for (const auto& Edge : MST)
    {
        // Randomly choose whether to go horizontal or vertical first
        FVector2D IntermediatePoint = FMath::RandBool() ? FVector2D(Edge.Value.X, Edge.Key.Y) :
                                               FVector2D(Edge.Key.X, Edge.Value.Y);

        // Add both segments of the L-shaped path
        Corridors.Add(TPair<FVector2D, FVector2D>(Edge.Key, IntermediatePoint));
        Corridors.Add(TPair<FVector2D, FVector2D>(IntermediatePoint, Edge.Value));
    }

    return Corridors;
}

/**
 * Removes rooms that aren't connected by corridors
 * Uses line traces to detect rooms along corridor paths
 * @param Rooms - Array of rooms to filter
 * @param CorridorLines - Corridor path segments
 */
void UDungeonSubsystem::RemoveRoomsNotInCorridorLines(TArray<ARoomBase*>& Rooms, TArray<TPair<FVector2D, FVector2D>> CorridorLines)
{
    // Set up collision for line traces
    for (ARoomBase* Room : Rooms)
    {
        Room->RoomExtent->SetCollisionProfileName(FName("BlockAll"));
    }

    TArray<ARoomBase*> RoomsToKeep;

    // Check each corridor line
    for (const TPair<FVector2D, FVector2D>& Corridor : CorridorLines)
    {
        TArray<FHitResult> Hits;

        // Trace in both directions to ensure we catch all rooms
        if (GetWorld()->LineTraceMultiByChannel(Hits, FVector(Corridor.Key, DungeonHeight), FVector(Corridor.Value, DungeonHeight), ECollisionChannel::ECC_WorldDynamic))
        {
            for (const FHitResult& Hit : Hits)
            {
                if (ARoomBase* Room = Cast<ARoomBase>(Hit.GetActor()))
                {
                    RoomsToKeep.AddUnique(Room);
                }
            }
        }
        // Trace in reverse direction
        if (GetWorld()->LineTraceMultiByChannel(Hits, FVector(Corridor.Value, DungeonHeight), FVector(Corridor.Key, DungeonHeight), ECollisionChannel::ECC_WorldDynamic))
        {
            for (const FHitResult& Hit : Hits)
            {
                if (ARoomBase* Room = Cast<ARoomBase>(Hit.GetActor()))
                {
                    RoomsToKeep.AddUnique(Room);
                }
            }
        }
    }

    // Remove rooms that aren't connected by corridors
    for (int32 i = Rooms.Num() - 1; i >= 0; --i)
    {
        ARoomBase* Room = Rooms[i];
        if (Room)
        {
            // Remove Room if it doesn't exist in ActorsToKeep
            if (!RoomsToKeep.Contains(Room))
            {
                Rooms[i]->Destroy();
                Rooms.RemoveAt(i);
            }
        }
    }
}

/**
 * Spawns corridor actors between connected rooms
 * Creates and scales corridor meshes along paths
 * @param CorridorLines - Corridor path segments
 * @return Array of spawned corridor actors
 */
TArray<ACorridorBase*> UDungeonSubsystem::CreateCorridors(TArray<TPair<FVector2D, FVector2D>> CorridorLines)
{
    TArray<ACorridorBase*> Corridors;

    // Create a corridor actor for each path segment
    for (const TPair<FVector2D, FVector2D>& CorridorLine : CorridorLines)
    {
        // Randomly select corridor class
        TSubclassOf<ACorridorBase> CorridorClass = m_CorridorClasses[FMath::RoundToInt(FMath::RandRange(0.f, m_CorridorClasses.Num() - 1.f))];
        
        // Calculate corridor transform
        FVector Location = FVector(CorridorLine.Key, DungeonHeight);
        FRotator Rotation = FVector(CorridorLine.Value - CorridorLine.Key, 0.f).ToOrientationRotator();
        FVector Scale = FVector(FVector::Dist(FVector(CorridorLine.Key, 0.f), FVector(CorridorLine.Value, 0.f)) / 100.f, 1.f, 1.f);
        if (ACorridorBase* Corridor = GetWorld()->SpawnActor<ACorridorBase>(CorridorClass, Location, Rotation))
        {
            Corridor->SetActorScale3D(Scale);
            Corridors.Add(Corridor);
        }
        
    }
    return Corridors;
}

/**
 * Checks if physics simulation has completed
 * Called periodically until all rooms are stationary
 */
void UDungeonSubsystem::CheckAllRoomsSleeping()
{
    // Check if all rooms are sleeping
    bool bAllSleeping = true;
    for (const ARoomBase* Room : m_Rooms)
    {
        if (Room->RoomExtent->IsAnyRigidBodyAwake())
        {
            bAllSleeping = false;
            break;
        }
    }

    if (bAllSleeping && SleepCheckHandle.IsValid())
    {
        SleepCheckHandle.Invalidate();
        OnAllRoomsSleep();
    }
}