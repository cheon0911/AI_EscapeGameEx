#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SecurityPatrol.generated.h"


UCLASS()
class AI_ESCAPEGAMEEX_API UBTTask_SecurityPatrol : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_SecurityPatrol();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds);

protected:
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float AcceptanceRadius = 50.f;



};
