// CPContextInventoryOnly.cpp

#include "UI/Context/CPContextInventoryOnly.h"
#include "CreatePotion.h"				// 로그

// 좌클릭인 경우
// [Ctrl + 좌클릭] 아이템 빠른 이동 (Quick Transfer)
// [Shift + 좌클릭] 아이템 나누기 (Split)
// [Alt + 좌클릭] 아이템 버리기 (Drop)
// [일반 좌클릭] '드래그 앤 드롭(Drag & Drop)'
// 우클릭인 경우
// [Ctrl + 우클릭] 아이템 사용 (Use Item)
// [Shift + 우클릭] 아이템 세부 정보 표시 (Show Item Detail)
// [Alt + 우클릭] 장비 아이템 장착 (Equip Item)
// [일반 우클릭] [LCtrl, Shift, Alt] + [좌, 우 클릭] 으로 가능한 6가지 기능을 UI 통합 처리

bool UCPContextInventoryOnly::HandleLeftClickOnly(UCPItemSlotWidget* ClickedSlot)
{
	// TODO[Container] : 아이템 집기 = 마우스가 움직이는대로 아이콘을 임시로 잡아서 들고있기
	// 좌클릭마다 토글 형태로 아이템 집기, 배치 기능이 변경되며
	// 아이템을 집어 빈 칸에 배치시 해당 칸으로 아이템 index 변경
	UE_LOG(LogContainer, Warning, TEXT("Container Context Test"));
	return false;
}
