// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MazeGameModeBase.generated.h"

class UMazeGenerator;
/**
 * 
 */
UCLASS(Blueprintable)
class MAZE_API AMazeGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void CreateMaze(TSubclassOf<AActor> WallActorClass, float CellSize
		, int MazeHaight, int MazeWidth, int SectionSize, bool Sleep);

private:
	void VisualizeMaze(UWorld* World, const UMazeGenerator* Generator, TSubclassOf<AActor> WallActorClass, float CellSize);
};
