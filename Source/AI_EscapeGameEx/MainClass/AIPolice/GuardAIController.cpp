#include "MainClass/AIPolice/GuardAIController.h"

AGuardAIController::AGuardAIController()
{
	// 추후 기능 추가
}

void AGuardAIController::BeginPlay()
{
	Super::BeginPlay();

	// Behavior Tree 실행
	if (BehaviorTree)
	{
		UBlackboardComponent* BlackboardComp = Blackboard;
		if (UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp))
		{
			RunBehaviorTree(BehaviorTree);
		}
	}
}