#include "AIBaseGuard.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "AI_EscapeGameExGameMode.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

AAIBaseGuard::AAIBaseGuard()
{
    PrimaryActorTick.bCanEverTick = true;

    // PawnSensing
    PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
    PawnSensingComponent->SetPeripheralVisionAngle(ViewAngle);
    PawnSensingComponent->SightRadius = SightRange;
    PawnSensingComponent->HearingThreshold = 1500.f;
    PawnSensingComponent->LOSHearingThreshold = 3000.f;
    PawnSensingComponent->SensingInterval = 1.f;
    PawnSensingComponent->bOnlySensePlayers = false;

    // Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Detection Sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(DetectionRange);
    DetectionSphere->SetCollisionProfileName(TEXT("Trigger"));

    // 기본 속성
    CurrentAlertLevel = EAlertLevel::Normal;
    TimeInCurrentAlertLevel = 0.f;
    LastKnownPlayerLocation = FVector::ZeroVector;
    TargetPlayer = nullptr;
    AIClass = TEXT("Class");
}

void AAIBaseGuard::BeginPlay()
{
    Super::BeginPlay();

    GuardController = Cast<AAIController>(GetController());

    if (PawnSensingComponent)
    {
        PawnSensingComponent->OnSeePawn.AddDynamic(this, &AAIBaseGuard::OnSeePlayer);
        PawnSensingComponent->OnHearNoise.AddDynamic(this, &AAIBaseGuard::OnHearNoise);
    }

    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAIBaseGuard::OnTargetPerceptionUpdated);
    }

    GetWorldTimerManager().SetTimer(TestTimerHandle, this, &AAIBaseGuard::TimerTest, 1.f, true);
}

void AAIBaseGuard::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    switch (CurrentAlertLevel)
    {
    case EAlertLevel::Normal:
        Patrol();
        break;

    case EAlertLevel::Suspicious:
    case EAlertLevel::Alert:
        InvestigateLastKnownLocation();
        break;

    case EAlertLevel::Pursuit:
        PursuePlayer();

        if (TargetPlayer &&
            FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation()) <= CaptureRange)
        {
            CapturePlayer();
        }
        break;
    }

    TimeInCurrentAlertLevel += DeltaTime;
}

void AAIBaseGuard::OnPlayerdetected(APawn* DetectedPawn)
{
    UE_LOG(LogTemp, Error, TEXT("Player Detected"));

    if (!DetectedPawn) return;

    if (DetectedPawn->GetClass()->ImplementsInterface(UDetectableInterface::StaticClass()))
    {
        if (IDetectableInterface::Execute_CanBeDetected(DetectedPawn))
        {
            LastKnownPlayerLocation = DetectedPawn->GetActorLocation();
            TargetPlayer = DetectedPawn;
            SetAlertLevel(EAlertLevel::Pursuit);
            AlertOtherGuards(LastKnownPlayerLocation);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("But the Player Can't be Detected"));
        }
    }
}

void AAIBaseGuard::OnHearNoise(APawn* NoiseMaker, const FVector& Location, float Volume)
{
    UE_LOG(LogTemp, Warning, TEXT("#1. AI Heard Something!"));

    if (!NoiseMaker) return;

    if (IDetectableInterface::Execute_CanMakeNoise(NoiseMaker))
    {
        LastKnownPlayerLocation = Location;

        if (Volume > 0.7f)
        {
            SetAlertLevel(EAlertLevel::Alert);
        }
        else if (Volume > 0.5f)
        {
            SetAlertLevel(EAlertLevel::Suspicious);
            AlertOtherGuards(Location);
        }
    }
}

void AAIBaseGuard::OnSeePlayer(APawn* SeenPawn)
{
    OnPlayerdetected(SeenPawn);
}

void AAIBaseGuard::SetAlertLevel(EAlertLevel NewAlertLevel)
{
    if (static_cast<uint8>(CurrentAlertLevel) >= static_cast<uint8>(NewAlertLevel))
        return;

    CurrentAlertLevel = NewAlertLevel;
    TimeInCurrentAlertLevel = 0.f;

    ResetAlertTimer();

    if (CurrentAlertLevel == EAlertLevel::Pursuit)
        GetCharacterMovement()->MaxWalkSpeed = PursuitSpeed;
    else
        GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

void AAIBaseGuard::PursuePlayer()
{
    if (!GuardController || !TargetPlayer) return;

    if (AIClass != "Police") return;

    GuardController->MoveToActor(TargetPlayer, -1.f, true, true, true);

    if (FMath::RandBool())
    {
        AlertOtherGuards(TargetPlayer->GetActorLocation());
    }
}

void AAIBaseGuard::InvestigateLastKnownLocation()
{
    if (!GuardController || LastKnownPlayerLocation.IsZero()) return;

    GuardController->MoveToLocation(LastKnownPlayerLocation, -1.f, true, true, true);

    float DistanceToTarget = FVector::Dist(GetActorLocation(), LastKnownPlayerLocation);

    if (DistanceToTarget < 100.f)
    {
        UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

        if (NavSystem)
        {
            FNavLocation RandomLocation;
            if (NavSystem->GetRandomPointInNavigableRadius(LastKnownPlayerLocation, 300.f, RandomLocation))
            {
                LastKnownPlayerLocation = RandomLocation.Location;
            }
        }
    }
}

void AAIBaseGuard::Patrol() {}

void AAIBaseGuard::CapturePlayer()
{
    if (!TargetPlayer) return;

    if (TargetPlayer->GetClass()->ImplementsInterface(UDetectableInterface::StaticClass()))
    {
        IDetectableInterface::Execute_CapturedByAI(TargetPlayer);
        SetAlertLevel(EAlertLevel::Normal);
        LastKnownPlayerLocation = FVector::ZeroVector;
        TargetPlayer = nullptr;
    }

    AGameModeBase* GM = UGameplayStatics::GetGameMode(GetWorld());

    if (GM && GM->GetClass()->ImplementsInterface(UGameRulesInterface::StaticClass()))
    {
        IGameRulesInterface::Execute_ReportPlayerCapture(GM, TargetPlayer, this);
    }
}

void AAIBaseGuard::AlertOtherGuards(const FVector& LocationToInvestigate)
{
    TArray<AActor*> Guards;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAIBaseGuard::StaticClass(), Guards);

    for (AActor* Guard : Guards)
    {
        if (Guard != this)
        {
            float Distance = FVector::Dist(GetActorLocation(), Guard->GetActorLocation());

            if (Distance <= CommunicationRange)
            {
                if (Guard->GetClass()->ImplementsInterface(UGuardInterface::StaticClass()))
                {
                    IGuardInterface::Execute_ReceiveAlert(
                        Guard,
                        LocationToInvestigate,
                        static_cast<uint8>(CurrentAlertLevel),
                        this
                    );
                }
            }
        }
    }
}

void AAIBaseGuard::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        APawn* DetectedPawn = Cast<APawn>(Actor);

        if (DetectedPawn)
        {
            if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
                OnSeePlayer(DetectedPawn);

            if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
                OnHearNoise(DetectedPawn, Stimulus.StimulusLocation, Stimulus.Strength);
        }
    }
    else
    {
        // 타겟이 유효하고, 현재 추격/경계 상태라면 추격을 해제합니다.
        if (Actor == TargetPlayer &&
            (CurrentAlertLevel == EAlertLevel::Pursuit || CurrentAlertLevel == EAlertLevel::Alert))
        {
            UE_LOG(LogTemp, Warning, TEXT("Player Lost/Stealth Activated. AI downgrading Alert Level."));

            // 타겟 해제: TargetPlayer가 nullptr이 되어 PursuePlayer()가 실행되지 않게 합니다.
            TargetPlayer = nullptr;

            // 경계 레벨을 Alert으로 낮춰 마지막 위치를 조사하도록 전환
            SetAlertLevel(EAlertLevel::Alert);

            // AlertTimer가 이후 자동으로 Suspicious -> Normal로 낮춰줄 것입니다.
        }
    }
}

void AAIBaseGuard::ResetAlertTimer()
{
    GetWorldTimerManager().ClearTimer(AlertTimerHandle);

    if (CurrentAlertLevel == EAlertLevel::Normal)
        return;

    GetWorldTimerManager().SetTimer(
        AlertTimerHandle,
        this,
        &AAIBaseGuard::HandleAlertTimeout,
        AlertTimeout,
        false
    );
}

void AAIBaseGuard::HandleAlertTimeout()
{
    switch (CurrentAlertLevel)
    {
    case EAlertLevel::Pursuit:
        SetAlertLevel(EAlertLevel::Alert);
        break;

    case EAlertLevel::Alert:
        SetAlertLevel(EAlertLevel::Suspicious);
        break;

    case EAlertLevel::Suspicious:
        SetAlertLevel(EAlertLevel::Normal);
        break;
    }
}

void AAIBaseGuard::TimerTest() {}

void AAIBaseGuard::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Interface Implementations
FVector AAIBaseGuard::GetLastKnownPlayerLocation_Implementation() const
{
    return LastKnownPlayerLocation;
}

uint8 AAIBaseGuard::GetAlertLevel_Implementation() const
{
    return static_cast<uint8>(CurrentAlertLevel);
}

APawn* AAIBaseGuard::GetTargetPlayer_Implementation() const
{
    return TargetPlayer;
}

void AAIBaseGuard::ReceiveAlert_Implementation(const FVector& Location, uint8 AlertLevel, AActor* AlertSource)
{
    LastKnownPlayerLocation = Location;

    EAlertLevel ReceivedLevel = static_cast<EAlertLevel>(AlertLevel);
    EAlertLevel FinalLevelToSet = ReceivedLevel;

    // Police가 아니면서 Pursuit 레벨을 받았다면, Alert으로 강제 전환
    if (AIClass != TEXT("Police") && ReceivedLevel == EAlertLevel::Pursuit)
    {
        FinalLevelToSet = EAlertLevel::Alert;
    }

    if (AlertLevel > static_cast<uint8>(CurrentAlertLevel))
    {
        SetAlertLevel(static_cast<EAlertLevel>(AlertLevel));
    }


	// 보고받은 위치로 이동하여 수사 개시
	InvestigateLastKnownLocation();
}