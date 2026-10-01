// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/Subsystem/CPCauldronSubsystem.h"

#include "Lab/Component/Interact/CPCauldronComponent.h"

void UCPCauldronSubsystem::RegisterCauldron(UCPCauldronComponent* InCauldronComponent)
{
	if (!IsValid(InCauldronComponent)) return;
	
	ActiveCauldronComponent = InCauldronComponent;
	if (bHasCauldronData){
		InCauldronComponent->LoadSaveData(CauldronData);
	}
}

void UCPCauldronSubsystem::UnRegisterCauldron(UCPCauldronComponent* InCauldronComponent)
{
	if (!IsValid(InCauldronComponent) || ActiveCauldronComponent.Get() != InCauldronComponent) return;
	
	CauldronData = InCauldronComponent->GetSaveData();
	bHasCauldronData = true;
	
	ActiveCauldronComponent.Reset();
}

FCPCauldronSaveData UCPCauldronSubsystem::GetSaveData()
{
	if (ActiveCauldronComponent.IsValid()){
		CauldronData = ActiveCauldronComponent->GetSaveData();
		bHasCauldronData = true;
	}
	
	return CauldronData;
}

void UCPCauldronSubsystem::LoadSaveData(const FCPCauldronSaveData& InSaveData)
{
	CauldronData = InSaveData;
	bHasCauldronData = true;
	
	ActiveCauldronComponent.Reset();
}

void UCPCauldronSubsystem::ResetData()
{
	CauldronData = FCPCauldronSaveData{};
	ActiveCauldronComponent.Reset();
	bHasCauldronData = false;
}
