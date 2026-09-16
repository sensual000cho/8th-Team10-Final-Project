#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CPEnemyAttributeSet.generated.h"

#define CP_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/** Enemy가 사용하는 최소 체력 AttributeSet. 체력 범위는 0.0~1.0이다. */
UCLASS()
class CREATEPOTION_API UCPEnemyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UCPEnemyAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	CP_ATTRIBUTE_ACCESSORS(UCPEnemyAttributeSet, Health)

private:
	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Attribute", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Health;
};

#undef CP_ATTRIBUTE_ACCESSORS
