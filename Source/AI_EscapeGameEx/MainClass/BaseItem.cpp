// Fill out your copyright notice in the Description page of Project Settings.


#include "MainClass/BaseItem.h"
#include "BaseItem.h"
#include "AI_EscapeGameEx/AI_EscapeGameExCharacter.h"
#include "DetectableInterface.h"
#include "GameFramework/GameModeBase.h"

// Sets default values
ABaseItem::ABaseItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 컴포넌트 생성
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionSphere;

	// 충돌 설정
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetSphereRadius(50.f);

	// 메시 컴포넌트 생성
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	
	// 기본 아이템 타입
	ItemType = 0; // 골드바는 예외적 1 설정

	// 층수 확인하는 변수
	SecondFloor = 400.f;
	bIsOn2F = false;

}

// Called when the game starts or when spawned
void ABaseItem::BeginPlay()
{
	Super::BeginPlay();
	
	// 오버랩 이벤트 바인딩
	// 언리얼 오버랩채널을 구독하는 수신자가 됨
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseItem::OnOverlapBegin);

}

void ABaseItem::CheckFloorLevel()
{
	FVector CurrentLocation = GetActorLocation();
	bIsOn2F = CurrentLocation.Z >= SecondFloor;
}

// Called every frame
void ABaseItem::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);

}

// 오버랩시 일어나는 일 작성
void ABaseItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("## Collect Event Triggered!! ##"));

	// 플레이어 캐릭터인지 확인
	AAI_EscapeGameExCharacter* PlayerCharacter = Cast<AAI_EscapeGameExCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		// 게임모드 가져오기
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(GetWorld());

		// GameMode가 GameRulesInterface를 구현했는지 확인
		if (GameMode)
		{
			// 인터페이스 포인터로 변환
			IGameRulesInterface* GameRulesInterface = Cast<IGameRulesInterface>(GameMode);
			if (GameRulesInterface)
			{
				// 아이템 획득 소식 및 타입을 전달
				GameRulesInterface->Execute_ReportGetCoin(GameMode, ItemType);
				UE_LOG(LogTemp, Warning, TEXT("ITEM COLLECTED AND REPORTED"));

			}
		}
	}

	// 딜레이 후 액터 제거
	FTimerHandle DestroyTimerHandle;
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, [this]() {
		Destroy(); }, 0.5f, false);

}



