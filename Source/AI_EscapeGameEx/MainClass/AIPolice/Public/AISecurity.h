#pragma once

#include "CoreMinimal.h"
#include "MainClass/AIPolice/Public/AIBaseGuard.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "AISecurity.generated.h"


UCLASS()
class AI_ESCAPEGAMEEX_API AAISecurity : public AAIBaseGuard
{
	GENERATED_BODY()
	

public:
	AAISecurity();

	// 타겟 포인트 찾기
	UFUNCTION(BlueprintCallable, Category = "Player Capture")
	void FindTargetPoints();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Capture")
	AActor* Target;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Capture")
	AActor* Target2;

	// 임시 Move To Target 함수 구현, 추후 BB 및 BT로 이관
	UFUNCTION(BlueprintCallable, Category = "AI Movement")
	void MoveToTarget();

	UFUNCTION(BlueprintCallable, Category = "AI Movement")
	void StartMoving();

	// 도착 확인 후 반대편으로 Target 설정하기 위한 bool 변수 선언
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Movement")
	bool bIsSucceeded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Movement")
	float AcceptanveRadius = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Movement")
	TArray<AActor*> FoundTargets;

	// 패트롤 로캐이션의 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Movement")
	FName PatrolLocationTag;

	// 도착했을 때 실행 함수
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

protected:
	virtual void BeginPlay() override;

private:
	// AI 컨트롤러 내부 변수 저장하여 관리
	UPROPERTY()
	AAIController* AIController;

	// 현재 이동중인지 확인
	UPROPERTY()
	bool bIsMoving;

};
