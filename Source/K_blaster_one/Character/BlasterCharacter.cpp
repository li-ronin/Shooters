#include "BlasterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"


ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// 摄像机弹簧臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600;
	CameraBoom->bUsePawnControlRotation = true;
	// 摄像机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;	// 让角色朝向移动方向，而不是镜头旋转方向
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);	// IE_Pressed是InputEventType
	// 绑定我们自定义的Action函数
	PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ThisClass::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::LookUp);
}

void ABlasterCharacter::MoveForward(float Value)
{
	if(Controller!=nullptr && Value!=0.f)
	{
		// 获取前方方向
		// 这里不使用GetActorForwardVector，是因为我们要获取的是Controller的前向而不是Actor的
		const FRotator YawRotation{0.f,Controller->GetControlRotation().Yaw, 0.f};	// Yaw是左右旋转
		const FVector Direction{ FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) };	// 用YawRotation创建一个FRotationMatriX并且获取它的X单位轴，返回的Vector就是Controller的前向
		// 移动
		AddMovementInput(Direction, Value);	// Value = 1向前， Value=-1向后
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if(Controller!=nullptr && Value!=0.f)
	{
		const FRotator YawRotation{0.f,Controller->GetControlRotation().Yaw, 0.f};	
		const FVector Direction{ FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) };	
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



