#pragma once

#include "CoreMinimal.h"
#include "BlackboardKeys.generated.h"

USTRUCT()
struct AI_ESCAPEGAMEEX_API FBlackboardKeys
{
	GENERATED_BODY()

	// 플레이어 관련 키
	static const FName TargetPlayer;
	static const FName LastKnwonLocation;
	static const FName bIsPlayerDetected;

	// 경계수준 및 상태
	static const FName AlertLevel;
	static const FName bIsMoving;

	// 추적 관련 키 폴리스용
	static const FName bCanPursue;
	static const FName bIsExhausted;
	
	// 순찰 관련 키
	static const FName PatrolLocation;
	static const FName CurrentPatrolTarget;
		
	// 조사 관련 키
	static const FName InvestigationLocation;


};