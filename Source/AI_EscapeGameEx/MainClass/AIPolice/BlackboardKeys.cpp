#include "MainClass/AIPolice/BlackboardKeys.h"

// 플레이어 관련 키
const FName FBlackboardKeys::TargetPlayer = TEXT("TargetPlayer");
const FName FBlackboardKeys::LastKnwonLocation = TEXT("LastKnwonLocation");
const FName FBlackboardKeys::bIsPlayerDetected = TEXT("bIsPlayerDetected");

// 경계수준 및 상태 키
const FName FBlackboardKeys::AlertLevel = TEXT("AlertLevel");
const FName FBlackboardKeys::bIsMoving = TEXT("bIsMoving");

// 순찰 관련 키
const FName FBlackboardKeys::bCanPursue = TEXT("bCanPursue");
const FName FBlackboardKeys::bIsExhausted = TEXT("bIsExhausted");

// 추격 관련 키
const FName FBlackboardKeys::PatrolLocation = TEXT("PatrolLocation");
const FName FBlackboardKeys::CurrentPatrolTarget = TEXT("CurrentPatrolTarget");

// 조사 관련 키
const FName FBlackboardKeys::InvestigationLocation = TEXT("InvestigationLocation");