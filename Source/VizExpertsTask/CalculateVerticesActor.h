
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CalculateVerticesActor.generated.h"

class UArrowComponent;

UCLASS()
class VIZEXPERTSTASK_API ACalculateVerticesActor : public AActor
{
	GENERATED_BODY()
	
public:	
	/** Constructor */
	ACalculateVerticesActor();

protected:
	/** Scene Root */
	UPROPERTY(EditAnywhere, Category="Components")
	USceneComponent* SceneRoot = nullptr;

	/** Left arrow for the left vertices */
	UPROPERTY(EditAnywhere, Category = "Components")
	UArrowComponent* LeftArrow = nullptr;

	/** Right arrow for the right vertices */
	UPROPERTY(EditAnywhere, Category = "Components")
	UArrowComponent* RightArrow = nullptr;

public:
	/** Set the actor transform according to spline location along the distance */
	void SetThisActorTransform(FTransform ActorTransform);

	/** Return left arrow world location */
	FVector GetLeftArrowLocation();

	/** Return right arrow world location */
	FVector GetRightArrowLocation();

	/** Set the width of arrows */
	void SetWidth(int32 Width);

private:
	/** Default location of the left location */
	FVector LeftLocation = FVector(0.f, -50.f, 0.f);

	/** Default location of the right location */
	FVector RightLocation = FVector(0.f, 50.f, 0.f);
};
