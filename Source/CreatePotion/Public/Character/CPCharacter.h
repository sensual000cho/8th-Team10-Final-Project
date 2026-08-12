// CPCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CPCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UCPInteractionComponent;
struct FInputActionValue;

class UCPItemContainerComponent;
class UCPContainerMainWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class CREATEPOTION_API ACPCharacter : public ACharacter
{
	GENERATED_BODY()
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	
	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

protected:
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;
	
	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;
	
	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;
	
	/** Interaction Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InteractAction;
	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseInteractAction;
	
	/** QuestList Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* QuestToggleAction;
	
public:

	/** Constructor */
	ACPCharacter();	
	
protected:
	
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;
	
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();
	
public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
protected:
	UPROPERTY(VisibleAnywhere, Category="Interaction")
	UCPInteractionComponent* InteractionComponent;
	
	UPROPERTY(EditAnywhere, Category = "UI|Tags")
	FGameplayTag QuestToggleTag;
	
	void OnInteractPressed(); // 입력 바인딩용
	void OnQuestTogglePressed(); // 퀘스트 입력 바인딩

#pragma region Container
private:
	// 인벤토리 상호작용 키(I키 등)를 눌렀을 때 실행될 토글 함수
	void ToggleInventoryUI();

public:
	// 컨테이너 컴포넌트 - 인벤토리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	UCPItemContainerComponent* InventoryComponent;

	// 인벤토리 UI 위젯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UCPContainerMainWidget> InventoryUIClass;

protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ToggleInventoryAction;

private:
	// 생성된 위젯 인스턴스를 보관할 변수
	UPROPERTY()
	UCPContainerMainWidget* InventoryUIInstance;
#pragma endregion

};
