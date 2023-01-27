#include "GridManager.h"

#include "GameManager.h"
#include "MovableObject.h"
#include "MovableObjectFactory.h"
#include "RectangleObject.h"
#include "RepairableObject.h"

GridManager* GridManager::instance_ = nullptr;

void GridManager::PrintGridManager() const
{
	std::cout << " -------------------------------------------------------------- " << std::endl;
	for (int y = gridHeight_ - 1; 0 <= y ; y--)
	{
		for (int x = 0; x < gridWidth_; x++)
		{
			std::cout << (grid_[y][x] != nullptr) << " ";
		}
		std::cout << std::endl;
	}
}

void GridManager::NotifyGridManager(MovableObject* caller, const Vector3& worldPosition)
{
	Vector2i gridPosition = ConvertWorldPositionToGridPosition(worldPosition);

	if (gridHeight_ <= gridPosition.y)
	{
		return;
	}

	if (gridPosition.y == 0 || grid_[gridPosition.y - 1][gridPosition.x] != nullptr && !(grid_[gridPosition.y - 1][gridPosition.x]->canFall_ && grid_[gridPosition.y - 1][gridPosition.x]->isMovementEnded_))
	{
		caller->StopMovement();
		SetGridPosition(caller, gridPosition);
		CheckAndSetObjectParent(caller, gridPosition);

		if (currentObject_ == caller)
		{
			currentObject_ = nullptr;
			MovableObjectFactory::GetInstance()->CreateMovableObject();
		}
	}
}

bool GridManager::IsGridEmpty(const Vector3& worldPosition)
{
	Vector2i gridPosition = ConvertWorldPositionToGridPosition(worldPosition);

	if (gridHeight_ <= gridPosition.y)
	{
		return false;
	}

	return 0 <= gridPosition.x && gridPosition.x < gridWidth_ && grid_[gridPosition.y][gridPosition.x] == nullptr;
}

GridManager::GridManager() : gridWidth_(9), gridHeight_(16)
{
	grid_ = new MovableObject**[gridHeight_];
	currentObject_ = nullptr;

	for (int y = 0; y < gridHeight_; y++)
	{
		grid_[y] = new MovableObject*[gridWidth_];

		for (int x = 0; x < gridWidth_; x++)
		{
			grid_[y][x] = nullptr;
		}
	}
}

GridManager::~GridManager()
{
	for (int y = 0; y < gridHeight_; y++)
	{
		delete[] grid_[y];
	}
	delete[] grid_;
}

void GridManager::CheckAndSetObjectParent(MovableObject* caller, const Vector2i& gridPosition)
{
	bool isRepaired = false;
	if (caller->GetType() == 0)
	{
		if (gridPosition.x + 1 < gridWidth_ && grid_[gridPosition.y][gridPosition.x + 1] != nullptr && grid_[gridPosition.y][gridPosition.x + 1]->GetType() == 1)
		{
			caller->SetParent(grid_[gridPosition.y][gridPosition.x + 1]->GetParent());
			caller->GetParent()->AddPartToRectangle(caller);
		}
		else
		{
			caller->SetParent(new RectangleObject());
			caller->GetParent()->AddPartToRectangle(caller);
		}
	}
	else if (caller->GetType() == 1)
	{
		if (gridPosition.x > 0 && grid_[gridPosition.y][gridPosition.x - 1] != nullptr && grid_[gridPosition.y][gridPosition.x - 1]->GetType() == 0)
		{
			caller->SetParent(grid_[gridPosition.y][gridPosition.x - 1]->GetParent());
			caller->GetParent()->AddPartToRectangle(caller);
		}
		else 
		{
			caller->SetParent(new RectangleObject());
			caller->GetParent()->AddPartToRectangle(caller);
		}
	}
	else if (caller->GetType() == 2)
	{
		if (gridPosition.y > 0 && grid_[gridPosition.y - 1][gridPosition.x] != nullptr && grid_[gridPosition.y - 1][gridPosition.x]->GetType() == 0)
		{
			caller->SetParent(grid_[gridPosition.y - 1][gridPosition.x]->GetParent());
			caller->GetParent()->AddPartToRectangle(caller);
		}
	}
	else if (caller->GetType() == 3)
	{
		if (gridPosition.y > 0 && grid_[gridPosition.y - 1][gridPosition.x] != nullptr && grid_[gridPosition.y - 1][gridPosition.x]->GetType() == 1)
		{
			caller->SetParent(grid_[gridPosition.y - 1][gridPosition.x]->GetParent());
			caller->GetParent()->AddPartToRectangle(caller);
		}
	}
}

void GridManager::ClearGrid()
{
	for (int y = 0; y < gridHeight_; y++)
	{
		for (int x = 0; x < gridWidth_; x++)
		{
			if (grid_[y][x] == nullptr)
			{
				continue;
			}

			grid_[y][x]->Destroy();
			grid_[y][x] = nullptr;
		}
	}
}

void GridManager::ClearRepairedObject(MovableObject* caller)
{
	if (caller != nullptr && caller->GetParent() != nullptr)
	{
		RepairableObject* callerParent = caller->GetParent();
		for (int y = gridHeight_ - 1; 0 <= y; y--)
		{
			for (int x = 0; x < gridWidth_; x++)
			{
				if (grid_[y][x] == caller) {
					grid_[y][x] = nullptr;
					if (caller->GetType() == 2 || caller->GetType() == 3)
					{
						for (int i = y + 1; i < gridHeight_ - 1; i++)
						{
							if (grid_[i][x] != nullptr) {
								
								grid_[i][x]->ResetFallSpeed();
								grid_[i][x] = nullptr;
							}
						}
					}
					caller->Destroy();
					return;
				}
			}
		}
	}
}

Vector2i GridManager::ConvertWorldPositionToGridPosition(const Vector3& worldPosition)
{
	return Vector2i(mathClamp(floor(worldPosition.x) + 4, -1, gridWidth_), floor(worldPosition.y));
}

void GridManager::SetGridPosition(MovableObject* caller, const Vector2i& gridPosition)
{
	if (gridPosition.y == gridHeight_ - 1)
	{
		GameManager::GetInstance()->GameOver();
	}

	grid_[gridPosition.y][gridPosition.x] = caller;
}
