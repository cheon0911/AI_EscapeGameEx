// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainClass/BaseItem.h"
#include "GoldBar.generated.h"

/**
 * 
 */
UCLASS()
class AI_ESCAPEGAMEEX_API AGoldBar : public ABaseItem
{
	GENERATED_BODY()
	
public:
	AGoldBar();

	virtual void BeginPlay() override;
	
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

private:
	// 모든 코인 수집여부 확인
	bool IsAllCoinsCollected() const;


};
