
#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class K_BLASTER_ONE_API AProjectileWeapon : public AWeaponBase
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;
};
