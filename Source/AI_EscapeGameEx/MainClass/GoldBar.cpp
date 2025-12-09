#include "MainClass/GoldBar.h"
#include "MainClass/Coin.h"
#include "AI_EscapeGameExCharacter.h"
#include "DetectableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

AGoldBar::AGoldBar()
{
	// 골드바 타입 설정
	ItemType = 1;
}

void AGoldBar::BeginPlay()
{
	Super::BeginPlay();

}

bool AGoldBar::IsAllCoinsCollected() const
{
	TArray<AActor*>FoundCoins;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACoin::StaticClass(), FoundCoins);

	// 찾은 코인이 0개여야 모든 코인을 획득한것이기 때문
	return FoundCoins.Num() == 0;
}


void AGoldBar::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	// 플레이어 확인
	AAI_EscapeGameExCharacter* PlayerCharacter = Cast<AAI_EscapeGameExCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		return;
	}

	// 모든 코인이 수집되었는지 확인
	if (IsAllCoinsCollected())
	{
		// GameMode에 획득 보고
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(GetWorld());
		if (GameMode)
		{
			IGameRulesInterface* GameRulesInterface = Cast<IGameRulesInterface>(GameMode);
			if (GameRulesInterface)
			{
				GameRulesInterface->Execute_ReportGetCoin(GameMode, ItemType);
			}
		}
	

	// 골드바 숨기기 및 충돌 비활성화
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NotAllCoinsCollected"));
	}

}
