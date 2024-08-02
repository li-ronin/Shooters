#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	OurCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	
}

// 每帧调用
void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if(OurCharacter == nullptr)
	{
		OurCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if(OurCharacter == nullptr)return;

	FVector Velocity = OurCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size(); // 模长Sqrt(X*X + Y*Y + Z*Z);
	
	bIsInAir = OurCharacter->GetCharacterMovement()->IsFalling();
	
	bIsAccelerating = OurCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
}
