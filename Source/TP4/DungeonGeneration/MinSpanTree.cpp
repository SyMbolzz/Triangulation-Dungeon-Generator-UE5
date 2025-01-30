#include "MinSpanTree.h"

/**
 * Generates Minimum Spanning Tree using Kruskal's algorithm
 * @param Triangulation - Input Delaunay triangulation
 * @return Array of edges forming the MST
 */
TArray<TPair<FVector2D, FVector2D>> UMinSpanTree::GenerateMST(const TArray<STriangle>& Triangulation)
{
    // Extract unique points from triangulation
    TArray<FVector2D> Points = GetPointsOfTriangles(Triangulation);

    // Create edges with their lengths
    TArray<TPair<float, TPair<FVector2D, FVector2D>>> Edges;

    for (const STriangle& Triangle : Triangulation)
    {
        // Add all edges from triangle with their lengths
        Edges.Add(TPair<float, TPair<FVector2D, FVector2D>>(FVector2D::Distance(Triangle.A, Triangle.B),
                                                            TPair<FVector2D, FVector2D>(Triangle.A, Triangle.B)));
        Edges.Add(TPair<float, TPair<FVector2D, FVector2D>>(FVector2D::Distance(Triangle.B, Triangle.C),
                                                            TPair<FVector2D, FVector2D>(Triangle.B, Triangle.C)));
        Edges.Add(TPair<float, TPair<FVector2D, FVector2D>>(FVector2D::Distance(Triangle.C, Triangle.A),
                                                            TPair<FVector2D, FVector2D>(Triangle.C, Triangle.A)));
    }

    // Sort edges by length (critical for Kruskal's algorithm)
    Edges.Sort([](const TPair<float, TPair<FVector2D, FVector2D>>& A, const TPair<float, TPair<FVector2D, FVector2D>>& B)
    {
        return A.Key < B.Key;
    });

    // Remove duplicate edges (same edge from different triangles)
    for (int i = Edges.Num() - 1; i >= 0; --i)
    {
        for (int j = i - 1; j >= 0; --j)
        {
            if ((Edges[i].Value.Key   == Edges[j].Value.Key &&
                 Edges[i].Value.Value == Edges[j].Value.Value) ||
                (Edges[i].Value.Key   == Edges[j].Value.Value &&
                 Edges[i].Value.Value == Edges[j].Value.Key))
            {
                Edges.RemoveAt(i);
                break;
            }
        }
    }

    // Build MST using Kruskal's algorithm
    TArray<TPair<FVector2D, FVector2D>> MST;
    TArray<FVector2D> ConnectedPoints;

    // Start with first point
    if (Points.Num() > 0)
    {
        ConnectedPoints.Add(Points[0]);
    }

    // Add edges until all points are connecte
    while (ConnectedPoints.Num() < Points.Num())
    {
        float BestDistance = FLT_MAX;
        TPair<FVector2D, FVector2D> BestEdge;
        bool EdgeFound = false;

        // Find shortest edge that connects a new point
        for (const auto& Edge : Edges)
        {
            bool AConnected = ConnectedPoints.Contains(Edge.Value.Key);
            bool BConnected = ConnectedPoints.Contains(Edge.Value.Value);

            // Edge must connect one connected and one unconnected point
            if (AConnected ^ BConnected)
            {
                if (Edge.Key < BestDistance)
                {
                    BestDistance = Edge.Key;
                    BestEdge = Edge.Value;
                    EdgeFound = true;
                }
            }
        }

        // Add best edge to MST
        if (EdgeFound)
        {
            MST.Add(BestEdge);
            ConnectedPoints.AddUnique(BestEdge.Key);
            ConnectedPoints.AddUnique(BestEdge.Value);
        }
        else
        {
            break; // No more valid edges found
        }
    }

    return MST;
}

TArray<FVector2D> UMinSpanTree::GetPointsOfTriangles(const TArray<STriangle>& Triangles)
{
    TArray<FVector2D> Points;

    for (const STriangle& Triangle : Triangles)
    {
        Points.AddUnique(Triangle.A);
        Points.AddUnique(Triangle.B);
        Points.AddUnique(Triangle.C);
    }

    return Points;
}
