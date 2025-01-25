#include "Triangulation.h"

TArray<STriangle> UTriangulation::GenerateTriangulation(const TArray<FVector2D>& Points)
{
    TArray<STriangle> Triangles;

    // Step 1: Create a super-triangle that contains all points
    STriangle SuperTriangle = GenerateSuperTriangle(Points);
    Triangles.Add(SuperTriangle);

    // Step 2: Add points one by one
    for (const FVector2D& Point : Points)
    {
        // Find all triangles whose circumcircle contains the point
        TArray<STriangle> BadTriangles;
        TArray<SEdge> Polygon;

        for (const STriangle& Triangle : Triangles)
        {
            if (GetCircumcircle(Triangle).ContainsPoint(Point))
            {
                BadTriangles.Add(Triangle);
            }
        }

        // Get unique edges from bad triangles
        for (const STriangle& Triangle : BadTriangles)
        {
            SEdge Edge1(Triangle.A, Triangle.B);
            SEdge Edge2(Triangle.B, Triangle.C);
            SEdge Edge3(Triangle.C, Triangle.A);

            bool IsShared1 = false;
            bool IsShared2 = false;
            bool IsShared3 = false;

            // Check if each edge is shared with other bad triangles
            for (const STriangle& OtherTriangle : BadTriangles)
            {
                if (&Triangle == &OtherTriangle) continue;

                if ((Edge1 == SEdge(OtherTriangle.A, OtherTriangle.B)) ||
                    (Edge1 == SEdge(OtherTriangle.B, OtherTriangle.C)) ||
                    (Edge1 == SEdge(OtherTriangle.C, OtherTriangle.A)))
                {
                    IsShared1 = true;
                }

                if ((Edge2 == SEdge(OtherTriangle.A, OtherTriangle.B)) ||
                    (Edge2 == SEdge(OtherTriangle.B, OtherTriangle.C)) ||
                    (Edge2 == SEdge(OtherTriangle.C, OtherTriangle.A)))
                {
                    IsShared2 = true;
                }

                if ((Edge3 == SEdge(OtherTriangle.A, OtherTriangle.B)) ||
                    (Edge3 == SEdge(OtherTriangle.B, OtherTriangle.C)) ||
                    (Edge3 == SEdge(OtherTriangle.C, OtherTriangle.A)))
                {
                    IsShared3 = true;
                }
            }

            // Add unshared edges to polygon
            if (!IsShared1) Polygon.AddUnique(Edge1);
            if (!IsShared2) Polygon.AddUnique(Edge2);
            if (!IsShared3) Polygon.AddUnique(Edge3);
        }

        // Remove bad triangles
        for (const STriangle& Triangle : BadTriangles)
        {
            Triangles.Remove(Triangle);
        }

        // Create new triangles from the polygon edges
        for (const SEdge& Edge : Polygon)
        {
            Triangles.Add(STriangle(Edge.Start, Edge.End, Point));
        }
    }

    // Remove triangles that share vertices with super-triangle
    for (int32 i = Triangles.Num() - 1; i >= 0; --i)
    {
        if (SharesVertexWithSuperTriangle(Triangles[i], SuperTriangle))
        {
            Triangles.RemoveAt(i);
        }
    }

    return Triangles;
}

STriangle UTriangulation::GenerateSuperTriangle(const TArray<FVector2D>& Points)
{
    if (Points.Num() == 0)
    {
        return STriangle(FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector);
    }

    // Find the bounds of the points
    FVector2D MinPoint(FLT_MAX, FLT_MAX);
    FVector2D MaxPoint(-FLT_MAX, -FLT_MAX);

    for (const FVector2D& Point : Points)
    {
        MinPoint.X = FMath::Min(MinPoint.X, Point.X);
        MinPoint.Y = FMath::Min(MinPoint.Y, Point.Y);

        MaxPoint.X = FMath::Max(MaxPoint.X, Point.X);
        MaxPoint.Y = FMath::Max(MaxPoint.Y, Point.Y);
    }

    // Calculate the center of the bounds
    FVector2D Center = (MinPoint + MaxPoint) * 0.5f;

    // Calculate extents
    float Width = MaxPoint.X - MinPoint.X;
    float Height = MaxPoint.Y - MinPoint.Y;

    // Generate a large triangle around the bounds
    FVector2D A = FVector2D(Center.X, Center.Y + Height * 2.0f); // Point above the bounds
    FVector2D B = FVector2D(Center.X - Width * 2.0f, Center.Y - Height); // Bottom-left
    FVector2D C = FVector2D(Center.X + Width * 2.0f, Center.Y - Height); // Bottom-right

    return STriangle(A, B, C);
}

SCircumcircle UTriangulation::GetCircumcircle(const STriangle& Triangle)
{
    FVector2D A = Triangle.A;
    FVector2D B = Triangle.B;
    FVector2D C = Triangle.C;

    SCircumcircle Circle;

    // Calculate the determinant (D)
    float D = 2 * (A.X * (B.Y - C.Y) + B.X * (C.Y - A.Y) + C.X * (A.Y - B.Y));
    if (FMath::Abs(D) < KINDA_SMALL_NUMBER)
    {
        // The points are collinear, no circumcircle exists
        return SCircumcircle();
    }

    // Calculate the center (Ux, Uy)
    float Ux = ((A.X * A.X + A.Y * A.Y) * (B.Y - C.Y) +
        (B.X * B.X + B.Y * B.Y) * (C.Y - A.Y) +
        (C.X * C.X + C.Y * C.Y) * (A.Y - B.Y)) / D;

    float Uy = ((A.X * A.X + A.Y * A.Y) * (C.X - B.X) +
        (B.X * B.X + B.Y * B.Y) * (A.X - C.X) +
        (C.X * C.X + C.Y * C.Y) * (B.X - A.X)) / D;

    Circle.Center = FVector2D(Ux, Uy);

    // Calculate the radius
    Circle.Radius = FVector2D::Distance(Circle.Center, A);

    return Circle;
}

bool UTriangulation::SharesVertexWithSuperTriangle(const STriangle& Triangle, const STriangle& SuperTriangle)
{
    return Triangle.A == SuperTriangle.A || Triangle.A == SuperTriangle.B || Triangle.A == SuperTriangle.C ||
        Triangle.B == SuperTriangle.A || Triangle.B == SuperTriangle.B || Triangle.B == SuperTriangle.C ||
        Triangle.C == SuperTriangle.A || Triangle.C == SuperTriangle.B || Triangle.C == SuperTriangle.C;
}
