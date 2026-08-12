#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "GameplayTagContainer.h"
#include "CPHintWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class CREATEPOTION_API UCPHintWidget : public UCPBaseUserWidget 
{
	GENERATED_BODY()

public:
	void InitHintWidget(FName InQuestID);

protected:

	virtual void BindEvents() override;
	virtual void UnbindEvents() override;

private:
	void UpdateHintUI();

	UFUNCTION()
	void OnAskHintClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Hint;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_AskHint;

private:
	FName CurrentQuestID;
	int32 CurrentHintLevel = 0;
};