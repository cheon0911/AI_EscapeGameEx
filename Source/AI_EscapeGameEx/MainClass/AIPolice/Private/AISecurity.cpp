#include "MainClass/AIPolice/Public/AISecurity.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"

AAISecurity::AAISecurity()
{
	PatrolLocationTag = TEXT("PatrolPoint");
}

void AAISecurity::FindTargetPoints()
{
	// 타겟 포인트가 설정되어 있지 않은 경우 자동으로 찾기
	if (Target || Target2)
	{
		return;
	}

	// 에디터에서 설정해둔 Tag와 일치하는 TargetPoint 수집
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), PatrolLocationTag, FoundTargets);

	// Targets 가 2 이상이면 실행
	if (FoundTargets.Num() >= 2)
	{
		Target = FoundTargets[0];
		Target2 = FoundTargets[1];
		UE_LOG(LogTemp, Warning, TEXT("FoundTargetPoints : %s and %s"),
			*Target->GetName(), *Target2->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough Target Points"));
	}
}

void AAISecurity::BeginPlay()
{
	Super::BeginPlay();

	// AI Controller 확보
	AIController = Cast<AAIController>(GetController());

	// AI Controller 있으면 이동 완료 이벤트 바인딩
	if (AIController)
	{
		// 디버깅 에러 방지를 위해 언바인딩 코드 실행
		AIController->ReceiveMoveCompleted.RemoveDynamic(this, &AAISecurity::OnMoveCompleted);

		// AI Controller 내부 이벤트에 AISecurity 함수 바인딩. 옵저버 구현.
		AIController->ReceiveMoveCompleted.AddDynamic(this, &AAISecurity::OnMoveCompleted);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("There's No AI Controller"));
	}

	// 바로 순찰 시작
	StartMoving();
}

void AAISecurity::MoveToTarget()
{
	// AI Controller 가 없거나 움직이는 중이면 return
	if (!AIController || bIsMoving)
	{
		return;
	}
	// 삼항연산자 활용하여 Target 선택
	AActor* SelectedTarget = bIsSucceeded ? Target : Target2;

	if (SelectedTarget)
	{
		bIsMoving = true;

		// AI Move To 함수 호출. 추후 BB, BT로 이관하는 핵심 파트
		FVector TargetLocation = SelectedTarget->GetActorLocation();
		EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
			TargetLocation,
			AcceptanveRadius,
			true,   // 목적지 오버랩을 도착으로 판정할지 여부
			true,  // pathfinding 활용 여부
			false, // projection 활용 여부
			true  // 네비개이션 데이터 사용
		);

		if (MoveResult == EPathFollowingRequestResult::Failed)
		{
			bIsMoving = false;
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Moving to %s (IsSucceeded: %s)"),
				*SelectedTarget->GetName(), bIsSucceeded ? TEXT("True") : TEXT("False"));
		}

	}
}

void AAISecurity::StartMoving()
{
	FindTargetPoints();
	MoveToTarget();
}

void AAISecurity::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	bIsMoving = false;
	
	// 이동 결과에 따라 IsSucceeded값 토글
	if (Result == EPathFollowingResult::Success)
	{
		// 이동 성공
		bIsSucceeded = !bIsSucceeded;

		// 약간의 지연 부여한 후 다음 이동 시작
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AAISecurity::MoveToTarget, 0.5f, false);
	}
	else
	{
		// 이동 실패
		UE_LOG(LogTemp, Warning, TEXT("Move Failed with result: %d"), static_cast<int32>(Result));
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AAISecurity::MoveToTarget, 1.0f, false);
	}
}