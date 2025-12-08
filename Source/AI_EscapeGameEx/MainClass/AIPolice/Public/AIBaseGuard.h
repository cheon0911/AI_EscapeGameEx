#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/PawnSensingComponent.h"
#include "DetectableInterface.h"
#include "AIBaseGuard.generated.h"

// 전방 선언
class UAIPerceptionComponent;
class UPawnSensingComponent;
class USphereComponent;
class AAIController;

// 경계 상태 열거형(enum) 정의
UENUM(BlueprintType)
enum class EAlertLevel : uint8
{
	// 일반, 의심, 경계, 추격 4단계로 설정
	
	Normal UMETA(DisplayName = "Normal"),
	Suspicious UMETA(DisplayName = "Suspicious"),
	Alert UMETA(DisplayName = "Alert"),
	Pursuit UMETA(DisplayName = "Pursuit")

};

UCLASS()
class AI_ESCAPEGAMEEX_API AAIBaseGuard : public ACharacter, public IGuardInterface
{
	GENERATED_BODY()

public:
	
	AAIBaseGuard();

public:	
	
	virtual void Tick(float DeltaTime) override;

	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// AI Guard에게 필요한 컴포넌트들 
	// AI 시각
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UAIPerceptionComponent* AIPerceptionComponent;

	// AI 청각
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UPawnSensingComponent* PawnSensingComponent;

	// AI Properties (속성)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Properties")
	float MovementSpeed = 400.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Properties")
	float DetectionRange = 1000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Properties")
	float ViewAngle = 60.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Properties")
	float HearingRange = 800.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Properties")
	float SightRange = 1500.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Properties")
	float PursuitSpeed = 600.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Properties")
	float CaptureRange = 100.f; // 1m 수준
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Properties")
	EAlertLevel CurrentAlertLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Properties")
	float AlertTimeout = 10.f;

	// AI 계급 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Properties")
	FName AIClass;

	// 마지막 발견 위치
	UPROPERTY(BlueprintReadOnly, Category = "AI|Tracking")
	FVector LastKnownPlayerLocation;

	// 경계시간 관리
	UPROPERTY(BlueprintReadOnly, Category = "AI|Tracking")
	float TimeInCurrentAlertLevel;

	// 타깃 플레이어
	UPROPERTY(BlueprintReadOnly, Category = "AI|Tracking")
	APawn* TargetPlayer;

	// 통신범위. AI끼리 무전 가능범위
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Communication")
	float CommunicationRange = 3000.f; // 30m -> 1unreal unit = 1cm

	// Guard 인터페이스 메소드 구현
	virtual FVector GetLastKnownPlayerLocation_Implementation() const override; // 인터페이스에 있는걸 받아 쓸 때 뒤에 Implementation 붙여서 구현
	virtual uint8 GetAlertLevel_Implementation() const override;
	virtual APawn* GetTargetPlayer_Implementation() const override;
	virtual void ReceiveAlert_Implementation(const FVector& Location, uint8 AlertLevel, AActor* AlertSource) override;

protected:

	virtual void BeginPlay() override;

	// AI Controller 참조
	UPROPERTY()
	AAIController* GuardController;

	// 타이머 핸들
	FTimerHandle AlertTimerHandle;
	FTimerHandle TestTimerHandle;

public:
	// AI Guard 함수 모음 /////////////////////////////////////////////////////////////////////////////////////////
	// 플레이어 감지 함수
	UFUNCTION(BlueprintCallable, Category = "AI|Detection")
	virtual void OnPlayerdetected(APawn* DetectedPawn);

	// 소리 감지 함수
	UFUNCTION(BlueprintCallable, Category = "AI|Detection")
	virtual void OnHearNoise(APawn* NoiseMaker, const FVector& Location, float Volume);

	// 시각 감지 함수
	UFUNCTION(BlueprintCallable, Category = "AI|Detection")
	virtual void OnSeePlayer(APawn* SeenPawn);

	// 경계수준 변경 함수
	UFUNCTION(BlueprintCallable, Category = "AI|Behavior")
	virtual void SetAlertLevel(EAlertLevel NewAlertLevel);

	// 플레이어 추적 함수
	UFUNCTION(BlueprintCallable, Category = "AI|Behavior")
	virtual void PursuePlayer();

	// 마지막으로 알려진 위치 조사하는 함수
	UFUNCTION(BlueprintCallable, Category = "AI|Behavior")
	virtual void InvestigateLastKnownLocation();

	// 순찰함수 (optional)
	UFUNCTION(BlueprintCallable, Category = "AI|Behavior")
	virtual void Patrol();

	// 플레이어 검거(Capture) 함수 
	UFUNCTION(BlueprintCallable, Category = "AI|Behavior")
	virtual void CapturePlayer();

	// AI간 통신 (무전)통한 정보 전달 함수
	UFUNCTION(BlueprintCallable, Category = "AI|Communication")
	virtual void AlertOtherGuards(const FVector& LocationToInvestigate);

	// 인지 이벤트 핸들러. 시청각 사용할 예정이기에 필요.
	UFUNCTION(BlueprintCallable, Category = "AI|Communication")
	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	// 타이머 관리 함수들
	void ResetAlertTimer();
	void HandleAlertTimeout();
	void TimerTest();

};
