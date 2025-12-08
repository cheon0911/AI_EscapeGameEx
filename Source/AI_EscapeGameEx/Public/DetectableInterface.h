#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DetectableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UDetectableInterface : public UInterface
{
	GENERATED_BODY()
};

// 감지 가능한지 확인하는 객체 인터페이스
class AI_ESCAPEGAMEEX_API IDetectableInterface 
{
	GENERATED_BODY()


public:
	// 감지 가능 여부 확인 (스텔스 모드인지?)
	// 엔진에서는 아래와 같이 BlueprintNativeEvent의 경우
	// 실제 구현은 이 인터페이스를 받아서 사용하는 클래스 내부에 작성되어 있음.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
	bool CanBeDetected() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
	bool CanMakeNoise() const;

	// 플레이어가 AI에게 잡혔을 때 호출함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
	void CapturedByAI();


};
// 게임모드(게임 룰)용 관리 인터페이스

// AI Police용 인터페이스
UINTERFACE(MinimalAPI)
class UGuardInterface : public UInterface
{
	GENERATED_BODY()
};

// 감지 가능한지 확인하는 객체 인터페이스
class AI_ESCAPEGAMEEX_API IGuardInterface
{
	GENERATED_BODY()


public:
	// 최근 플레이어(도둑) 감지 위치 확인
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guard")
	FVector GetLastKnownPlayerLocation() const;

	// 경계레벨 확인
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guard")
	uint8 GetAlertLevel() const;

	// 누구를 쫒아야 할지 타겟 플레이어 확인
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guard")
	APawn* GetTargetPlayer() const;

	// 다른 AI에게 알림을 받는 기능
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Guard")
	void ReceiveAlert(const FVector& Location, uint8 AlertLevel, AActor* AlertSource);
};