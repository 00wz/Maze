#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MazeGenerator.generated.h"

/** Структура одной ячейки лабиринта */
USTRUCT()
struct FMazeCell
{
	GENERATED_BODY()

	bool bVisited = false;
	bool bWallTop = true;
	bool bWallBottom = true;
	bool bWallLeft = true;
	bool bWallRight = true;
};

/** Генератор лабиринта */
UCLASS(Blueprintable)
class MAZE_API UMazeGenerator : public UObject
{
	GENERATED_BODY()

public:
	void GenerateMaze(int Width, int Height, int SectionSize, bool Sleep);

private:
	void GenerateMazeSection(int SectionX, int SectionY);
	void MergeSectionBorders();

	int MazeWidth;
	int MazeHeight;
	int MazeSectionSize;
	TArray<TArray<FMazeCell>> MazeGrid;

	FCriticalSection Mutex;

public:
	int GetHeight() const { return MazeHeight;}
	int GetWidth() const { return  MazeWidth;}
	TArray<TArray<FMazeCell>>& GetGrid() { return MazeGrid;}
};
