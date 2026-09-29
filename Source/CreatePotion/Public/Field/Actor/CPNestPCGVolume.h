#pragma once

#include "CoreMinimal.h"
#include "PCGVolume.h"
#include "CPNestPCGVolume.generated.h"

// 필드 위 둥지 PCG 생성 시드값 변경을 위한 PCG볼륨
UCLASS()
class CREATEPOTION_API ACPNestPCGVolume : public APCGVolume
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
private:
	void GenerateNests();
};
