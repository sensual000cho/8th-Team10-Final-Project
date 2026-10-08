// CPWidgetLocationPlacer.cpp

#include "UI/Widgets/Base/CPWidgetLocationPlacer.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

FVector2D CPWidgetLocationPlacer::CalcPositionNearCursor(UUserWidget* Widget, const FVector2D& CursorPos, const FVector2D& Offset)
{
	if (!Widget)
	{
		return CursorPos + Offset;
	}

	// DPI 배율로 나눠 다양한 해상도에 맞추기
	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(Widget);
	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(Widget) / FMath::Max(ViewportScale, KINDA_SMALL_NUMBER);

	// 팝업 크기 계산
	Widget->ForceLayoutPrepass();
	const FVector2D WidgetSize = Widget->GetDesiredSize();

	FVector2D Pos = CursorPos + Offset;

	// 오른쪽이 넘치면 커서 왼쪽으로 뒤집기
	if (Pos.X + WidgetSize.X > ViewportSize.X)
	{
		Pos.X = CursorPos.X - Offset.X - WidgetSize.X;
	}

	// 아래가 넘치면 커서 위쪽으로 뒤집기
	if (Pos.Y + WidgetSize.Y > ViewportSize.Y)
	{
		Pos.Y = CursorPos.Y - Offset.Y - WidgetSize.Y;
	}

	// 그래도 넘치는 경우(팝업이 화면보다 크거나 모서리 근처)를 대비해 최종적으로 화면 안에 고정
	Pos.X = FMath::Clamp(Pos.X, 0.f, FMath::Max(0.f, ViewportSize.X - WidgetSize.X));
	Pos.Y = FMath::Clamp(Pos.Y, 0.f, FMath::Max(0.f, ViewportSize.Y - WidgetSize.Y));

	return Pos;
}
