
#include "CalculateVerticesActor.h"
#include "Components/ArrowComponent.h"

ACalculateVerticesActor::ACalculateVerticesActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	SetRootComponent(SceneRoot);

	LeftArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Left Arrow")); 
	LeftArrow->SetupAttachment(SceneRoot);
	LeftArrow->SetRelativeLocation(LeftLocation);

	RightArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Right Arrow"));
	RightArrow->SetupAttachment(SceneRoot);
	RightArrow->SetRelativeLocation(RightLocation);
}

void ACalculateVerticesActor::SetThisActorTransform(FTransform ActorTransform)
{
	SetActorTransform(ActorTransform);
}

FVector ACalculateVerticesActor::GetLeftArrowLocation()
{
	return LeftArrow->GetComponentLocation();
}

FVector ACalculateVerticesActor::GetRightArrowLocation()
{
	return RightArrow->GetComponentLocation();
}

void ACalculateVerticesActor::SetWidth(int32 Width)
{
	LeftLocation.Y *= Width;
	RightLocation.Y *= Width;

	LeftArrow->SetRelativeLocation(LeftLocation);
	RightArrow->SetRelativeLocation(RightLocation);
}

