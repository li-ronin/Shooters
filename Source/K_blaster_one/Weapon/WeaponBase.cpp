
#include "Components/SphereComponent.h"
#include "WeaponBase.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;	// * If true, this actor will replicate to remote machines
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponSKMesh");
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 武器的碰撞初始状态：禁用

	// 武器区域球体，与玩家重叠时给予玩家
	AreaSphere = CreateDefaultSubobject<USphereComponent>("AreaSphere");
	AreaSphere->SetupAttachment(RootComponent);
	// 这种重叠事件应该放在服务器上检测，所以客户端本地我们忽略碰撞
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 我们想在服务器上启用AreaSphere的碰撞，在BeginPlay中这么做
}


void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
	if(HasAuthority())	// 若当前为服务器 GetLocalRole() == ROLE_Authority
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	}
}


void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

