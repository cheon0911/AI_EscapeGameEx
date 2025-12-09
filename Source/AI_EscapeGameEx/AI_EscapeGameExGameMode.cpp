#include "AI_EscapeGameExGameMode.h"
#include "AI_EscapeGameExCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AAI_EscapeGameExGameMode::AAI_EscapeGameExGameMode()
{
	// 게임 틱 활성화
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// 게임 시간 초기화
	RemainingTime = MaxGameTime;

	// 기본 폰클래스 세팅
	static ConstructorHelpers::FClassFinder<APawn>PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

}

void AAI_EscapeGameExGameMode::StartPlay()
{
	Super::StartPlay();

	// 디버그 
	UE_LOG(LogTemp, Warning, TEXT("[GM] StartPlay() - Class: %s"), *GetClass()->GetName());

	// 층별 데이터 초기화


	// 게임 타이머 시작


	// Menu->Game의 루트를 따를 경우, Input전환 필요
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

	if (PC)
	{
		// 게임 입력모드 설정
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	// 레벨명이 게임 오버 등과 관련된 경우가 아닌 때에만 UI를 표시하도록 디자인.
	FString CurrentLevelName = GetWorld()->GetMapName();
	CurrentLevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix); // 접두사 제거. ex) Lv1.Example -> Example 앞의 레벨이 없어지고 Name만 가져오는것 
	if (!CurrentLevelName.Contains("GameOver")) // 포함하고 있는지
	{
		ShowGameStartWidget();
	}

}

void AAI_EscapeGameExGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UE_LOG(LogTemp, Error, TEXT("CurrentCaptureCount = %d / Max = %d"),
		CurrentCaptureCount,
		MaxCaptureCount
	);

	// 게임 진행중에만 타이머 감소하도록 디자인
	if (!bGameEnded)
	{
		RemainingTime -= DeltaTime;

		// 1초마다 이벤트 발생시키지 않고 Tick마다 시간 업데이트
		// 옵저버 패턴의 Broadcast 파트
		OnTimeUpdated.Broadcast(RemainingTime);

		// 시간초과 되었을 경우 로직 작성
		if (RemainingTime <= 0.f)
		{
			RemainingTime = 0.f;
			// 게임종료 조건 더블체크
			if (CheckGameOverCondition())
			{
				// 게임 종료
				TriggerGameOver();
			}

		}

		if (CurrentCaptureCount >= MaxCaptureCount)
		{
			TriggerGameOver();
			UE_LOG(LogTemp, Error, TEXT("CurrentCaptureCount >= MaxCaptureCount  Tick"));
			return;
		}

		// 승리조건 충족여부 확인 및 처리
		if (CheckWinCondition())
		{
			// 게임 승리
			TriggerGameWin();
		}
	}

}

void AAI_EscapeGameExGameMode::CollectCoin(int32 ItemType)
{
	// 아이템이 금괴 및 코인으로 두개이므로 타입 확인 진행
	if (ItemType < 0 || ItemType > 1)
	{
		return;
	}

	// 일반 코인이거나 골드바인 경우 아래 로직 실행
	CollectedCoins++;
	OnCoinCollected.Broadcast(ItemType);

	if (ItemType == 1) // 0: 코인 1: 골드바  1번 골드바 이면
	{
		// 골드바 획득
		bIsGoldBarCollected = true;
	}

	// 승리조건 확인
	if (CheckWinCondition())
	{
		TriggerGameWin();
	}

}

bool AAI_EscapeGameExGameMode::CheckWinCondition()
{
	// 모든 코인과 골드바를 수집했는지 확인
	return CollectedCoins >= TotalCoins && bIsGoldBarCollected;
}

bool AAI_EscapeGameExGameMode::CheckGameOverCondition()
{
	//게임 오버 조건 : 시간초과 혹은 3회 체포
	return RemainingTime <= 0.f || CurrentCaptureCount >= MaxCaptureCount;
}

void AAI_EscapeGameExGameMode::TriggerGameOver()
{
	UE_LOG(LogTemp, Error, TEXT("TriggerGameOver Trigged"));
	if (bGameEnded)
	{
		return;
	}

	bGameEnded = true;

	// 게임 타이머 중지
	GetWorldTimerManager().ClearTimer(GameTimerHandle);

	// 게임 오버 이벤트 발생
	OnGameOver.Broadcast();

	// 플레이어 컨트롤러에 게임 오버 알림
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		bIsPlayerWin = false;

		// 우선 일시정지로 코드작성. 추후 필요시 기능 추가.
		PC->SetPause(true);
		DisableInput(PC);

		// 게임오버 UI위젯 생성 및 표시
		if (GameOverWidgetClass != nullptr)
		{
			GameOverWidget = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
			UE_LOG(LogTemp, Error, TEXT("GameOverWidget = %s"), GameOverWidget ? TEXT("OK") : TEXT("NULL"));
			if (GameOverWidget)
			{
				GameOverWidget->AddToViewport();

				// 마우스로 입력 받으려면 아래 코드 사용
				PC->SetInputMode(FInputModeUIOnly());
				PC->bShowMouseCursor = true;
			}
		}

	}

}

void AAI_EscapeGameExGameMode::TriggerGameWin()
{
	// Player가 모든 코인을 시간 내에 탈취했을 때 승리

	if (bGameEnded)
	{
		return;
	}

	bGameEnded = true;

	// 게임 타이머 중지
	GetWorldTimerManager().ClearTimer(GameTimerHandle);

	// 게임 승리 이벤트 발생
	OnGameWin.Broadcast();

	// 게임 승리 알림
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		bIsPlayerWin = true;

		// 우선 일시정지로 코드작성. 추후 필요시 기능 추가.
		PC->SetPause(true);
		//DisableInput(PC);
		UE_LOG(LogTemp, Error, TEXT("You Win!!!!!!!!!!!"));
		// 게임오버 UI위젯 생성 및 표시,  추후 게임 승리 UI 텍스트 추가로직 구현 필요
		if (GameOverWidgetClass != nullptr)
		{
			GameOverWidget = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
			if (GameOverWidget)
			{
				GameOverWidget->AddToViewport();

				// 마우스로 입력 받으려면 아래 코드 사용
				PC->SetInputMode(FInputModeUIOnly());
				PC->bShowMouseCursor = true;
			}
		}

	}
}

void AAI_EscapeGameExGameMode::NotifyFloorChange(int32 NewFloor)
{
	// 유효한 층 번호 확인
	if (NewFloor < 1 || NewFloor > 3)
	{
		return;
	}

	// 현재 층 업데이트
	CurrentPlayerFloor = NewFloor;

}

void AAI_EscapeGameExGameMode::ShowGameStartWidget()
{
	RemoveCurrentWidget();
	// 메인 메뉴 위젯 생성 및 표시
	if (GameStartWidgetClass)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			CurrentWidtet = CreateWidget<UUserWidget>(PC, GameStartWidgetClass);
			if (CurrentWidtet)
			{
				CurrentWidtet->AddToViewport();

				// UI 입력모드 설정. 마우스 허용.
				FInputModeUIOnly InputMode;
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = true;				
			}
		}
	}
}

void AAI_EscapeGameExGameMode::ShowGameOverWidget()
{
	RemoveCurrentWidget();

	// 메인메뉴 위젯 생성 및 표시. 추후 게임 승리 및 패배 세부 UI 배치 로직 추가.
	if (GameOverWidgetClass)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			CurrentWidtet = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
			if (CurrentWidtet)
			{
				CurrentWidtet->AddToViewport();

				// UI 입력모드 설정. 마우스 허용.
				FInputModeUIOnly InputMode;
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = true;
			}
		}
	}
}

void AAI_EscapeGameExGameMode::RemoveCurrentWidget()
{
	if (CurrentWidtet)
	{
		CurrentWidtet->RemoveFromParent();
		CurrentWidtet = nullptr;
	}
}

void AAI_EscapeGameExGameMode::QuitGame()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

	if (PC)
	{
		// 시스템 라이브러리 통해 게임종료
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, true);
	}
}

void AAI_EscapeGameExGameMode::UpdateGameTimer()
{
	// 매초마다 호출되는 타이머 업데이트 구현
	if (!bGameEnded)
	{
		UE_LOG(LogTemp, Warning, TEXT("Remaining Time is : %f second(s)"), RemainingTime);
		return;
	}

	// Broadcast 필요시 주석 해제
	//OnTimeUpdated.Broadcast(RemainingTime);

}

// IGameRulesInterface 구현부
void AAI_EscapeGameExGameMode::ReportPlayerCapture_Implementation(APawn* CapturePlayer, AActor* Captor)
{
	// 플레이어 체포시 보고 처리
	if (!bGameEnded)
	{
		CurrentCaptureCount++;
		// 이벤트 발생
		OnPlayerCaptured.Broadcast(CapturePlayer, Captor);
		//  게임 오버 조건 확인 (목숨이 0이 되면 바로 종료)
		if (CurrentCaptureCount >= MaxCaptureCount)
		{
			UE_LOG(LogTemp, Error, TEXT("Player life reached 0 → GameOver triggered"));
			TriggerGameOver();
			return;
		}

		// 게임오버 조건 확인
		if (CheckGameOverCondition())
		{
			TriggerGameOver();
		}

	}
}

void AAI_EscapeGameExGameMode::ReportPlayerDetection_Implementation(APawn* DetectedPlayer, AActor* Detector)
{
	// 플레이어 감지시 보고
	if (!bGameEnded)
	{
		// Broadcast이벤트 발생
		OnPlayerDetected.Broadcast(DetectedPlayer, Detector); // Subscriber 클래스가 없으면 방송해도 무의미.
		// 필요시 로직 추가
	}
}

void AAI_EscapeGameExGameMode::ReportAlertLevelChange_Implementation(AActor* Guard, uint8 NewAlertLevel)
{
	// AI 경계상태 보고 처리
	if (!bGameEnded)
	{
		// 이벤트 발생
		OnAlertLevelChanged.Broadcast(Guard, NewAlertLevel);
	}
}

void AAI_EscapeGameExGameMode::ReportGetCoin_Implementation(int32 type)
{
	if (!bGameEnded)
	{
		// 코인 획득처리 로직
		CollectCoin(type);
	}
}

// 남은 체력 반환 함수
int32 AAI_EscapeGameExGameMode::ReportCurrentLives_Implementation()
{
	if (!bGameEnded)
	{
		// 남은 체포기회 반환(최대 체포 횟수 - 현재 체포 횟수)
		return MaxCaptureCount - CurrentCaptureCount;
	}

	return 0;
}
