#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralSplineActor.generated.h"

class USplineComponent;
class UProceduralMeshComponent;
class ACalculateVerticesActor;

UCLASS()
class VIZEXPERTSTASK_API AProceduralSplineActor : public AActor
{
	GENERATED_BODY()
	
public:	
	/** Constructor */
	AProceduralSplineActor();

	/** Reads the data from the file and returns it */
	UFUNCTION()
	static FString ReadStringFromFile(FString& InFilePath, bool& bOutSuccess);
	
protected:
	/** Begin play event */
	virtual void BeginPlay() override;

	/** Sets the spline by accessing the data from the given file and adding points accordingly */
	void SetupSplineComponent();

	/** Scene root for the class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot = nullptr;

	/** Spline component */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* Spline = nullptr;

	/** Procedural mesh component */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UProceduralMeshComponent* ProceduralMesh = nullptr;

	/** Path that contains the text file frow which we have to read the spline points data, to be set by user */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh Data")
	FString SplinePointsFilePath = "C:/Users/mails/Downloads/Assignment Points.txt";

	/** Path of the image file which conatins the PNG/JPEG image to make texture from, to be set by user */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh Data")
	FString ImageFilePath = "C:/Users/mails/Downloads/Assignment Points.txt";

	/** Width of our procedurally generated mesh, to be set by user */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh Data")
	int32 WidthOfGeometry = 1;

	/** Debug actor helps to calculate the locations of vertices for the procedurally generated mesh to form */
	static TSubclassOf<ACalculateVerticesActor> SpawnedCalculateVerticesActor;

	/** Material for the generated mesh */
	UMaterialInterface* BaseMaterial = nullptr;

	/** Created the material from the given image by the user */
	void CreateMaterialFromImage(const FString& InFilePath);

	/** Loads and returns the texture after getting the PNG/JPEG file from the system */
	UTexture2D* LoadTextureFromFile(const FString& InFilePath);

private:
	/** Stores the location at which spline points are to be added */
	TArray<TArray<int>> SplinePointsLocation;

	/** Stores the count of Spline points */
	int SplinePoints = 0;

	/** Stores the location of all the vertices where mesh points has to be spawned for the procedurally generated mesh */
	TArray<FVector3d> MeshVertices;

	/** Stores the vertices count of every created triangle */
	TArray<int32> MeshTriangles;

	/** Calculates the number of spline points present in the spline */
	void CalculateSplinePoints();

	/** Calculates and sets the vertices for procedural mesh generation */
	void SetMeshVerticesLocation();

	/** Set the traingles vaule for the procedural mesh generation */
	void SetMeshTriangles();

	/** Create the procedural mesh */
	void CreateProceduralMesh();
};
