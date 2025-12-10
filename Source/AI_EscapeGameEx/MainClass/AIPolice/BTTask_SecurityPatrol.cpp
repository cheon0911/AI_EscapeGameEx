#include "MainClass/AIPolice/BTTask_SecurityPatrol.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MainClass/AIPolice/Public/AISecurity.h"
#include "BlackboardKeys.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_SecurityPatrol::UBTTask_SecurityPatrol()
{
	NodeName = "Patrol";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_SecurityPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	AAISecurity* Security = Cast<AAISecurity>(AIController->GetPawn());
	if (Security)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return EBTNodeResult::Failed;
	}

	

	// Find Targetpoints
	TArray<AActor*> FoundTargets;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), Security->PatrolLocationTag, FoundTargets);

	// Targets 개수 확인
	if (FoundTargets.Num() < 2)
	{
		UE_LOG(LogTemp, Display, TEXT("Not Enough Target Points"));
	}

	AActor* Target = FoundTargets[0];
	AActor* Target2 = FoundTargets[1];

	UE_LOG(LogTemp, Display, TEXT("Found TargetPoints : %s and %s"),
		*Target->GetName(), *Target2->GetName());

	// 타겟 설정
	FVector TargetLocation = Security->bIsSucceeded ? Target->GetActorLocation() : Target2->GetActorLocation();

	// 블랙보드 업데이트
	if (BlackboardComp->GetKeyID(FBlackboardKeys::CurrentPatrolTarget) != FBlackboard::InvalidKey)
	{
		BlackboardComp->SetValueAsVector(FBlackboardKeys::CurrentPatrolTarget, TargetLocation);
	}

	// Move To 트리거 추후 다른 비헤이비어 트리 노드로 분리 가능 - Optional
	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
		TargetLocation,
		AcceptanceRadius,
		true,   // 목적지 오버랩을 도착으로 판정할지 여부
		true,  // pathfinding 활용 여부
		false, // projection 활용 여부
		true  // 네비개이션 데이터 사용
	);
	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_SecurityPatrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return; 
	}

	AAISecurity* Security = Cast<AAISecurity>(AIController->GetPawn());
	if (!Security)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return; 
	}

	// PathFollowingComp 선언

	UPathFollowingComponent* PathFollowingComp = AIController->GetPathFollowingComponent();

	EPathFollowingStatus::Type Status = PathFollowingComp->GetStatus();

	if (Status == EPathFollowingStatus::Idle)
	{
		Security->bIsSucceeded = !Security->bIsSucceeded;
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	else if (Status != EPathFollowingStatus::Moving && Status != EPathFollowingStatus::Waiting)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}