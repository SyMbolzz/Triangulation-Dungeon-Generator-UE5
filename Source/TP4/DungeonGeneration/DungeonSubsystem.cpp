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

    Rooms.Empty();
    CreateRooms(RoomClasses, RoomNumber, DungeonPosition, DungeonBounds);

    TArray<FVector2D> Points = {};
    for (ARoomBase* Room : Rooms)
    {
        Points.Add(FVector2D(Room->GetActorLocation()));
    }

    TArray<STriangle> Triangles = UTriangulation::GenerateTriangulation(Points);

    TArray<TPair<FVector2D, FVector2D>> MST = UMinSpanTree::GenerateMST(Triangles);

    TArray<TPair<FVector2D, FVector2D>> Corridors = GenerateCorridors(MST);

    // Draw dungeon bounds
    DrawDebugBox(GetWorld(), DungeonPosition, FVector(DungeonBounds, 0.f), FColor::Red, true, -1.f, 0, 1.f);

    // Draw triangles
    for (const auto& Triangle : Triangles)
    {
        FColor Color = FColor::Red;
        DrawDebugLine(GetWorld(), FVector(Triangle.A, DungeonPosition.Z), FVector(Triangle.B, DungeonPosition.Z), Color, true, -1.f, 0, 5.f);
        DrawDebugLine(GetWorld(), FVector(Triangle.B, DungeonPosition.Z), FVector(Triangle.C, DungeonPosition.Z), Color, true, -1.f, 0, 5.f);
        DrawDebugLine(GetWorld(), FVector(Triangle.C, DungeonPosition.Z), FVector(Triangle.A, DungeonPosition.Z), Color, true, -1.f, 0, 5.f);
    }

    //// Draw minimum spanning tree
    //for (const auto& Edge : MST)
    //{
    //    DrawDebugLine(GetWorld(), FVector(Edge.Key, DungeonPosition.Z + 100.f), FVector(Edge.Value, DungeonPosition.Z + 100.f), FColor::Green, true, -1.f, 0, 20.f);
    //}

    //// Draw corridors
    //for (const auto& Corridor : Corridors)
    //{
    //    DrawDebugLine(GetWorld(), FVector(Corridor.Key, DungeonPosition.Z + 200.f), FVector(Corridor.Value, DungeonPosition.Z + 200.f), FColor::Blue, true, -1.f, 0, 20.f);
    //}

    return true;
}

void UDungeonSubsystem::CreateRooms(TArray<TSubclassOf<ARoomBase>> RoomClasses, int RoomNumber, FVector DungeonPosition, FVector2D DungeonBounds)
{
    // Ensure at least one of each class is spawned
    Algo::RandomShuffle(RoomClasses);
    for (int i = 0; i < FMath::Min(RoomNumber, RoomClasses.Num()); i++)
    {  
        SpawnRoom(RoomClasses[i], DungeonPosition, DungeonBounds);
    }

    for (int i = 0; i < RoomNumber - RoomClasses.Num(); i++)
    {
        SpawnRoom(RoomClasses[FMath::RandRange(0, RoomClasses.Num() - 1)], DungeonPosition, DungeonBounds);
    }
}

bool UDungeonSubsystem::SpawnRoom(TSubclassOf<ARoomBase> RoomClass, FVector DungeonPosition, FVector2D DungeonBounds)
{
    //ARoomBase* SpawnedRoom = GetWorld()->SpawnActor<ARoomBase>(RoomClass);

    //// Tries 50 positions before failing
    //for (int i = 0; i < 50; i++)
    //{
    //    FVector2D RandPosition = FVector2D(FMath::FRandRange(-DungeonBounds.X, DungeonBounds.X), 
    //                                       FMath::FRandRange(-DungeonBounds.Y, DungeonBounds.Y));

    //    bool bIsOverlapping = false;
    //    for (ARoomBase* Room : Rooms)
    //    {
    //        FVector SpawnedRoomExtent = GetActorExtent(SpawnedRoom);
    //        FVector RoomExtent = GetActorExtent(Room);
    //        FVector RoomPosition = Room->GetActorLocation();

    //        bool bOverlapsX = FMath::Abs(RandPosition.X - RoomPosition.X) <= (SpawnedRoomExtent.X + RoomExtent.X);
    //        bool bOverlapsY = FMath::Abs(RandPosition.Y - RoomPosition.Y) <= (SpawnedRoomExtent.Y + RoomExtent.Y);

    //        // Rooms overlap if they overlap on all axes
    //        if (bOverlapsX && bOverlapsY)
    //        {
    //            UE_LOG(LogTemp, Log, TEXT("Unvalid room position."));
    //            bIsOverlapping = true;
    //            break;
    //        }
    //    }

    //    if (!bIsOverlapping)
    //    {
    //        SpawnedRoom->SetActorLocation(FVector(RandPosition, DungeonPosition.Z));
    //        Rooms.Add(SpawnedRoom);
    //        return true;
    //    }
    //}

    //// Destroy the actor if no suitable position is found
    //UE_LOG(LogTemp, Warning, TEXT("No suitable position found for a room."));
    //SpawnedRoom->Destroy();
    //return false;

    ARoomBase* SpawnedRoom = GetWorld()->SpawnActor<ARoomBase>(RoomClass, 
        FVector(DungeonPosition.X + FMath::FRandRange(-DungeonBounds.X, DungeonBounds.X),
                DungeonPosition.Y + FMath::FRandRange(-DungeonBounds.Y, DungeonBounds.Y),
                0.f), FRotator::ZeroRotator);

    Rooms.Add(SpawnedRoom);

    return true;
}

FVector UDungeonSubsystem::GetActorExtent(AActor* Actor)
{
    FVector Origin;
    FVector Extent;
    Actor->GetActorBounds(true, Origin, Extent);
    return Extent;
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
