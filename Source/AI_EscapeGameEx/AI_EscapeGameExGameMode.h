#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DetectableInterface.h"
#include "Blueprint/UserWidget.h"
#include "AI_EscapeGameExGameMode.generated.h"

// 델리게이트 선언. 방송채널 여는것과 유사
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerCapturedSignature, APawn*, CapturedPlayer, AActor*, Captor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDetectedSignature, APawn*, DetectedPlayer, AActor*, Detector);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAlertLevelChangedSignature, AActor*, Guard, uint8, NewAlertLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinCollectedSignature, int32, FloorNumber); // ItemType  FloorNumber
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFloorCoinCollectedSignature, int32, FloorNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeUpdatedSignature, float, RemainingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOverSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameWinSignature);




UCLASS(abstract)
class AAI_EscapeGameExGameMode : public AGameModeBase, public IGameRulesInterface
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AAI_EscapeGameExGameMode();


	// 인터페이스 구현
	virtual void ReportPlayerCapture_Implementation(APawn* CapturePlayer, AActor* Captor) override;
	virtual void ReportPlayerDetection_Implementation(APawn* DetectedPlayer, AActor* Detector) override;
	virtual void ReportAlertLevelChange_Implementation(AActor* Guard, uint8 NewAlertLevel) override;
	virtual void ReportGetCoin_Implementation(int32 type) override;
	virtual int32 ReportCurrentLives_Implementation() override;
	
	// 게임 룰 관련 기본 속성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	float MaxGameTime = 120.f; // 2분

	// 런타임중 남은시간
	UPROPERTY(BlueprintReadOnly, Category = "Game Rules")
	float RemainingTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	int32 MaxCaptureCount = 3; // 최대 체포 가능 횟수, 3회 체포되면 게임 오버

	UPROPERTY(BlueprintReadOnly, Category = "Game Rules")
	int32 CurrentCaptureCount = 0; // 현재까지 잡힌 횟수

	// 총 몇 단계인지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	int32 TotalFloors = 2; // 2층 구조

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	int32 TotalCoins = 6; // 임시

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	bool bIsGoldBarCollected; // 금괴 획득 여부(승리요건)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	bool b2FCoinCollected; // 2층 코인 수집여부
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	bool bIsAllCoinsCollected; // 모든 코인을 수집했는지 확인

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
	int32 CollectedCoins = 0; // 현재 획득한 코인

	// 게임 이벤트 델리게이트. 특정 이벤트를 구독(AddDynamic)중인 클래스에게 방송하기 위함.
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnPlayerCapturedSignature OnPlayerCaptured;

	// 플레이어 Detected
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnPlayerDetectedSignature OnPlayerDetected;

	// 경계수준 Changed
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnAlertLevelChangedSignature OnAlertLevelChanged;

	// 코인 수집되었을 때
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnCoinCollectedSignature OnCoinCollected;

	// 특정 층의 코인이 모두 수집 되었을 때
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnFloorCoinCollectedSignature OnFloorCoinCollected;

	// 게임 시간이 종료되었을 때
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnTimeUpdatedSignature OnTimeUpdated;

	// 게임이 종료되었을 때
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnGameOverSignature OnGameOver;

	// 플레이어가 승리하였을 때
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnGameWinSignature OnGameWin;
	
	// 게임 승패로직관련 함수/ 변수
	// 게임 시작시 초기화
	virtual void StartPlay() override;

	// 게임 타이머 업데이트
	virtual void Tick(float DeltaTime) override;

	// 코인 수집 처리
	UFUNCTION(BlueprintCallable, Category = "GameRules")
	void CollectCoin(int32 ItemType);

	// 게임 승리조건 확인
	UFUNCTION(BlueprintCallable, Category = "GameRules")
	bool CheckWinCondition();

	// 게임 오버 확인
	UFUNCTION(BlueprintCallable, Category = "GameRules")
	bool CheckGameOverCondition();

	// 게임오버 처리 
	UFUNCTION(BlueprintCallable, Category = "GameRules")
	void TriggerGameOver();

	// 게임 승리 처리
	UFUNCTION(BlueprintCallable, Category = "GameRules")
	void TriggerGameWin();

	// 현재 층 알림(플레이어가 2층에서 모든 코인을 획득하고 다음 층으로 이동할 경우 체크할 수 있도록)
	UFUNCTION(BlueprintCallable, Category = "GameRules")
	void NotifyFloorChange(int32 NewFloor); 

	// UI 관련 함수
	// 게임시작 UI
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameStartWidgetClass;

	UPROPERTY()
	UUserWidget* GameStartWidget; // 메인메뉴

	// 게임 오버 UI
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY()
	UUserWidget* GameOverWidget;

	// 현재 위젯
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	UUserWidget* CurrentWidtet;

	// UI 표시해주는 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameStartWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameOverWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void RemoveCurrentWidget();

	// 게임 시스템 종료 등
	UFUNCTION(BlueprintCallable, Category = "UI")
	void QuitGame();

	// 플레이어 승리 여부 확인
	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayerWin;

private:
	FTimerHandle GameTimerHandle;
	bool bGameEnded = false;
	int32 CurrentPlayerFloor;

	// 게임 타이머 업데이트
	void UpdateGameTimer();

};



