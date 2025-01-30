#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Triangulation.generated.h"

/**
 * Represents a triangle in 2D space
 * Vertices are stored in sorted order for consistent comparison
 */
struct STriangle
{
public:
    FVector2D A, B, C;

    STriangle(FVector2D InA, FVector2D InB, FVector2D InC)
    {
        // Sort vertices to ensure consistent ordering for comparison
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

/**
 * Represents an edge between two points
 * Points are stored in sorted order for consistent comparison
 */
struct SEdge
{
public:
    FVector2D Start;
    FVector2D End;

    SEdge(FVector2D InStart, FVector2D InEnd)
    {
        // Ensure consistent ordering by sorting points
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

/**
 * Represents a circumcircle of a triangle
 * Used in Delaunay triangulation algorithm
 */
struct SCircumcircle
{
public:
    FVector2D Center;
    float Radius;

    // Checks if a point lies within the circumcircle
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