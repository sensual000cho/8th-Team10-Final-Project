#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "GameplayTagContainer.h"
#include "CPTagEntryWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTagSelectedSignature, FGameplayTag, SelectedTag);

UCLASS()
class CREATEPOTION_API UCPTagEntryWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()

public:
	void InitTagEntry(const FGameplayTag& InTag);

	UPROPERTY(BlueprintAssignable, Category = "Tag")
	FOnTagSelectedSignature OnTagSelected;

protected:
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;

private:
	UFUNCTION()
	void OnButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Tag;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_TagName;

	FGameplayTag TagData;
};