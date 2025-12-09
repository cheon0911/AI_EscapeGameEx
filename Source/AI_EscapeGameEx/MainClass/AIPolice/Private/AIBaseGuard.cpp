#include "AIBaseGuard.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

AAIBaseGuard::AAIBaseGuard()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	
	// PawnSensing 사용 version:: 감지 컴포넌트 설정
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
	PawnSensingComponent->SetPeripheralVisionAngle(ViewAngle);
	PawnSensingComponent->SightRadius = SightRange;
	PawnSensingComponent->HearingThreshold = 1500.f;
	PawnSensingComponent->LOSHearingThreshold = 3000.f;
	PawnSensingComponent->SensingInterval = 1.f;
	PawnSensingComponent->bOnlySensePlayers = false;
	
	// Perception 초기화
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	// AI 감지범위 시각화 -> 주석 처리로 없애기 가능
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(DetectionRange);
	DetectionSphere->SetCollisionProfileName(TEXT("Trigger"));

	// 기본 속성 초기화
	CurrentAlertLevel = EAlertLevel::Normal;
	TimeInCurrentAlertLevel = 0.f;
	LastKnownPlayerLocation = FVector::ZeroVector;
	TargetPlayer = nullptr;

	AIClass = TEXT("Class");

}

void AAIBaseGuard::BeginPlay()
{
	Super::BeginPlay();
	
	// AI 컨트롤러 확보
	GuardController = Cast<AAIController>(GetController());

	// PawnSensing 이벤트 바인딩
	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AAIBaseGuard::OnSeePlayer);
		PawnSensingComponent->OnHearNoise.AddDynamic(this, &AAIBaseGuard::OnHearNoise);
	}

	// AI 인지 이벤트 바인딩
	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAIBaseGuard::OnTargetPerceptionUpdated);
	}

	// 테스트 타이머 설정
	// 1초에 1번식 호출시켜서 게임경과시간 기록하는 용도.
	GetWorldTimerManager().SetTimer(TestTimerHandle, this, &AAIBaseGuard::TimerTest, 1.f, true);

}

void AAIBaseGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 경계 수준의 변화 등에 따른 행동 업데이트
	switch (CurrentAlertLevel)
	{
	case EAlertLevel::Normal:
		Patrol();
		break;

	case EAlertLevel::Suspicious:
		InvestigateLastKnownLocation();
		break;


	case EAlertLevel::Alert:
		InvestigateLastKnownLocation();
		break;

	case EAlertLevel::Pursuit:
		PursuePlayer();

		// 플레이어가 잡혔는지 확인
		// TargetPlayer가 nullptr이 아니면서 검거 가능한 범위 내에 있는 경우 True
		if (TargetPlayer && FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation()) <= CaptureRange)
		{
			CapturePlayer();// 조건 충족시 플레이어 체포
		}
		break;
	}

	// 상태누적시간 업데이트
	TimeInCurrentAlertLevel += DeltaTime;

}

// 플레이어 잡혔을 때
void AAIBaseGuard::OnPlayerdetected(APawn* DetectedPawn)
{
	UE_LOG(LogTemp, Error, TEXT("Player Detected"));
	if (!DetectedPawn)
	{
		return;
	}

	// DetectableInterface가 구현되어 있는지 확인
	if (DetectedPawn->GetClass()->ImplementsInterface(UDetectableInterface::StaticClass()))
	{
		// 메서드 Execute
		if (IDetectableInterface::Execute_CanBeDetected(DetectedPawn))
		{
			LastKnownPlayerLocation = DetectedPawn->GetActorLocation();
			TargetPlayer = DetectedPawn;

			// 경계수준 업데이트
			SetAlertLevel(EAlertLevel::Pursuit);

			// 다른 가드에게 알림
			AlertOtherGuards(LastKnownPlayerLocation);
		}

		else if(!IDetectableInterface::Execute_CanBeDetected(DetectedPawn))
		{
			UE_LOG(LogTemp, Error, TEXT("But the Player Can't be Detected"));
		}
	}
}

void AAIBaseGuard::OnHearNoise(APawn* NoiseMaker, const FVector& Location, float Volume)
{
	UE_LOG(LogTemp, Warning, TEXT("#1. AI Heard Something!"));
	if (!NoiseMaker)
	{
		return;
	}

	if (IDetectableInterface::Execute_CanMakeNoise(NoiseMaker))
	{
		// 노이즈 확인 위치로 LastknownLocation 초기화
		LastKnownPlayerLocation = Location;

		// 볼륨 크기에 따라 경계 수준 설정
		if (Volume > 0.7f)
		{
			SetAlertLevel(EAlertLevel::Alert);
		}

		else if (Volume > 0.5f)
		{
			SetAlertLevel(EAlertLevel::Suspicious);
			AlertOtherGuards(Location); // 다른 가드에게 알림
		}

	}

}

void AAIBaseGuard::OnSeePlayer(APawn* SeenPawn)
{
	OnPlayerdetected(SeenPawn);
}

void AAIBaseGuard::SetAlertLevel(EAlertLevel NewAlertLevel)
{
	// 이미 해당 경계수준이거나 더 높으면 return
	if (static_cast<uint8>(CurrentAlertLevel) >= static_cast<uint8>(NewAlertLevel))
	{
		return;
	}

	CurrentAlertLevel = NewAlertLevel;  // 현재 상태보다 새로운 경계상태가 더 강한(인덱스가 높은) 경계상태일 경우 새로운 경계상태로 업데이트
	TimeInCurrentAlertLevel = 0.f; // 경계상태 변경 후 누적 시간 초기화

	// 경계 타이머 설정
	ResetAlertTimer();

	// 경계 수준에 따른 속력 조정
	if (CurrentAlertLevel == EAlertLevel::Pursuit)
	{
		GetCharacterMovement()->MaxWalkSpeed = PursuitSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
	}
}

void AAIBaseGuard::PursuePlayer()
{
	if (!GuardController || !TargetPlayer)
	{
		return;;
	}
	// police 인지 확인 . 추격은 police만 가능
	if (AIClass != "Police")
	{
		UE_LOG(LogTemp, Warning, TEXT("AIClass is not Police"));
		return;
	}
	//플레이어 추격. 추후 BT, BB 제작 후 수정
	//GuardController->GetBlackboardComponent();
	GuardController->MoveToActor(TargetPlayer, -1.f, true, true, true); // 임시코드

	// 주기적으로 다른 가드에게 최신 정보와 상태 업데이트
	if (FMath::RandBool())// 50% 확률로 true or false 반환하는 함수
	{
		AlertOtherGuards(TargetPlayer->GetActorLocation());
	}
}

void AAIBaseGuard::InvestigateLastKnownLocation()
{
	if (!GuardController || LastKnownPlayerLocation.IsZero())
	{
		return;
	}

	// 마지막으로 알려진 위치로 이동
	GuardController->MoveToLocation(LastKnownPlayerLocation, -1.f, true, true, true);

	// 목적지 도착 확인
	float DistanceToTarget = FVector::Dist(GetActorLocation(), LastKnownPlayerLocation);

	if (DistanceToTarget < 100.f) // 100 보다 작으면 도착으로 판단
	{
		// 주변의 랜덤위치 생성하여 추가 탐색
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSystem)
		{
			FNavLocation RandomLocation;
			if (NavSystem->GetRandomPointInNavigableRadius(LastKnownPlayerLocation, 300.f, RandomLocation)) // LastKnownLocation의 300범주 내를 랜덤으로 지정
			{
				LastKnownPlayerLocation = RandomLocation.Location; // Random 위치로 LastKnownLocation 업데이투
			}
		}
	}
}

void AAIBaseGuard::Patrol()
{
	// Base에서 할 필요 없음 , 추후 자손에서 개별 구현
}

void AAIBaseGuard::CapturePlayer()
{
	// TargetPlayer가 nullptr이면 return
	if (!TargetPlayer)
	{
		return;
	}

	// 인터페이스 사용하여 capture
	if (TargetPlayer->GetClass()->ImplementsInterface(UDetectableInterface::StaticClass()))
	{
		// 플레이어 캡쳐 처리
		IDetectableInterface::Execute_CapturedByAI(TargetPlayer);

		// 경계수준 초기화
		SetAlertLevel(EAlertLevel::Normal);

		// 일정시간 대기 후 순찰 재개 ,  추후 블랙보드 키로 대체
		LastKnownPlayerLocation = FVector::ZeroVector;
		TargetPlayer = nullptr;

	}
}

void AAIBaseGuard::AlertOtherGuards(const FVector& LocationToInvestigate)
{
	// 다른 AI찾기
	TArray<AActor*> Guards;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAIBaseGuard::StaticClass(), Guards);

	// 찾은 가드에게 정보 전달
	for (AActor* Guard : Guards)
	{
		if (Guard != this)
		{
			// 통신범위 내에 있는지 확인
			float Distance = FVector::Dist(GetActorLocation(), Guard->GetActorLocation());
			if (Distance <= CommunicationRange)
			{
				// 인터페이스 구현 확인
				if (Guard->GetClass()->ImplementsInterface(UGuardInterface::StaticClass()))
				{
					// 인터페이스 발신  호출이므로 반드시 execute_ 접두사 사용. 수신한 AI 클래스 내부에서 작동하는 함수
					IGuardInterface::Execute_ReceiveAlert(Guard, LocationToInvestigate, static_cast<uint8>(CurrentAlertLevel), this);
				}
			}
		}
	}

}

void AAIBaseGuard::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 자극 종류에 따른 처리
	if (Stimulus.WasSuccessfullySensed())
	{
		APawn* DetectedPawn = Cast<APawn>(Actor);
		if (DetectedPawn)
		{
			// 시각 자극
			if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
			{
				OnSeePlayer(DetectedPawn);
			}
			// 청각 자극
			if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
			{
				OnHearNoise(DetectedPawn, Stimulus.StimulusLocation, Stimulus.Strength);
			}
		}
	}
}

void AAIBaseGuard::ResetAlertTimer()
{
	// 기존 타이머 취소
	GetWorldTimerManager().ClearTimer(AlertTimerHandle);

	// Normal일 경우 타이머 불필요하므로 설정 해제
	if(CurrentAlertLevel == EAlertLevel::Normal)
	{
		return;
	}

	// 새 타이머 설정
	GetWorldTimerManager().SetTimer(AlertTimerHandle, this, &AAIBaseGuard::HandleAlertTimeout, AlertTimeout, false);

}

void AAIBaseGuard::HandleAlertTimeout()
{
	// HandleAlertTimeout (10초) 경과 후 경계 수준 다운그레이드
	switch (CurrentAlertLevel)
	{
	case EAlertLevel::Pursuit:
		SetAlertLevel(EAlertLevel::Alert);
		break;

	case EAlertLevel::Alert:
		SetAlertLevel(EAlertLevel::Suspicious);
		break;

	case EAlertLevel::Suspicious:
		SetAlertLevel(EAlertLevel::Normal);
		break;

	default:
		break;
	}
}

void AAIBaseGuard::TimerTest()
{

}


void AAIBaseGuard::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// 인터페이스 구현 메소드들
FVector AAIBaseGuard::GetLastKnownPlayerLocation_Implementation() const
{
	return LastKnownPlayerLocation;
}

uint8 AAIBaseGuard::GetAlertLevel_Implementation() const
{
	return static_cast<uint8>(CurrentAlertLevel);
}

APawn* AAIBaseGuard::GetTargetPlayer_Implementation() const
{
	return TargetPlayer;
}

void AAIBaseGuard::ReceiveAlert_Implementation(const FVector& Location, uint8 AlertLevel, AActor* AlertSource)
{
	// 외부의 다른 AI에게서 본 Ai에게 들어온 호출/정보임에 유의
	// 정보 수신처리
	LastKnownPlayerLocation = Location;

	// 경계레벨 업데이트
	if (AlertLevel > static_cast<uint8>(CurrentAlertLevel))
	{
		SetAlertLevel(static_cast<EAlertLevel>(AlertLevel));
	}
}