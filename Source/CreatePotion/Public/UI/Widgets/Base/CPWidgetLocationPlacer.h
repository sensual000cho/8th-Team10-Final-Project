// CPWidgetLocationPlacer.h

#pragma once

#include "CoreMinimal.h"

class CREATEPOTION_API CPWidgetLocationPlacer
{
public:
public:
	// 커서 기준으로 팝업 위치를 미리 계산하여 화면 밖으로 나가게 될 경우 반대쪽으로 뒤집는 기능
	static FVector2D CalcPositionNearCursor(UUserWidget* Widget, const FVector2D& CursorPos, const FVector2D& Offset);
};
