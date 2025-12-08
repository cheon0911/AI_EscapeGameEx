#include "AIBaseGuard.h"

AAIBaseGuard::AAIBaseGuard()
{
 	
	PrimaryActorTick.bCanEverTick = true;

}

FVector AAIBaseGuard::GetLastKnownPlayerLocation_Implementation() const
{
	return FVector();
}

uint8 AAIBaseGuard::GetAlertLevel_Implementation() const
{
	return uint8();
}

APawn* AAIBaseGuard::GetTargetPlayer_Implementation() const
{
	return nullptr;
}

void AAIBaseGuard::ReceiveAlert_Implementation(const FVector& Location, uint8 AlertLevel, AActor* AlertSource)
{
}

void AAIBaseGuard::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAIBaseGuard::OnPlayerdetected(APawn* DetectedPawn)
{
}

void AAIBaseGuard::OnHearNoise(APawn* NoiseMaker, const FVector& Location, float Volume)
{
}

void AAIBaseGuard::OnSeePlayer(APawn* SeenPawn)
{
}

void AAIBaseGuard::SetAlertLevel(EAlertLevel NewAlertLevel)
{
}

void AAIBaseGuard::PursuePlayer()
{
}

void AAIBaseGuard::InvestigateLastKnownLocation()
{
}

void AAIBaseGuard::Patrol()
{
}

void AAIBaseGuard::CapturePlayer()
{
}

void AAIBaseGuard::AlertOtherGuards(const FVector& LocationToInvestigate)
{
}

void AAIBaseGuard::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
}

void AAIBaseGuard::ResetAlertTimer()
{
}

void AAIBaseGuard::HandleAlertTimeout()
{
}

void AAIBaseGuard::TimerTest()
{
}

void AAIBaseGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAIBaseGuard::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

