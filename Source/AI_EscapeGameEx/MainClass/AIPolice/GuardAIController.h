#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GuardAIController.generated.h"


UCLASS()
class AI_ESCAPEGAMEEX_API AGuardAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AGuardAIController();

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTree;

	UBlackboardComponent* GetBlackboard() const;

	virtual void BeginPlay() override;

};
