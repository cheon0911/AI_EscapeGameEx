// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BaseItem.generated.h"


UCLASS()
class AI_ESCAPEGAMEEX_API ABaseItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 충돌감지 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionSphere;
	
	// 아이템 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ItemMesh;

	// 플레이어와 충돌시 호출되는 함수
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// 아이템 타입 식별을 위한 열거형
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ItemType;

	// 아이템 타입 반환 함수
	int32 GetItemType() const { return ItemType; }

	// 현재 위치한 층 확인하는 기준점 변수에 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float SecondFloor;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Item")
	bool bIsOn2F;

	// 2층 여부 확인하는 함수
	void CheckFloorLevel();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
