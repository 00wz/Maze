#include "MazeGenerator.h"

#include "Algo/RandomShuffle.h"
#include "Async/TaskGraphInterfaces.h"
#include "HAL/PlatformProcess.h"

void UMazeGenerator::GenerateMaze(int Width, int Height, int SectionSize, bool Sleep)
{
    FGraphEventArray SectionTasks;
    
    MazeWidth = Width;
    MazeHeight = Height;
    MazeSectionSize = SectionSize;

    // Инициализация сетки
    MazeGrid.SetNum(MazeWidth);
    for (int x = 0; x < MazeWidth; ++x)
    {
        MazeGrid[x].SetNum(MazeHeight);
    }

    const int NumSectionsX = FMath::CeilToInt((float)MazeWidth / MazeSectionSize);
    const int NumSectionsY = FMath::CeilToInt((float)MazeHeight / MazeSectionSize);

    // Создание задач
    for (int sx = 0; sx < NumSectionsX; ++sx)
    {
        for (int sy = 0; sy < NumSectionsY; ++sy)
        {
            auto Task = FFunctionGraphTask::CreateAndDispatchWhenReady(
                [this, sx, sy, Sleep]()
                {
                    GenerateMazeSection(sx, sy);
                    if (Sleep) FPlatformProcess::Sleep(0.5f);
                },
                TStatId(),
                nullptr, 
		        ENamedThreads::AnyBackgroundThreadNormalTask // Любой поток, кроме GameThread т.к. используем Sleep
            );
            SectionTasks.Add(Task);
        }
    }

    // Синхронная задача после всех секций
    FGraphEventRef MergeAndCompleteTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [this]()
        {
            MergeSectionBorders();
        },
        TStatId(),
        &SectionTasks, 
        ENamedThreads::AnyThread // Любой поток
    );

    // Ждем завершение генерации лабиринта
    FTaskGraphInterface::Get().WaitUntilTaskCompletes(MergeAndCompleteTask);
}

void UMazeGenerator::GenerateMazeSection(int SectionX, int SectionY)
{
    int StartX = SectionX * MazeSectionSize;
    int StartY = SectionY * MazeSectionSize;
    int EndX = FMath::Min(StartX + MazeSectionSize, MazeWidth);
    int EndY = FMath::Min(StartY + MazeSectionSize, MazeHeight);

    // Простейшая генерация DFS — начнём с одного узла
    TArray<FIntPoint> Stack;
    Stack.Add(FIntPoint(StartX, StartY));

    while (Stack.Num() > 0)
    {
        FIntPoint Current = Stack.Pop();

        if (Current.X < StartX || Current.X >= EndX || Current.Y < StartY || Current.Y >= EndY)
            continue;

        //FScopeLock Lock(&Mutex);
        FMazeCell& Cell = MazeGrid[Current.X][Current.Y];

        Cell.bVisited = true;

        TArray<FIntPoint> Neighbors;

        if (Current.X > StartX) Neighbors.Add(FIntPoint(Current.X - 1, Current.Y));
        if (Current.X + 1 < EndX) Neighbors.Add(FIntPoint(Current.X + 1, Current.Y));
        if (Current.Y > StartY) Neighbors.Add(FIntPoint(Current.X, Current.Y - 1));
        if (Current.Y + 1 < EndY) Neighbors.Add(FIntPoint(Current.X, Current.Y + 1));

        Algo::RandomShuffle(Neighbors);

        for (const FIntPoint& Next : Neighbors)
        {
            //FScopeLock Lock(&Mutex);
            FMazeCell& NextCell = MazeGrid[Next.X][Next.Y];
            if (!NextCell.bVisited)
            {
                // Удаляем стену между текущей и следующей
                if (Next.X == Current.X + 1)
                {
                    Cell.bWallRight = false;
                    NextCell.bWallLeft = false;
                }
                else if (Next.X == Current.X - 1)
                {
                    Cell.bWallLeft = false;
                    NextCell.bWallRight = false;
                }
                else if (Next.Y == Current.Y + 1)
                {
                    Cell.bWallBottom = false;
                    NextCell.bWallTop = false;
                }
                else if (Next.Y == Current.Y - 1)
                {
                    Cell.bWallTop = false;
                    NextCell.bWallBottom = false;
                }

                Stack.Add(Current); // вернёмся назад
                Stack.Add(Next);    // углубимся вперёд
                break;
            }
        }
    }
}

void UMazeGenerator::MergeSectionBorders()
{
    // Простая реализация: соединяем соседние секции в рандомных местах
    for (int x = MazeSectionSize; x < MazeWidth; x += MazeSectionSize)
    {
        for (int y = 0; y < MazeHeight; y += MazeSectionSize)
        {
            int rand_y = y + FMath::RandRange(0, FMath::Min(MazeSectionSize, MazeHeight - y) - 1);
            MazeGrid[x][rand_y].bWallLeft = false;
            MazeGrid[x - 1][rand_y].bWallRight = false;
        }
    }

    for (int y = MazeSectionSize; y < MazeHeight; y += MazeSectionSize)
    {
        for (int x = 0; x < MazeWidth; x += MazeSectionSize)
        {
            int rand_x = x + FMath::RandRange(0, FMath::Min(MazeSectionSize, MazeWidth - x) - 1);
            MazeGrid[rand_x][y].bWallTop = false;
            MazeGrid[rand_x][y - 1].bWallBottom = false;
        }
    }
}