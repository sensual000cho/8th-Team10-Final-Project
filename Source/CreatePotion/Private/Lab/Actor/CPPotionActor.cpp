// CPPotionActor.cpp

#include "Lab/Actor/CPPotionActor.h"

#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/CPForageableItemData.h"
#include "Data/CPTagDefinitionTypes.h"
#include "Engine/DataTable.h"
#include "GameFramework/Pawn.h"
#include "Items/Potion/CPPotionImpactComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Settings/CPDTSettings.h"

ACPPotionActor::ACPPotionActor():
    ColorTransitionDuration(1.f), 
    ColorHoldDuration(2.f), 
    TransitionNeutralizeStrength(0.2f), 
    ColorIndex(0), 
    RemainingTime(0.f), 
    bTransitioning(false), 
    bExplosionTriggered(false)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    PotionImpactComponent = CreateDefaultSubobject<UCPPotionImpactComponent>(TEXT("PotionImpact"));
}

void ACPPotionActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdatePotionVisual(DeltaTime);
}

FName ACPPotionActor::GetInteractionName_Implementation()
{
    return WorkingIngredient.SourceItemData ? 
        FName(*WorkingIngredient.SourceItemData->DisplayName.ToString()) : Super::GetInteractionName_Implementation();   
}

void ACPPotionActor::InitializeFromItemData(UCPForageableItemData* ItemData, const TArray<FGameplayTag>& EffectTags)
{
    Super::InitializeFromItemData(ItemData, EffectTags);
    ApplyPotionVisual();
}

void ACPPotionActor::ApplySpawnImpulse(const FVector& SpawnImpulse)
{
    if (!IsValid(StaticMeshComponent)) return;
    
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StaticMeshComponent->SetGenerateOverlapEvents(true);
    StaticMeshComponent->SetLinearDamping(PostImpactLinearDamping);
    StaticMeshComponent->SetAngularDamping(PostImpactAngularDamping);
    StaticMeshComponent->SetSimulatePhysics(true);
    StaticMeshComponent->WakeAllRigidBodies();
    StaticMeshComponent->AddImpulse(SpawnImpulse);
    
    SetPropState(ECPThrowablePropState::Dropped);
    StartRestCheck();
}

void ACPPotionActor::HandleThrowStarted(AActor* Thrower)
{
    Super::HandleThrowStarted(Thrower);
    if (!IsValid(PotionImpactComponent)) return;

    APawn* ThrowingPawn = Cast<APawn>(Thrower);
    if (!IsValid(ThrowingPawn)) return;
    
    // Impact 처리
    PotionImpactComponent->EnableImpactProcessing(ThrowingPawn);
}

void ACPPotionActor::HandleThrownImpact(AActor* OtherActor, const FHitResult& HitResult)
{
    Super::HandleThrownImpact(OtherActor, HitResult);

    if (bExplosionTriggered || !IsValid(PotionImpactComponent)) return;
    
    // 플레이어 제외
    if (IsValid(OtherActor) && OtherActor == LastThrower.Get()) return;

    // 포션 컴포넌트에서 첫 Impact시 폭발로 넘어감
    if (!PotionImpactComponent->TryTriggerPotionImpact(HitResult, GetWorkingIngredient().CurrentEffects)) return;

    TriggerPotionExplosion(HitResult);
}

void ACPPotionActor::TriggerPotionExplosion(const FHitResult& HitResult)
{
    if (bExplosionTriggered) return;
    bExplosionTriggered = true;

    // Base의 정지 확인 타이머 중단
    StopRestCheck();

    if (IsValid(PotionImpactComponent))
    {
        PotionImpactComponent->DisableImpactProcessing();
    }

    if (IsValid(StaticMeshComponent))
    {
        StaticMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
        StaticMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        StaticMeshComponent->SetSimulatePhysics(false);
        StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        StaticMeshComponent->SetGenerateOverlapEvents(false);
        StaticMeshComponent->SetVisibility(false, true);
    }

    SetActorEnableCollision(false);

    // 이펙트 생성 지점(나이아가라, 사운드)
    K2_OnPotionExploded(HitResult);
    Destroy();
}

void ACPPotionActor::ApplyPotionVisual()
{
    UNiagaraComponent* LiquidNiagaraComponent = 
        Cast<UNiagaraComponent>(LiquidNiagaraComponentReference.GetComponent(this));
    if (!IsValid(LiquidNiagaraComponent) || !IsValid(LiquidMaterial) || WorkingIngredient.CurrentEffects.IsEmpty()) return;
    
    const UCPDTSettings* DTSettings = GetDefault<UCPDTSettings>();
    if (!DTSettings) return;
    
    UDataTable* TagDefinitionTable = DTSettings->TagDefinitionTable.LoadSynchronous();
    if (!TagDefinitionTable) return;   
    
    TArray<FCPTagDefinitionRow*> Rows;
    TagDefinitionTable->GetAllRows<FCPTagDefinitionRow>(TEXT("PotionVisualTagDefinition"), Rows);
    
    VisualColors.Reset();
    
    // 보유한 Tag 별 Color 저장
    for (const FGameplayTag& Tag : WorkingIngredient.CurrentEffects){
        if (!Tag.IsValid()) continue;
        
        for (const FCPTagDefinitionRow* Row : Rows){
            if (!Row || Row->Tag != Tag) continue;
            
            VisualColors.Add({Row->LiquidColor01, Row->LiquidColor02, Row->SurfaceColor01});
            break;
        }
    }
    
    DynamicLiquidMaterial = UMaterialInstanceDynamic::Create(LiquidMaterial, this);
    if (!IsValid(DynamicLiquidMaterial)) return;

    CurrentVisualColors = VisualColors[0];

    ApplyLiquidMaterialColors();
    LiquidNiagaraComponent->SetVariableMaterial(TEXT("User.Material"), DynamicLiquidMaterial);

    SetActorTickEnabled(WorkingIngredient.CurrentEffects.Num() > 1);
}

void ACPPotionActor::UpdatePotionVisual(float DeltaTime)
{
    // MI가 없거나 Tag(색)가 1개인 경우
    if (!IsValid(DynamicLiquidMaterial) || VisualColors.Num() <= 1){
        SetActorTickEnabled(false);
        return;
    }

    RemainingTime -= DeltaTime;
    
    if (!bTransitioning){
        if (RemainingTime > 0.f) return;
        
        RemainingTime = ColorTransitionDuration;
        bTransitioning = true;
    }

    const int32 NextColorIndex = (ColorIndex + 1) % VisualColors.Num();
    
    const FCPPotionVisualColors& FromColor = VisualColors[ColorIndex];
    const FCPPotionVisualColors& ToColor = VisualColors[NextColorIndex];
    
    const float Alpha = ColorTransitionDuration > 0.f
        ? FMath::Clamp(1.f - RemainingTime / ColorTransitionDuration, 0.f, 1.f) : 1.f;
    
    // 2가지 Tag 색 변경
    CurrentVisualColors.LiquidColor01 = FMath::Lerp(FromColor.LiquidColor01, ToColor.LiquidColor01, Alpha);
    CurrentVisualColors.LiquidColor02 = FMath::Lerp(FromColor.LiquidColor02, ToColor.LiquidColor02, Alpha);
    CurrentVisualColors.SurfaceColor01 = FMath::Lerp(FromColor.SurfaceColor01, ToColor.SurfaceColor01, Alpha);

    const float NeutralizeAlpha = FMath::Sin(Alpha * PI) * TransitionNeutralizeStrength;

    // 색의 휘도 값
    const float LiquidColor01Gray = CurrentVisualColors.LiquidColor01.GetLuminance();
    const float LiquidColor02Gray = CurrentVisualColors.LiquidColor02.GetLuminance();
    const float SurfaceColor01Gray = CurrentVisualColors.SurfaceColor01.GetLuminance();

    // 색 전환 시 채도를 살짝 낮춤
    CurrentVisualColors.LiquidColor01 = FLinearColor(
        FMath::Lerp(CurrentVisualColors.LiquidColor01.R, LiquidColor01Gray, NeutralizeAlpha),
        FMath::Lerp(CurrentVisualColors.LiquidColor01.G, LiquidColor01Gray, NeutralizeAlpha),
        FMath::Lerp(CurrentVisualColors.LiquidColor01.B, LiquidColor01Gray, NeutralizeAlpha),
        CurrentVisualColors.LiquidColor01.A);

    CurrentVisualColors.LiquidColor02 = FLinearColor(
        FMath::Lerp(CurrentVisualColors.LiquidColor02.R, LiquidColor02Gray, NeutralizeAlpha),
        FMath::Lerp(CurrentVisualColors.LiquidColor02.G, LiquidColor02Gray, NeutralizeAlpha),
        FMath::Lerp(CurrentVisualColors.LiquidColor02.B, LiquidColor02Gray, NeutralizeAlpha),
        CurrentVisualColors.LiquidColor02.A);

    CurrentVisualColors.SurfaceColor01 = FLinearColor(
        FMath::Lerp(CurrentVisualColors.SurfaceColor01.R, SurfaceColor01Gray, NeutralizeAlpha),
        FMath::Lerp(CurrentVisualColors.SurfaceColor01.G, SurfaceColor01Gray, NeutralizeAlpha),
        FMath::Lerp(CurrentVisualColors.SurfaceColor01.B, SurfaceColor01Gray, NeutralizeAlpha),
        CurrentVisualColors.SurfaceColor01.A);

    ApplyLiquidMaterialColors();

    if (RemainingTime <= 0.f)
    {
        ColorIndex = NextColorIndex;
        CurrentVisualColors = VisualColors[ColorIndex];

        ApplyLiquidMaterialColors();

        RemainingTime = ColorHoldDuration;
        bTransitioning = false;
    }
}

void ACPPotionActor::ApplyLiquidMaterialColors() const
{
    if (!IsValid(DynamicLiquidMaterial)) return;

    DynamicLiquidMaterial->SetVectorParameterValue(TEXT("Liquid_Color01"), CurrentVisualColors.LiquidColor01);
    DynamicLiquidMaterial->SetVectorParameterValue(TEXT("Liquid_Color02"), CurrentVisualColors.LiquidColor02);
    DynamicLiquidMaterial->SetVectorParameterValue(TEXT("Surface_Color01"), CurrentVisualColors.SurfaceColor01);
}
