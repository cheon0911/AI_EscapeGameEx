// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainClass/BaseItem.h"
#include "Coin.generated.h"

/**
 * 
 */
UCLASS()
class AI_ESCAPEGAMEEX_API ACoin : public ABaseItem
{
	GENERATED_BODY()
	
public:
	ACoin();

	virtual void BeginPlay() override;

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

};
