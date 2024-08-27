

#include "Casting.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasting::ACasting()
{
	PrimaryActorTick.bCanEverTick = false;
	CastingMesh = CreateDefaultSubobject<UStaticMeshComponent>("CastingMesh");
	SetRootComponent(CastingMesh);
	CastingMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	CastingMesh->SetSimulatePhysics(true);	
	CastingMesh->SetEnableGravity(true);
	CastingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectImpulse = 100.f;
}

void ACasting::BeginPlay()
{
	Super::BeginPlay();
	CastingMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	CastingMesh->AddImpulse(GetActorForwardVector()*ShellEjectImpulse);
}

void ACasting::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACasting::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// 子弹落地声音
	if(ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	
	Destroy();
}