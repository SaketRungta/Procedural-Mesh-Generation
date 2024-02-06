#include "ProceduralSplineActor.h"
#include "HAL/PlatformFileManager.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "Misc/Char.h"
#include "ProceduralMeshComponent.h"
#include "CalculateVerticesActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Modules/ModuleManager.h"

TSubclassOf<ACalculateVerticesActor> AProceduralSplineActor::SpawnedCalculateVerticesActor = nullptr;

AProceduralSplineActor::AProceduralSplineActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	SetRootComponent(SceneRoot);

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(SceneRoot);

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	ProceduralMesh->SetupAttachment(SceneRoot);

	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> asset(TEXT("Material'/Game/Materials/MI_MainMaterial.MI_MainMaterial'"));
		BaseMaterial = asset.Object;
	}

	{
		static ConstructorHelpers::FObjectFinder<UClass> asset(TEXT("'/Script/VizExpertsTask.CalculateVerticesActor'"));
		SpawnedCalculateVerticesActor = (UClass*)asset.Object;
	}
}

FString AProceduralSplineActor::ReadStringFromFile(FString& InFilePath, bool& bOutSuccess)
{
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*InFilePath))
	{
		bOutSuccess = false;
		return FString();
	}

	FString RetString = "";

	if (!FFileHelper::LoadFileToString(RetString , *InFilePath))
	{
		bOutSuccess = false;
		return FString();
	}

	bOutSuccess = true;
	return RetString;
}

void AProceduralSplineActor::BeginPlay()
{
	Super::BeginPlay();
	
	SetupSplineComponent();
	CreateProceduralMesh();
}

void AProceduralSplineActor::SetupSplineComponent()
{
	bool bWasSuccess = false;

	FString ReadString = ReadStringFromFile(SplinePointsFilePath, bWasSuccess);

	// If file reading failed
	if (!bWasSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Error reading File"));
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, false);
		return;
	}

	// If empty file detected
	if (ReadString == "")
	{
		UE_LOG(LogTemp, Error, TEXT("Empty File"));
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, false);
		return;
	}

	// Get the no of points present in the file
	int points = 1;
	for (int i = 0; i < ReadString.Len(); i++)
	{
		if (ReadString[i] == '\n')
		{
			points++;
		}
	}

	// Set the dummy data
	SplinePointsLocation.SetNum(points);
	for (int i = 0; i < points; i++)
	{
		SplinePointsLocation[i].SetNum(2);
	}

	// Gets the points and multiplies it by hundered to have some distance along the points as (1,1) will be very bear to  (2,3) in world space so now it is (100,100) and (200,300)
	for (int i = 0; i < points; i++)
	{
		int u = i * 5, v = (i * 5) + 2;
		SplinePointsLocation[i][0] = (ReadString[u] - '0') * 100.f;
		SplinePointsLocation[i][1] = (ReadString[v] - '0') * 100.f;
	}
	
	// Sets the location of the default first and second spline point
	Spline->SetLocationAtSplinePoint(0, FVector3d(SplinePointsLocation[0][0], SplinePointsLocation[0][1], 0.f), ESplineCoordinateSpace::Local);
	Spline->SetLocationAtSplinePoint(1, FVector3d(SplinePointsLocation[1][0], SplinePointsLocation[1][1], 0.f), ESplineCoordinateSpace::Local);

	// Adds and sets the location os the rest spline points
	for (int i = 0; i < points - 2; i++)
	{
		Spline->AddSplinePoint(FVector(SplinePointsLocation[i + 2][0], SplinePointsLocation[i + 2][1], 0), ESplineCoordinateSpace::Local);
	}
}

void AProceduralSplineActor::CalculateSplinePoints()
{
	float SplineLength = Spline->GetSplineLength();
	// Dividing by 20 so that we can have some space betweeen consecutive points in the spline
	SplinePoints = SplineLength / 20.f;
}

void AProceduralSplineActor::SetMeshVerticesLocation()
{
	CalculateSplinePoints();

	// Add required dummy points
	for (int i = 0; i < SplinePoints - 1; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			MeshVertices.Add(FVector(0.f, 0.f, 0.f));
		}
	}

	// Spawn the debug actor to help calculate location of vertices
	ACalculateVerticesActor* SpawnedActor = GetWorld()->SpawnActor<ACalculateVerticesActor>(SpawnedCalculateVerticesActor, FTransform());

	// Calculate and initialize the spline points of mesh vertices
	for (int i = 0; i < SplinePoints; i++)
	{
		float dist = i * 20.f;
		if (SpawnedCalculateVerticesActor)
		{
			FTransform SpawnTransform = Spline->GetTransformAtDistanceAlongSpline(dist, ESplineCoordinateSpace::World);
			SpawnedActor->SetActorTransform(SpawnTransform);
			SpawnedActor->SetWidth(WidthOfGeometry);

			FVector LeftArrowLocation = SpawnedActor->GetLeftArrowLocation(), RightArrowLocation = SpawnedActor->GetRightArrowLocation();
			if (i == 0)
			{
				MeshVertices[0] = LeftArrowLocation;
				MeshVertices[5] = LeftArrowLocation;
				MeshVertices[1] = RightArrowLocation;
			}
			else if (i == SplinePoints - 1)
			{
				MeshVertices[(i * 6) - 2] = LeftArrowLocation;
				MeshVertices[(i * 6) - 4] = RightArrowLocation;
				MeshVertices[(i * 6) - 3] = RightArrowLocation;
			}
			else
			{
				int k = (i + 1) * 6;
				MeshVertices[k - 1] = LeftArrowLocation;
				MeshVertices[k - 6] = LeftArrowLocation;
				MeshVertices[k - 8] = LeftArrowLocation;
				MeshVertices[k - 10] = RightArrowLocation;
				MeshVertices[k - 9] = RightArrowLocation;
				MeshVertices[k - 5] = RightArrowLocation;
			}
		}
	}

	// Set the vertices of mesh to be 0 locally in z axis
	for (int i = 0; i < MeshVertices.Num(); i++)
	{
		MeshVertices[i].Z = 0;
	}
}

void AProceduralSplineActor::SetMeshTriangles()
{
	for (int i = 0; i < SplinePoints * 6; i++)
	{
		MeshTriangles.Add(i);
	}
}

void AProceduralSplineActor::CreateProceduralMesh()
{
	SetMeshVerticesLocation();
	SetMeshTriangles();

	// UV for a square
	TArray<FVector2D> Temp;
	Temp.Add(FVector2D(0, -1));
	Temp.Add(FVector2D(0, 1));
	Temp.Add(FVector2D(1, 1));
	Temp.Add(FVector2D(1, 1));
	Temp.Add(FVector2D(1, -1));
	Temp.Add(FVector2D(0, -1));

	// UV for all the spline points
	TArray<FVector2D> UV0;
	for (int i = 0; i < SplinePoints - 1; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			UV0.Add(Temp[j]);
		}
	}

	// Create the procedural mesh
	ProceduralMesh->CreateMeshSection(0, MeshVertices, MeshTriangles, TArray<FVector>(), UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
	CreateMaterialFromImage(ImageFilePath);
}

void AProceduralSplineActor::CreateMaterialFromImage(const FString& InFilePath)
{
	// Load the texture file
	UTexture2D* Texture = LoadTextureFromFile(InFilePath);

	if (Texture)
	{
		// Create dynamic material instance
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);

		// Set the texture parameter in the material
		DynamicMaterial->SetTextureParameterValue("CustomTexture", Texture);

		// Apply the material to the mesh or other renderable object
		ProceduralMesh->SetMaterial(0, DynamicMaterial);
	}
}

UTexture2D* AProceduralSplineActor::LoadTextureFromFile(const FString& InFilePath)
{
	// Load the image file
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *InFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load image file: %s"), *InFilePath);
		return nullptr;
	}

	// Create image wrapper
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create image wrapper for file: %s"), *InFilePath);
		return nullptr;
	}

	// Decompress image data
	TArray<uint8> UncompressedData;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedData))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to decompress image data for file: %s"), *InFilePath);
		return nullptr;
	}

	// Create texture
	UTexture2D* Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
	if (!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create texture for file: %s"), *InFilePath);
		return nullptr;
	}

	// Update texture data
	void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, UncompressedData.GetData(), UncompressedData.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();

	// Update texture settings
	Texture->UpdateResource();

	return Texture;
}
