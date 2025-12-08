// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NavigationInvokerComponent.h"
#include "AIController.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "Blueprint/UserWidget.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "AI_EscapeGameExCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

// 스킬 활성화 상태를 추적하기 위한 구조체
USTRUCT(BlueprintType)
struct FSkillState
{
	GENERATED_BODY()

	// 스킬이 활성화 되어 있는 상태인가?>
	bool bIsActive = false;

	// 스킬의 활성화 시간
	float Duration = 0.f;

	// 스킬 종료까지 남은 시간
	float RemainingTime = 0.f;

	// 스킬에 필요한 마나(비용)
	float ManaCost = 0.f;
};


/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AAI_EscapeGameExCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

public:

	/** Constructor */
	AAI_EscapeGameExCharacter();	

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();


/// ////////////// 내가 추가한 스킬
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void TeleportPlayer();

	UPROPERTY(EditAnywhere, Category = "Skill|Teleport")
	UNiagaraSystem* TeleportStartEffect;

	UPROPERTY(EditAnywhere, Category = "Skill|Teleport")
	UNiagaraSystem* TeleportEndEffect;

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void KillSkill();

	UPROPERTY(EditAnywhere, Category = "Skill|Teleport")
	UAnimMontage* TeleportMontage;

	UPROPERTY(EditAnywhere, Category = "Skill|Teleport")
	USoundBase* TeleportSound;


	// Kill Skill
	UPROPERTY(EditAnywhere, Category = "Skill|Kill")
	UAnimMontage* KillMontage;

	UPROPERTY(EditAnywhere, Category = "Skill|Kill")
	USoundBase* KillSound;


public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// AI Bank Game////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// // AI Bank Game////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 스텔스모드 플래그 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	bool bIsInStealthMode;

	// 소음발생 가능 여부 플래그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	bool bCanMakeNoise;

	// 설득 성공여부 플래그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	bool bIsSucceedPersuasion;

	// 네비게이션 메시 반경 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Navigation)
	float NaviGenerationRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Navigation)
	float NavRemovalRadius;

	// 플레이어 생명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int32 Lives;

	// 게임오버 UI
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY()
	UUserWidget* GameOverWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noise")
	class UPawnNoiseEmitterComponent* NoiseEmitterComponent;

protected:
	// 살금살금 걷기 스킬
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SilentMovementAction;

	// 은신 스킬
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* StealthAction;

	// 설득 스킬
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PersuadeAction;

	// 달리기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////위까지 예제 스킬///////////////////////////////////////////
	// 암살 스킬 내가 추가한거
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* KillAction;

	// 텔레포트 내가 추가한거
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TeleportAction;
	
	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

private:
	UPROPERTY()
	bool bIsMoving;

	// 이동중에는 항상 MakeNoise 작동
	void MakeNoise(float Loudness, FVector NoiseLocation);

	// 게임시스템의 over가 아니라 playercharacter가 GameOver시 적용되어야 하는것들 구현하는 함수로 쓰일 예정
	void GameOver();

public:
	// 마나 시스템
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats");
	float MaxMana = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats");
	float CurrentMana = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats");
	float ManaRegenRate = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats");
	float ManaRegenInterval = 1.f;

	// 스킬 상태 관리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Skill");
	FSkillState SilentMovementSkill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Skill");
	FSkillState StealthSkill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Skill");
	FSkillState InvulnerabilirtySkill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Skill")
	float DefaultWalkSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Skill")
	bool bIsSprinting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Skill")
	float SlowWalkSpeed = 250.f;

	// 스킬 사용 함수  
	UFUNCTION(BlueprintCallable, Category = "Player Skill")
	void ActivateSilentMovement();

	UFUNCTION(BlueprintCallable, Category = "Player Skill")
	void ActivateStealth();

	UFUNCTION(BlueprintCallable, Category = "Player Skill")
	void TryPersuade();

	UFUNCTION(BlueprintCallable, Category = "Player Skill")
	void StartSprint();

	// Player <-> AI 간 인터랙션 관련////////////////////////////////
	// 잡혔나?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Interaction")
	bool bIsCaptured = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Interaction")
	bool bIsInvulnerable = false; // 무적 상태 확인

	// 타이머 세팅
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Interaction")
	bool bIsTimerSet = false;

	// 무적 지속시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Interaction")
	float InvulnerabilityDuration = 10.f;

	// 설득 성공 확률. 50% 부여
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Interaction")
	float PersuasionChance = 0.5f;

	// 설득 가능 범위 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Interaction")
	float PersuasionWindow = 5.f;

	// 잡힌 시간
	UPROPERTY(EditAnywhere, Category = "AI Interaction")
	float CaptureTime = 0.f;

	// AI에 의해 잡혔을 때 호출되는 함수
	UFUNCTION(BlueprintCallable, Category = "AI INteraction")
	void PlayerCaptured(); // 추후 작성될 인터페이스 함수명과 겹치면 오류 발생!

private:
	// 마나 재생 타이머
	FTimerHandle ManaRegenTimerHandle;

	// 스킬 지속시간 타이머
	FTimerHandle SilentMovementTimerHandle;
	FTimerHandle StealthTimerHandle;
	FTimerHandle InvulnerabilityTimerHandle;

	// 잡힐경우 10초 무적 타이머
	FTimerHandle CapturedTimerHandle;

	// 마나 리젠 함수
	void RegenerationMana();

	// 스킬 종료 함수
	void EndSilentMovement();
	void EndStealth();
	void EndInvulnerability();
	void StopSprint();

	// 캐릭터 이동시 소음의 크기
	UPROPERTY(EditAnywhere, Category = "Noise", meta = (AllowPrivateAccess = "true"))
	float DefaultMovementNoiseLoudness = 1.f; 

	// 기존 메시 머티리얼 저장
	TArray<UMaterialInterface*> OriginalMaterials;

	// 스텔스용 머티리얼
	UPROPERTY(EditAnywhere, Category = "Player Skill", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* StealthMaterial;


};


