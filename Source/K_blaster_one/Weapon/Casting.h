// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casting.generated.h"

UCLASS()
class K_BLASTER_ONE_API ACasting : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasting();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
private:
	UPROPERTY(VisibleAnywhere, Category=CastingProperty)
	UStaticMeshComponent* CastingMesh;

	UPROPERTY(EditAnywhere, Category=CastingProperty)
	float ShellEjectImpulse;

	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;
public:	
	

};
