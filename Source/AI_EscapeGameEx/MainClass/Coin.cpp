// Fill out your copyright notice in the Description page of Project Settings.


#include "MainClass/Coin.h"
#include "AI_EscapeGameExCharacter.h"
#include "DetectableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

ACoin::ACoin()
{
	// 코인타입 설정
	ItemType = 0;
	// 2층 기준값 재설정. 부모클래스에 있지만 명시적으로 재설정.
	SecondFloor = 400.f;

}

void ACoin::BeginPlay()
{
	Super::BeginPlay();

	// 2층여부 확인
	CheckFloorLevel();
}

void ACoin::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	// 플레이어 확인
	AAI_EscapeGameExCharacter* PlayerCharacter = Cast<AAI_EscapeGameExCharacter>(OtherActor);
	if (!PlayerCharacter)
	{
		return;
	}

	// GameMode에 획득 보고
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(GetWorld());
	if (GameMode)
	{
		IGameRulesInterface* GameRulesInterface = Cast<IGameRulesInterface>(GameMode);
		if (GameRulesInterface)
		{
			GameRulesInterface->Execute_ReportGetCoin(GameMode, ItemType);
		}

		// 딜레이 후 액터 제거
		FTimerHandle DestroyTimerHandle;
		GetWorldTimerManager().SetTimer(DestroyTimerHandle, [this]() {
			Destroy(); }, 0.5f, false);

	}
}
