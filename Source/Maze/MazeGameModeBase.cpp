// Copyright Epic Games, Inc. All Rights Reserved.


#include "MazeGameModeBase.h"

#include "MazeGenerator.h"

void AMazeGameModeBase::CreateMaze(TSubclassOf<AActor> WallActorClass, float CellSize
		, int MazeHaight, int MazeWidth, int SectionSize, bool Sleep)
{	
	UMazeGenerator*	MazeGenerator = NewObject<UMazeGenerator>();
	
	
	// Создаём граф-задачу генерации
	FGraphEventRef MazeGenTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
		[MazeGenerator, MazeHaight, MazeWidth, SectionSize, Sleep]()
		{
			double StartTime = FPlatformTime::Seconds();
			MazeGenerator->GenerateMaze(MazeWidth, MazeHaight, SectionSize, Sleep);
			double EndTime = FPlatformTime::Seconds();
			double ElapsedTime = EndTime - StartTime;
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,                   // Key (−1 — всегда новая строка)
					5.0f,                 // Время отображения (сек.)
					FColor::Yellow,       // Цвет текста
					FString::Printf(TEXT("Время выполнения: %.6f сек"), ElapsedTime)
				);
			}
		},
		TStatId(),
		nullptr,                  // Нет зависимостей
		ENamedThreads::AnyBackgroundThreadNormalTask // Любой поток, кроме GameThread
	);
	
	// Создаём граф-задачу визуализации
	FGraphEventRef MazeVisualizeTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
		[this, MazeGenerator, WallActorClass, CellSize]()
		{
			VisualizeMaze(GetWorld(), MazeGenerator, WallActorClass, CellSize);
		},
		TStatId(),
		MazeGenTask,                  // После генерации лабиринта
		ENamedThreads::GameThread // Создание акторов в GameThread
	);
}

void AMazeGameModeBase:: VisualizeMaze(UWorld* World, const UMazeGenerator* Generator, TSubclassOf<AActor> WallActorClass, float CellSize)
{
	if (!World || !Generator || !WallActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameters for VisualizeMaze"));
		return;
	}

	const int Width = Generator->GetWidth();
	const int Height = Generator->GetHeight();
	const auto& Grid = Generator->GetGrid();

	for (int x = 0; x < Width; ++x)
	{
		for (int y = 0; y < Height; ++y)
		{
			const FMazeCell& Cell = Grid[x][y];
			const FVector CellOrigin = FVector(x * CellSize, y * CellSize, 0.0f);

			// Визуализируем стены (Z – высота стен)
			if (y == 0 && Cell.bWallTop)
			{
				FVector Pos = CellOrigin + FVector(CellSize / 2, 0, 0);
				FRotator Rot = FRotator(0, 0, 0);
				World->SpawnActor<AActor>(WallActorClass, FTransform(Rot, Pos));
			}

			if (Cell.bWallBottom)
			{
				FVector Pos = CellOrigin + FVector(CellSize / 2, CellSize, 0);
				FRotator Rot = FRotator(0, 0, 0);
				World->SpawnActor<AActor>(WallActorClass, FTransform(Rot, Pos));
			}

			if (x == 0 && Cell.bWallLeft)
			{
				FVector Pos = CellOrigin + FVector(0, CellSize / 2, 0);
				FRotator Rot = FRotator(0, 90, 0);
				World->SpawnActor<AActor>(WallActorClass, FTransform(Rot, Pos));
			}

			if (Cell.bWallRight)
			{
				FVector Pos = CellOrigin + FVector(CellSize, CellSize / 2, 0);
				FRotator Rot = FRotator(0, 90, 0);
				World->SpawnActor<AActor>(WallActorClass, FTransform(Rot, Pos));
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Maze visualization complete."));
}
