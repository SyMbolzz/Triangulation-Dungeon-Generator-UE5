#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Triangulation.h"
#include "MinSpanTree.generated.h"

UCLASS()
class TP4_API UMinSpanTree : public UObject
{
    GENERATED_BODY()

public:

    static TArray<TPair<FVector2D, FVector2D>> GenerateMST(const TArray<STriangle>& Triangulation);

private:

    static TArray<FVector2D> GetPointsOfTriangles(const TArray<STriangle>& Triangles);

};

