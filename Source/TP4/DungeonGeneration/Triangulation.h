#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Triangulation.generated.h"

struct STriangle
{
public:
    FVector2D A, B, C;

    STriangle(FVector2D InA, FVector2D InB, FVector2D InC)
    {
        // Sort vertices consistently
        TArray<FVector2D> Vertices = { InA, InB, InC };
        Vertices.Sort();
        A = Vertices[0];
        B = Vertices[1];
        C = Vertices[2];
    }

    // Equality operator
    bool operator==(const STriangle& Other) const
    {
        return A == Other.A && B == Other.B && C == Other.C;
    }
};

struct SEdge
{
public:
    FVector2D Start;
    FVector2D End;

    SEdge(FVector2D InStart, FVector2D InEnd)
    {
        // Ensure consistent ordering of points
        if (InStart.X < InEnd.X || (InStart.X == InEnd.X && InStart.Y < InEnd.Y))
        {
            Start = InStart;
            End = InEnd;
        }
        else
        {
            Start = InEnd;
            End = InStart;
        }
    }

    bool operator==(const SEdge& Other) const
    {
        return Start == Other.Start && End == Other.End;
    }
};

struct SCircumcircle
{
public:
    FVector2D Center;
    float Radius;

    bool ContainsPoint(const FVector2D& Point) const
    {
        return FVector2D::Distance(Center, Point) <= Radius;
    }
};

UCLASS()
class TP4_API UTriangulation : public UObject
{
    GENERATED_BODY()

public:

    static TArray<STriangle> GenerateTriangulation(const TArray<FVector2D>& Points);

private:

    static STriangle GenerateSuperTriangle(const TArray<FVector2D>& Points);

    static SCircumcircle GetCircumcircle(const STriangle& Triangle);

    static bool SharesVertexWithSuperTriangle(const STriangle& Triangle, const STriangle& SuperTriangle);
};