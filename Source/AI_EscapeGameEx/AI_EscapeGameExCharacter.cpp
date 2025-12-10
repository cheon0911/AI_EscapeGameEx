// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI_EscapeGameExCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AIBaseGuard.h"
#include "Kismet/GameplayStatics.h"
#include "AI_EscapeGameEx.h"

AAI_EscapeGameExCharacter::AAI_EscapeGameExCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Lives 3 추가
	Lives = 3;
	
	// AI 게임 관련 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 스킬 초기화
	// 살금살금 걷기 스킬 
	SilentMovementSkill.Duration = 10.f;
	SilentMovementSkill.ManaCost = 10.f;

	// 스텔스 스킬
	StealthSkill.Duration = 5.f;
	StealthSkill.ManaCost = 10.f;

	// 무적 상태
	InvulnerabilirtySkill.Duration = 10.f;
	InvulnerabilirtySkill.ManaCost = 0.f;

	/////// 내가 만든 거 
	TeleportSkill.ManaCost = 50.f;
	TeleportSkill.Duration = 3.f;

	KillSkillStat.ManaCost = 30.f;
	KillSkillStat.Duration = 3.f;
}

void AAI_EscapeGameExCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAI_EscapeGameExCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AAI_EscapeGameExCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAI_EscapeGameExCharacter::Look);

		// 발소리 제거 스킬 트리거
		if (SilentMovementAction)
		{
			EnhancedInputComponent->BindAction(SilentMovementAction, ETriggerEvent::Started, this, &AAI_EscapeGameExCharacter::ActivateSilentMovement);
		}
		
		// 스텔스 스킬 트리거
		if (StealthAction)
		{
			EnhancedInputComponent->BindAction(StealthAction, ETriggerEvent::Started, this, &AAI_EscapeGameExCharacter::ActivateStealth);
		}
		
		// 설득 스킬 트리거
		if (PersuadeAction)
		{
			EnhancedInputComponent->BindAction(PersuadeAction, ETriggerEvent::Started, this, &AAI_EscapeGameExCharacter::TryPersuade);
		}
		
		// 달리기 스킬 트리거
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AAI_EscapeGameExCharacter::StartSprint);
			// 손떼면 달리기에서 걷기 전환
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AAI_EscapeGameExCharacter::StopSprint);
		}

		// teleport
		EnhancedInputComponent->BindAction(TeleportAction, ETriggerEvent::Started, this, &AAI_EscapeGameExCharacter::TeleportPlayer);

		// kill
		EnhancedInputComponent->BindAction(KillAction, ETriggerEvent::Started, this, &AAI_EscapeGameExCharacter::KillSkill);
	}
	else
	{
		UE_LOG(LogAI_EscapeGameEx, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AAI_EscapeGameExCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AAI_EscapeGameExCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AAI_EscapeGameExCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AAI_EscapeGameExCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AAI_EscapeGameExCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AAI_EscapeGameExCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AAI_EscapeGameExCharacter::TeleportPlayer()
{
	if (CurrentMana < TeleportSkill.ManaCost)
	{
		return;
	}

	// 마나 소모 후 스킬 활성화
	// 마나소모
	CurrentMana -= TeleportSkill.ManaCost;

	//스킬 활성화
	TeleportSkill.bIsActive = true;
	// 1) 애니메이션 재생
	if (TeleportMontage)
	{
		PlayAnimMontage(TeleportMontage);
	}

	// 2) 사운드 재생
	if (TeleportSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, TeleportSound, GetActorLocation());
	}
	FRotator Rot = GetActorRotation();
	Rot.Yaw -= 90.0f;
	if (TeleportStartEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			TeleportStartEffect,
			GetActorLocation(),
			Rot
		);
	}

	// 3) 이동 처리 (Forward로 400만큼)
	FVector Forward = GetActorForwardVector();
	FVector TargetLocation = GetActorLocation() + (Forward * 1000.f);

	

	if (TeleportEndEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			TeleportEndEffect,
			TargetLocation,
			Rot
			
		);
	}

	// 순간이동
	SetActorLocation(TargetLocation, true);
}

void AAI_EscapeGameExCharacter::KillSkill()
{
	UE_LOG(LogTemp, Warning, TEXT("Trace Start"));
	// 1) 라인트레이스로 앞에 캐릭터가 있는지 체크
	FVector Start = GetActorLocation() + FVector(0, 0, 50.f);
	FVector End = Start + (GetActorForwardVector() * 200.f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	// 디버그 라인
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.0f, 0, 2.0f);

	// 2) 맞은 액터가 캐릭터인지 판단
	if (bHit && Hit.GetActor()->IsA<ACharacter>())
	{
		ACharacter* TargetCharacter = Cast<ACharacter>(Hit.GetActor());
		LineHitActor = TargetCharacter;
		UE_LOG(LogTemp, Warning, TEXT("Hit"));
		if (AAIBaseGuard* TargetAlert = Cast<AAIBaseGuard>(Hit.GetActor()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Trace Success"));
			if (TargetAlert->CurrentAlertLevel == EAlertLevel::Normal)
			{
				UE_LOG(LogTemp, Warning, TEXT("Kill"));
				// 3) 애니메이션 재생
				if (KillMontage)
				{
					PlayAnimMontage(KillMontage);
				}

				// 4) 사운드 재생
				if (KillSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, KillSound, GetActorLocation());
				}

				// 5) 상대방에게 데미지 / 죽이기
				//TargetCharacter->Destroy();
			}
		}
		
		
		
		
	}
}

void AAI_EscapeGameExCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 마나 재생 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(ManaRegenTimerHandle, this, &AAI_EscapeGameExCharacter::RegenerationMana, ManaRegenInterval, true);

	// 은신시 머티리얼 변경 예정이므로 기존 머티리얼 캡쳐(배열에 저장)
	OriginalMaterials.Empty();
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
		{
			UMaterialInterface* Material = MeshComp->GetMaterial(i);
			OriginalMaterials.Add(Material);

			// 로그 추가
			UE_LOG(LogTemp, Warning, TEXT("Saved Original Material %d: %s"),
				i, Material ? *Material->GetName() : TEXT("null"));
		}

		// 총 머티리얼 개수 로그
		UE_LOG(LogTemp, Warning, TEXT("Total Original Materials Saved: %d"), OriginalMaterials.Num());
	}
	

}

void AAI_EscapeGameExCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 살금살금 스킬 타이머 업데이트
	if (SilentMovementSkill.bIsActive)
	{
		SilentMovementSkill.RemainingTime -= DeltaTime;
		if (SilentMovementSkill.RemainingTime <= 0.f)
		{
			EndSilentMovement();
		}
	}

	// 스텔스 스킬 타이머 업데이트
	if (StealthSkill.bIsActive)
	{
		StealthSkill.RemainingTime -= DeltaTime;
		if (StealthSkill.RemainingTime <= 0.f)
		{
			EndStealth();
		}
	}

	// 무적 타이머 업데이트
	if (InvulnerabilirtySkill.bIsActive)
	{
		InvulnerabilirtySkill.RemainingTime -= DeltaTime;
		if (InvulnerabilirtySkill.RemainingTime <= 0.f)
		{
			EndInvulnerability();
		}
	}

	// 걷거나 뛸 때 소음 발생 (소리제거 스킬이 활성화 상태인지 확인 후 false면 실행 하도록 작성)
	if (!SilentMovementSkill.bIsActive && GetVelocity().Size() > 10.f && GetCharacterMovement()->IsMovingOnGround())
	{
		MakeNoise(DefaultMovementNoiseLoudness, GetActorLocation());
	}


}

void AAI_EscapeGameExCharacter::MakeNoise(float Loudness, FVector NoiseLocation)
{
	// 소리제거 스킬 활성화 시 소음 생성 X
	if (SilentMovementSkill.bIsActive) 
	{
		return;
	}

	// NoiseEmitterComponent 활용해 소음 발생
	if (NoiseEmitterComponent)
	{
		NoiseEmitterComponent->MakeNoise(this, Loudness, NoiseLocation);
		UE_LOG(LogTemp, Warning, TEXT("Noise Created at %s with Loudness %f"),
			*NoiseLocation.ToString(), Loudness);
	}
	
}

void AAI_EscapeGameExCharacter::TryPersuade()
{
	// 잡히지 않은 상태면 설득 불가
	if (!bIsCaptured) //|| !CapturingGuard) // 추후 AI 경탈 구현 후 설득 직전에 상대중인 클래스가 AI경찰이 맞는지 확인하도록 수정
	{
		return;
	}
	// 설득 가능시간이 지났으며 설득 불가
	float CurrentTime = GetWorld()->GetTimeSeconds();
	// 추후 Player가 Ai에게 잡힌 시점에 Captured Time을 저장할 예정. 저장한 Captured TIme과 설득시도한 시점의 Time을 뺀 값이, 
	// 사전 정의한 PersuasionWindow보다 큰지 작은지 확인
	// 크면 너무 늦은것, 작으면 아직 시간내에 있기에 설득 시도 가능
	if (CurrentTime - CaptureTime > PersuasionWindow)
	{
		UE_LOG(LogTemp, Warning, TEXT("Persuasion Failed - Too Late !!"));
		return;
	}
	
	// 설득 성공 확률에 따라 성공여부 결정
	bool bPersuasionSuccess = FMath::FRand() < PersuasionChance;
	if (bPersuasionSuccess && CurrentMana > 20)
	{
		// 설득 성공 플래그
		bIsSucceedPersuasion = true;
		// 설득 성공 -> 무적 상태 부여
		// 무적상태 관련 struct 요소 지정
		bIsCaptured = false;
		bIsInvulnerable = true;
		InvulnerabilirtySkill.bIsActive = true;
		InvulnerabilirtySkill.RemainingTime = InvulnerabilityDuration;
		CurrentMana = CurrentMana / 2; // 성공시 현재 마나의 절반 사용. 최소한 10 이상 사용하도록 디자인.

		// AI 경찰의 경계 수준 리셋 추후 AI class 작성하면 주석 해제
		// CapturingGuard->SetAlertLevel(EAlertLevel::Normal);

		UE_LOG(LogTemp, Warning, TEXT("Persuasion Success!! Invulnerability Time : %.1f (s)"), InvulnerabilityDuration);	
		return;
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Persuasion Failed"));
		return;
	}

}

void AAI_EscapeGameExCharacter::GameOver()
{
	// 추후 디자인
}

// 살금살금 걷기 스킬
void AAI_EscapeGameExCharacter::ActivateSilentMovement()
{
	// 필터 걸기
	// 이미 활성화 된 경우 중복사용 방지
	if (SilentMovementSkill.bIsActive)
	{
		return;
	}

	//마나가 충분한지 확인
	if (CurrentMana < SilentMovementSkill.ManaCost)
	{
		return;
	}
	
	// 마나 소모 후 스킬 활성화
	// 마나소모
	CurrentMana -= SilentMovementSkill.ManaCost;
	
	//스킬 활성화
	SilentMovementSkill.bIsActive = true;
	SilentMovementSkill.RemainingTime = SilentMovementSkill.Duration;
	
	// 실제 스킬 로직
	// 본 캐릭터의 MaxWalkSpeed를 낮춰주면 천천히 걷게 되는 점을 활용
	GetCharacterMovement()->MaxWalkSpeed = SlowWalkSpeed;

	UE_LOG(LogTemp, Warning, TEXT("Silent Movement Skill Activated for %.f (s)"), SilentMovementSkill.Duration);

	return;

}

void AAI_EscapeGameExCharacter::ActivateStealth()
{
	// 필터 걸기 ( 활성화 여부 및 마나 충분한지 여부 등 )
	// 활성화 여부 확인
	if (StealthSkill.bIsActive)
	{
		return;
	}
	// 마나가 충분한지
	if (CurrentMana < StealthSkill.ManaCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("You've not enough Mana"));
		return;
	}

	// 스킬 활성화
	// 마나 소모
	CurrentMana -= StealthSkill.ManaCost;

	StealthSkill.bIsActive = true;
	StealthSkill.RemainingTime = StealthSkill.Duration;

	// 스킬 관련 구체적 상태 부여 
	// 스텔스 효과로 반투명 머티리얼 적용
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp && StealthMaterial) // 메시가 없거나, 스텔스 머티리얼 설정을 안해뒀을 경우 작동방지
	{
		for (int32 i = 0;  i < MeshComp->GetNumMaterials(); i++)
		{
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(StealthMaterial, this);
			if (DynamicMaterial)
			{
				// i번째 슬롯 머티리얼을 DynamicMaterial에 대입된 머티리얼로 전환
				MeshComp->SetMaterial(i, DynamicMaterial);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Stealth Activated for %1.f (s)"), StealthSkill.Duration);
	return;
}

// 마나 리젠 간격(인터벌)로 반복 호출 통해 점진적 마나회복 로직 구현
void AAI_EscapeGameExCharacter::RegenerationMana()
{
	CurrentMana = FMath::Min(CurrentMana + ManaRegenRate, MaxMana);

}

void AAI_EscapeGameExCharacter::EndSilentMovement()
{
	// 예외 필터 걸기
	if (!SilentMovementSkill.bIsActive)
	{
		return;
	}

	// SilentMovementSkill을 Deactivate(비활성화)
	SilentMovementSkill.bIsActive = false;

	// 기본 걷기속도로 복원
	if (!bIsSprinting) // Sprint 중에 속도가 복원되면 낭패기 때문에 필터
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	}
	UE_LOG(LogTemp, Warning, TEXT("Silent Movement Ended"));
}

void AAI_EscapeGameExCharacter::EndStealth()
{
	// 필터 걸기
	if (!StealthSkill.bIsActive)
	{
		return;
	}

	// 스킬종료 기능 실행 
	StealthSkill.bIsActive = false;

	//원래 머티리얼로 복원
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp && OriginalMaterials.Num()>0) // 메시가 없거나, beginPlay에서 OriginalMaterial 선언에 성공했을 경우에만 실행.
	{
		for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++) // 현재 MeshComp의 Material slot 수가 BeginPlay시 저장된 Material의 수와 차이가 있을때 방지
		{
			if (OriginalMaterials[i])
			{
				// i번째 슬롯 머티리얼을 OriginalMaterials에 대입된 머티리얼로 전환
				MeshComp->SetMaterial(i, OriginalMaterials[i]);
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Stealth Ended"));

}

void AAI_EscapeGameExCharacter::EndInvulnerability()
{
	bIsInvulnerable = false;
	InvulnerabilirtySkill.bIsActive = false;

	UE_LOG(LogTemp, Warning, TEXT("Invulnerability Ended!"));
}

// 빠르게 달리기 Start/ Stop 함수
void AAI_EscapeGameExCharacter::StartSprint()
{
	bIsSprinting = true;

	// 소리 제거스킬의 Active 여부에 따라 속력 조절
	if (!SilentMovementSkill.bIsActive)
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed; // 달리기 속도 지정.
	}
}

void AAI_EscapeGameExCharacter::StopSprint()
{
	bIsSprinting = false;

	// 소리 제거스킬이 활성화 되어있으면 느린 이동속도 유지 
	if (SilentMovementSkill.bIsActive == false)
	{
		GetCharacterMovement()->MaxWalkSpeed = 500.f; // 달리기 속도 지정.
	}

	if (SilentMovementSkill.bIsActive == true)
	{
		GetCharacterMovement()->MaxWalkSpeed = SlowWalkSpeed; // 달리기 속도 지정.
	}
	
}

// DetectableInterface 함수 구현부
bool AAI_EscapeGameExCharacter::CanBeDetected_Implementation() const
{
	// 현재 잡히면 안되는 상태이지 확인하여 T or F 반화낳도록 코드 작성
	// 스텔스 상태거나 무적 상태면 Detect(감지) 불가
	return !(StealthSkill.bIsActive || bIsInvulnerable || bIsInStealthMode); 
}

bool AAI_EscapeGameExCharacter::CanMakeNoise_Implementation() const
{
	// 소음 발생 가능 상태일 때만 소음 감지
	return bCanMakeNoise && !SilentMovementSkill.bIsActive;
}

void AAI_EscapeGameExCharacter::CapturedByAI_Implementation()
{
	// AI 경찰 (Police)에 의해 호출되는 함수
	// bIsInvulnerable이면 캡쳐 불가하도록 필터링
	if (bIsInvulnerable)
	{
		UE_LOG(LogTemp, Display, TEXT("AI Capture Attempt Ignored. Player is Invulnerable."));
		return;
	}
	if (StealthSkill.bIsActive)
	{
		UE_LOG(LogTemp, Display, TEXT("AI Capture Attempt Ignored. Player is Stealth."));
		return;
	}
	
	// 잡혔다고 표시
	bIsCaptured = true;
	CaptureTime = GetWorld()->GetTimeSeconds();
	bIsInvulnerable = true; // 무적 처리
	InvulnerabilirtySkill.bIsActive = true;

	// PlayerCaptured 타이머의 중복 호출 방지를 위해 필터 작성
	if (!bIsTimerSet) 
	{
		bIsTimerSet = true;  // 한번 들어오면 바로 true로 바뀌니까 tick도 1번만 작동하게 가능
		UE_LOG(LogTemp, Display, TEXT("Player Captured by AI Police!! Player Can Use Persuasion Skill within %.1f seconds"), PersuasionWindow);
		GetWorldTimerManager().SetTimer(CapturedTimerHandle, this, &AAI_EscapeGameExCharacter::PlayerCaptured, PersuasionWindow, false); // 5초동안 기다리기 

	
	}
}

void AAI_EscapeGameExCharacter::PlayerCaptured()
{
	
	bIsInvulnerable = false;
	bIsCaptured = false;
	
	// 현재 설득에 성공한 상태인지 확인 
	if (!bIsSucceedPersuasion)
	{
		// 플레이어 캡쳐 처리(게임 플레이 내부에 효과 추가, UI 표시 등)
		// 게임모드 만든 후 플레이어 캡쳐 보고로직 작성. 추후 게임모드 디자인 후 업데이트
		
		// 플레이어 생명 1 감소 후 최신화
		Lives--;
		UE_LOG(LogTemp, Display, TEXT("You Just Loose A Life!. Remaning Live(s) : %d"), Lives);

		// 캡쳐 타이머 초기화
		bIsTimerSet = false;
	}

	// 플레이어 생명이 0일 경우 GameOver
	if (Lives <= 0)
	{
		GameOver();
	}

	bIsCaptured = true;
	CaptureTime = GetWorld()->GetTimeSeconds(); // 언제 붙잡혔는지 기록 추후 설득 시도시 유효시간 내에 있는지 확인하는용

	UE_LOG(LogTemp, Warning, TEXT("Player Captured by AI!"));
	


	// 설득 성공여부 확인 후 설득 실패시 플레이어 생명 감소 등 로직 작동. 추후 플레이어 내부에서 로직 작동하도록 분리

		/* 이미 플레이어가 Captured 된 경우, Pursasion만 변수로 두도록 무적상태와 스텔스상태는 예외처리.
	// 필터 걸기
	// 현재 무적 상태인지 확인
	if (bIsInvulnerable)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Capture attempt IGNORED - Player is Invulnerable"));
		return;
	}

	// 현재 스텔스 상태인지 확인
	if (StealthSkill.bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Capture attempt IGNORED - Player is Stealth"));
		return;
	}
	*/
}