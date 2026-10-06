#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Quest/QuestTypes.h"
#include "CPQuestMarkerWidget.generated.h"

class UImage;
class UTexture2D;

UCLASS()
class CREATEPOTION_API UCPQuestMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetMarkerState(EQuestMarkerState NewState);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MarkerImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Marker")
	TObjectPtr<UTexture2D> AvailableTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Marker")
	TObjectPtr<UTexture2D> InProgressTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Marker")
	TObjectPtr<UTexture2D> CompletableTexture;
};