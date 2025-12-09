#include "MainClass/AIPolice/Public/AIPolice.h"

AAIPolice::AAIPolice()
{
	PrimaryActorTick.bCanEverTick = true;

	AIClass = TEXT("Police");
}

void AAIPolice::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 스태미나 관련 로직 구현
	// 휴식 (일반 상태) 일 때 스태미나 회복
	if (CurrentAlertLevel == EAlertLevel::Normal)
	{
		bIsResting = true;
		Stamina += (StaminaRecoveryRate * DeltaTime / (Age * 0.04)); // 나이가 적을수록 빨리 회복
		Stamina = FMath::Min(Stamina, MaxStamina); // Stamina 최대값 제한
	}

	// 추격중일때 스태미나 소모
	else if (CurrentAlertLevel == EAlertLevel::Pursuit && !bIsResting)
	{
		// Stamina 소진
		Stamina -= (StaminaDrainRate * DeltaTime * (Age * 0.04)); // 나이많을수록 스태미나 소모가 커지는 로직
		Stamina = FMath::Max(Stamina, 0.0f);  

		// 스태미나가 임계값 아래로 떨어지면 강제 휴식 기능
		if (Stamina < 10.f)
		{
			bIsResting = true;
			UE_LOG(LogTemp, Warning, TEXT("PoliceGuard is Exhausted! Can't Pursue Player Anymore"));
			SetAlertLevel(EAlertLevel::Alert); // 경계상태로 전환
		}
	}

	// 휴식 중이지만, 일반(Normal)상태는 아닌 경우, 스태미나 서서히 회복
	else if (bIsResting)
	{
		Stamina += (StaminaRecoveryRate * 0.5f * DeltaTime);
		Stamina = FMath::Min(Stamina, MaxStamina);

		// 충분히 회복 되었으면 다시 움직일 수 있도록 디자인
		if (Stamina >= 30.f) // Stamina가 30 이상으로 회복되면 다시 움직일 수 있도록 디자인
		{
			bIsResting = false;
		}
	}
}
