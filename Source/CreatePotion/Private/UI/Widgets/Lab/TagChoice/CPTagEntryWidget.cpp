#include "UI/Widgets/Lab/TagChoice/CPTagEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UCPTagEntryWidget::BindEvents()
{
	Super::BindEvents();

	if (Button_Tag)
	{
		Button_Tag->OnClicked.AddDynamic(this, &UCPTagEntryWidget::OnButtonClicked);
	}
}

void UCPTagEntryWidget::UnbindEvents()
{
	if (Button_Tag)
	{
		Button_Tag->OnClicked.RemoveDynamic(this, &UCPTagEntryWidget::OnButtonClicked);
	}

	Super::UnbindEvents();
}

void UCPTagEntryWidget::InitTagEntry(const FGameplayTag& InTag)
{
	TagData = InTag;
	if (Text_TagName)
	{
		// 태그의 전체 이름(예: Alchemy.Heal)을 가져와서
		FString DisplayName = TagData.GetTagName().ToString();

		// "Alchemy." 부분을 제거하고 UI에 표시
		DisplayName.RemoveFromStart(TEXT("Alchemy."));

		Text_TagName->SetText(FText::FromString(DisplayName));
	}
}

void UCPTagEntryWidget::OnButtonClicked()
{
	OnTagSelected.Broadcast(TagData);
}