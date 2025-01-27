#include "DungeonSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Algo/RandomShuffle.h"
#include "Triangulation.h"
#include "MinSpanTree.h"

bool UDungeonSubsystem::GenerateDungeon(int Seed, TArray<TSubclassOf<ARoomBase>> RoomClasses, int RoomNumber, FVector DungeonPosition, FVector2D DungeonBounds)
{
    if (RoomClasses.IsEmpty()) { return false; }

    // Seed the randomness
    FMath::RandInit(Seed);

    m_Rooms = CreateRooms(RoomClasses, RoomNumber, DungeonPosition, DungeonBounds);

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

    // Draw dungeon bounds
    DrawDebugBox(GetWorld(), DungeonPosition, FVector(DungeonBounds, 0.f), FColor::Red, true, -1.f, 0, 1.f);
    DungeonHeight = DungeonPosition.Z;

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

FVector UDungeonSubsystem::GetActorExtent(AActor* Actor)
{
    FVector Origin;
    FVector Extent;
    Actor->GetActorBounds(true, Origin, Extent);
    return Extent;
}

TArray<FVector2D> UDungeonSubsystem::GetPoints(const TArray<ARoomBase*>& Rooms)
{
    TArray<FVector2D> Points;

    // Select randomly one fourth of the rooms
    for (int i = 0; i < Rooms.Num(); i++)
    {
        if (i%4 == 0)
        {
            Points.Add(FVector2D(Rooms[i]->GetActorLocation()));
        }
    }

    return Points;
}

TArray<TPair<FVector2D, FVector2D>> UDungeonSubsystem::GenerateCorridors(const TArray<TPair<FVector2D, FVector2D>>& MST)
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

void UDungeonSubsystem::OnAllRoomsSleep()
{
    UE_LOG(LogTemp, Warning, TEXT("Rooms sleep"));

    TArray<ARoomBase*> RoomsToRemove;
    for (ARoomBase* Room : m_Rooms)
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
        m_Rooms.Remove(RoomToRemove);
    }

    TArray<FVector2D> Points = GetPoints(m_Rooms);

    TArray<STriangle> Triangles = UTriangulation::GenerateTriangulation(Points);

    TArray<TPair<FVector2D, FVector2D>> MST = UMinSpanTree::GenerateMST(Triangles);

    TArray<TPair<FVector2D, FVector2D>> Corridors = GenerateCorridors(MST);

    // Draw triangles
    for (const auto& Triangle : Triangles)
    {
        FColor Color = FColor::Red;
        DrawDebugLine(GetWorld(), FVector(Triangle.A, DungeonHeight), FVector(Triangle.B, DungeonHeight), Color, true, -1.f, 0, 5.f);
        DrawDebugLine(GetWorld(), FVector(Triangle.B, DungeonHeight), FVector(Triangle.C, DungeonHeight), Color, true, -1.f, 0, 5.f);
        DrawDebugLine(GetWorld(), FVector(Triangle.C, DungeonHeight), FVector(Triangle.A, DungeonHeight), Color, true, -1.f, 0, 5.f);
    }

    // Draw minimum spanning tree
    for (const auto& Edge : MST)
    {
        DrawDebugLine(GetWorld(), FVector(Edge.Key, DungeonHeight + 100.f), FVector(Edge.Value, DungeonHeight + 100.f), FColor::Green, true, -1.f, 0, 20.f);
    }

    // Draw corridors
    for (const auto& Corridor : Corridors)
    {
        DrawDebugLine(GetWorld(), FVector(Corridor.Key, DungeonHeight + 200.f), FVector(Corridor.Value, DungeonHeight + 200.f), FColor::Blue, true, -1.f, 0, 20.f);
    }
}
