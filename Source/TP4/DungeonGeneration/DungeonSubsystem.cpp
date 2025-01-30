#include "DungeonSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Algo/RandomShuffle.h"
#include "Triangulation.h"
#include "MinSpanTree.h"

bool UDungeonSubsystem::GenerateDungeon(int Seed, TArray<TSubclassOf<ARoomBase>> RoomClasses, int RoomNumber, TArray<TSubclassOf<ACorridorBase>> CorridorClasses, FVector DungeonPosition, FVector2D DungeonMinBounds)
{
    if (RoomClasses.IsEmpty()) { return false; }

    // Seed the randomness
    FMath::RandInit(Seed);

    DungeonHeight = DungeonPosition.Z;
    m_CorridorClasses = CorridorClasses;

    m_Rooms = CreateRooms(RoomClasses, RoomNumber, DungeonPosition, DungeonMinBounds);

    GetWorld()->GetTimerManager().SetTimer(SleepCheckHandle, this, &UDungeonSubsystem::CheckAllRoomsSleeping, 0.05f, true);
    
    // Safety timer
    GetWorld()->GetTimerManager().SetTimer(SafetyHandle, [this]()
        {
            // Is called after 5 seconds if the rooms are still not sleeping
            if (SleepCheckHandle.IsValid())
            {
                SleepCheckHandle.Invalidate();
                OnAllRoomsSleep();
            }
        }, 5.0f, false);

    //// Draw dungeon bounds
    //DrawDebugBox(GetWorld(), DungeonPosition, FVector(DungeonMinBounds, 0.f), FColor::Red, true, -1.f, 0, 1.f);

    return true;
}

TArray<ARoomBase*> UDungeonSubsystem::CreateRooms(const TArray<TSubclassOf<ARoomBase>>& RoomClasses, const int& RoomNumber, const FVector& DungeonPosition, const FVector2D& DungeonBounds)
{
    TArray<ARoomBase*> SpawnedRooms;

    // Ensure at least one of each class is spawned
    Algo::RandomShuffle(RoomClasses);
    for (int i = 0; i < FMath::Min(RoomNumber, RoomClasses.Num()); i++)
    {  
        ARoomBase* SpawnedRoom = GetWorld()->SpawnActor<ARoomBase>(RoomClasses[i],
            FVector(DungeonPosition.X + FMath::FRandRange(-DungeonBounds.X, DungeonBounds.X),
                    DungeonPosition.Y + FMath::FRandRange(-DungeonBounds.Y, DungeonBounds.Y),
                    0.f), FRotator::ZeroRotator);

        SpawnedRooms.Add(SpawnedRoom);
    }

    for (int i = 0; i < RoomNumber - RoomClasses.Num(); i++)
    {
        ARoomBase* SpawnedRoom = GetWorld()->SpawnActor<ARoomBase>(RoomClasses[FMath::RandRange(0, RoomClasses.Num() - 1)],
            FVector(DungeonPosition.X + FMath::FRandRange(-DungeonBounds.X, DungeonBounds.X),
                    DungeonPosition.Y + FMath::FRandRange(-DungeonBounds.Y, DungeonBounds.Y),
                    0.f), FRotator::ZeroRotator);

        SpawnedRooms.Add(SpawnedRoom);
    }

    return SpawnedRooms;
}

void UDungeonSubsystem::OnAllRoomsSleep()
{
    RemoveOverlapedRooms(m_Rooms);

    TArray<FVector2D> Points = GetPoints(m_Rooms);

    TArray<STriangle> Triangles = UTriangulation::GenerateTriangulation(Points);

    TArray<TPair<FVector2D, FVector2D>> MST = UMinSpanTree::GenerateMST(Triangles);

    TArray<TPair<FVector2D, FVector2D>> CorridorLines = GenerateCorridorLines(MST);

    RemoveRoomsNotInCorridorLines(m_Rooms, CorridorLines);

    m_Corridors = CreateCorridors(CorridorLines);

    for (ARoomBase* Room : m_Rooms)
    {
        Room->RoomExtent->SetCollisionProfileName(FName("NoCollision"));
    }

    //// Draw triangles
    //for (const auto& Triangle : Triangles)
    //{
    //    FColor Color = FColor::Red;
    //    DrawDebugLine(GetWorld(), FVector(Triangle.A, DungeonHeight + 100.f), FVector(Triangle.B, DungeonHeight + 100.f), Color, true, -1.f, 0, 5.f);
    //    DrawDebugLine(GetWorld(), FVector(Triangle.B, DungeonHeight + 100.f), FVector(Triangle.C, DungeonHeight + 100.f), Color, true, -1.f, 0, 5.f);
    //    DrawDebugLine(GetWorld(), FVector(Triangle.C, DungeonHeight + 100.f), FVector(Triangle.A, DungeonHeight + 100.f), Color, true, -1.f, 0, 5.f);
    //}

    //// Draw minimum spanning tree
    //for (const auto& Edge : MST)
    //{
    //    DrawDebugLine(GetWorld(), FVector(Edge.Key, DungeonHeight + 200.f), FVector(Edge.Value, DungeonHeight + 200.f), FColor::Green, true, -1.f, 0, 20.f);
    //}

    //// Draw corridors
    //for (const auto& Corridor : CorridorLines)
    //{
    //    DrawDebugLine(GetWorld(), FVector(Corridor.Key, DungeonHeight + 300.f), FVector(Corridor.Value, DungeonHeight + 300.f), FColor::Blue, true, -1.f, 0, 20.f);
    //}
}

void UDungeonSubsystem::RemoveOverlapedRooms(TArray<ARoomBase*>& Rooms)
{
    TArray<ARoomBase*> RoomsToRemove;
    for (ARoomBase* Room : Rooms)
    {
        if (!IsValid(Room))
        {
            RoomsToRemove.Add(Room);
            continue;
        }

        Room->RoomExtent->SetSimulatePhysics(false);
        Room->RoomExtent->SetCollisionProfileName(FName("OverlapAll"));

        TSet<AActor*> OverlapingActors;
        Room->RoomExtent->GetOverlappingActors(OverlapingActors, ARoomBase::StaticClass());

        for (AActor* OverlapingActor : OverlapingActors)
        {
            if (OverlapingActor != Room)
            {
                OverlapingActor->Destroy();
            }
        }
    }

    for (ARoomBase* RoomToRemove : RoomsToRemove)
    {
        Rooms.Remove(RoomToRemove);
    }
}

TArray<FVector2D> UDungeonSubsystem::GetPoints(const TArray<ARoomBase*>& Rooms)
{
    TArray<ARoomBase*> RoomsCopy = Rooms;
    Algo::RandomShuffle(RoomsCopy);

    TArray<FVector2D> Points;

    // Calculate how many points to get (at least 4, up to 1/4 of rooms)
    int32 NumPointsToGet = FMath::Max(4, RoomsCopy.Num() / 4);

    // Make sure we don't try to get more points than we have rooms
    NumPointsToGet = FMath::Min(NumPointsToGet, RoomsCopy.Num());

    // Select the points
    for (int32 i = 0; i < NumPointsToGet; i++)
    {
        Points.Add(FVector2D(RoomsCopy[i]->GetActorLocation()));
    }

    return Points;
}

TArray<TPair<FVector2D, FVector2D>> UDungeonSubsystem::GenerateCorridorLines(const TArray<TPair<FVector2D, FVector2D>>& MST)
{
    TArray<TPair<FVector2D, FVector2D>> Corridors;
    for (const auto& Edge : MST)
    {
        FVector2D IntermediatePoint = FMath::RandBool() ? FVector2D(Edge.Value.X, Edge.Key.Y) :
                                               FVector2D(Edge.Key.X, Edge.Value.Y);

        Corridors.Add(TPair<FVector2D, FVector2D>(Edge.Key, IntermediatePoint));
        Corridors.Add(TPair<FVector2D, FVector2D>(IntermediatePoint, Edge.Value));
    }

    return Corridors;
}

void UDungeonSubsystem::RemoveRoomsNotInCorridorLines(TArray<ARoomBase*>& Rooms, TArray<TPair<FVector2D, FVector2D>> CorridorLines)
{
    for (ARoomBase* Room : Rooms)
    {
        Room->RoomExtent->SetCollisionProfileName(FName("BlockAll"));
    }

    TArray<ARoomBase*> RoomsToKeep;

    for (const TPair<FVector2D, FVector2D>& Corridor : CorridorLines)
    {
        TArray<FHitResult> Hits;
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

TArray<ACorridorBase*> UDungeonSubsystem::CreateCorridors(TArray<TPair<FVector2D, FVector2D>> CorridorLines)
{
    TArray<ACorridorBase*> Corridors;
    for (const TPair<FVector2D, FVector2D>& CorridorLine : CorridorLines)
    {
        TSubclassOf<ACorridorBase> CorridorClass = m_CorridorClasses[FMath::RoundToInt(FMath::RandRange(0.f, m_CorridorClasses.Num() - 1.f))];
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