#pragma once

#include "CoreMinimal.h"
#include "MainClass/AIPolice/Public/AIBaseGuard.h"
#include "AIPolice.generated.h"

UCLASS()
class AI_ESCAPEGAMEEX_API AAIPolice : public AAIBaseGuard
{
	GENERATED_BODY()
	
public:
	AAIPolice();

	// AI Police 체력 프로퍼티 부여
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Police Feature")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Police Feature")
	float StaminaRecoveryRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Police Feature")
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Police Feature")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Police Feature")
	int32 Age;

	virtual void UpdateBlackboard();



private:
	bool bIsResting;

protected:
	virtual void Tick(float DeltaTime) override;


};
