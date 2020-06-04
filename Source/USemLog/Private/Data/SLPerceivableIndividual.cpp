// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLPerceivableIndividual.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// Utils
#include "Utils/SLTagIO.h"

// Ctor
USLPerceivableIndividual::USLPerceivableIndividual()
{
	// Load the mask material
	static ConstructorHelpers::FObjectFinder<UMaterial>MaterialAsset(
		TEXT("Material'/USemLog/Individual/M_VisualIndividualMask.M_VisualIndividualMask'"));
	if (MaterialAsset.Succeeded())
	{
		VisualMaskMaterial = MaterialAsset.Object;		
		////VisualMaskDynamicMaterial = CreateDefaultSubobject<UMaterialInstanceDynamic>(TEXT("VisualMaskDynamicMaterial"));
		////VisualMaskDynamicMaterial->SetParentInternal(MaterialAsset.Object, false);
	}

	bIsInit = false;
	bIsLoaded = false;

	bMaskMaterialOn = false;
}

// Called before destroying the object.
void USLPerceivableIndividual::BeginDestroy()
{
	ApplyOriginalMaterials();
	Super::BeginDestroy();
}

// Create and set the dynamic material, the owners visual component
void USLPerceivableIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Set pointer to the semantic owner
bool USLPerceivableIndividual::Init(bool bReset)
{
	if (bReset)
	{
		bIsInit = false;
	}

	if (IsInit())
	{
		return true;
	}

	if (!Super::Init(bReset))
	{
		return false;
	}

	bIsInit = InitImpl();
	return bIsInit;
}

// Check if individual is initialized
bool USLPerceivableIndividual::IsInit() const
{
	return bIsInit && Super::IsInit();
}

// Load semantic data
bool USLPerceivableIndividual::Load(bool bReset)
{
	if (bReset)
	{
		bIsLoaded = false;
	}

	if (IsLoaded())
	{
		return true;
	}

	if (!IsInit())
	{
		return false;
	}

	if (!Super::Load(bReset))
	{
		if (!Init(bReset))
		{
			return false;
		}
	}

	bIsLoaded = LoadImpl();
	return bIsLoaded;
}

// Check if semantic data is succesfully loaded
bool USLPerceivableIndividual::IsLoaded() const
{
	return bIsLoaded && Super::IsLoaded();
}

// Save data to owners tag
bool USLPerceivableIndividual::ExportToTag(bool bOverwrite)
{
	bool bMarkDirty = false;
	bMarkDirty = Super::ExportToTag(bOverwrite) || bMarkDirty;
	if (!VisualMask.IsEmpty())
	{
		bMarkDirty = FSLTagIO::AddKVPair(SemanticOwner, TagTypeConst, "VisualMask", VisualMask, bOverwrite) || bMarkDirty;
	}
	if (!CalibratedVisualMask.IsEmpty())
	{
		bMarkDirty = FSLTagIO::AddKVPair(SemanticOwner, TagTypeConst, "CalibratedVisualMask", CalibratedVisualMask, bOverwrite) || bMarkDirty;
	}
	return bMarkDirty;
}

// Load data from owners tag
bool USLPerceivableIndividual::ImportFromTag(bool bOverwrite)
{
	bool bNewValue = false;
	bNewValue = Super::ImportFromTag(bOverwrite) || bNewValue;
	bNewValue = ImportVisualMaskFromTag(bOverwrite) || bNewValue;
	bNewValue = ImportCalibratedVisualMaskFromTag(bOverwrite) || bNewValue;
	return bNewValue;
}

// Apply visual mask material
bool USLPerceivableIndividual::ApplyVisualMaskMaterials(bool bReload)
{

	if (!bIsLoaded)
	{
		return false;
	}

	if (!bMaskMaterialOn || bReload)
	{
		for (int32 MatIdx = 0; MatIdx < VisualSMC->GetNumMaterials(); ++MatIdx)
		{
			VisualSMC->SetMaterial(MatIdx, VisualMaskDynamicMaterial);
		}
		bMaskMaterialOn = true;
		return true;
	}
	return false;
}

// Apply original materials
bool USLPerceivableIndividual::ApplyOriginalMaterials()
{
	if (!bIsLoaded)
	{
		return false;
	}

	if (bMaskMaterialOn)
	{
		int32 MatIdx = 0;
		for (const auto& Mat : OriginalMaterials)
		{
			VisualSMC->SetMaterial(MatIdx, Mat);
			++MatIdx;
		}
		bMaskMaterialOn = false;
		return true;
	}
	return false;
}

// Toggle between the visual mask and the origina materials
bool USLPerceivableIndividual::ToggleMaterials()
{
	if (bMaskMaterialOn)
	{
		return ApplyOriginalMaterials();
	}
	else
	{
		return ApplyVisualMaskMaterials();
	}
}

// Set  visual mask
void USLPerceivableIndividual::SetVisualMask(const FString& InVisualMask, bool bReload, bool bClearCalibratedValue)
{
	// Clear the calibrated color in case of a new visual mask value
	if (!VisualMask.Equals(InVisualMask) && bClearCalibratedValue) 
	{
		CalibratedVisualMask = ""; 
	}
	
	// Set the new visual mask
	VisualMask = InVisualMask;

	// Update the dynamic material
	ApplyVisualMaskColorToDynamicMaterial();
	
	// If the mask visualization is active, dynamically update the colors
	if (bMaskMaterialOn && bReload)
	{
		ApplyVisualMaskMaterials(true);
	}
}

// Apply color to the dynamic material
bool USLPerceivableIndividual::ApplyVisualMaskColorToDynamicMaterial()
{	
	if (VisualMaskDynamicMaterial && HasVisualMask())
	{
		VisualMaskDynamicMaterial->SetVectorParameterValue(FName("Color"), FColor::FromHex(VisualMask));
		return true;
	}
	return false;
}

// Import visual mask from tag, true if new value is written
bool USLPerceivableIndividual::ImportVisualMaskFromTag(bool bOverwrite)
{
	bool bNewValue = false;
	if (VisualMask.IsEmpty() || bOverwrite)
	{
		const FString PrevVal = VisualMask;
		SetVisualMask(FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "VisualMask"));
		bNewValue = VisualMask.Equals(PrevVal);
		if (VisualMask.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No VisualMask value could be imported from %s's tag.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	return bNewValue;
}

// Import calibrated visual mask from tag, true if new value is written
bool USLPerceivableIndividual::ImportCalibratedVisualMaskFromTag(bool bOverwrite)
{
	bool bNewValue = false;
	if (CalibratedVisualMask.IsEmpty() || bOverwrite)
	{
		const FString PrevVal = CalibratedVisualMask;
		SetCalibratedVisualMask(FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "CalibratedVisualMask"));
		bNewValue = CalibratedVisualMask.Equals(PrevVal);
		if (CalibratedVisualMask.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No CalibratedVisualMask value could be imported from %s's tag.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	return bNewValue;
}

// Private init implementation
bool USLPerceivableIndividual::InitImpl()
{
	if (!VisualMaskMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual mask material asset, init failed.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}
	VisualMaskDynamicMaterial = UMaterialInstanceDynamic::Create(VisualMaskMaterial, this);

	if (!VisualMaskDynamicMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has no VisualMaskDynamicMaterial, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(SemanticOwner))
	{
		if (UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
		{
			VisualSMC = SMC;
			OriginalMaterials = SMC->GetMaterials();
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

// Private load implementation
bool USLPerceivableIndividual::LoadImpl()
{
	bool bSuccess = true;
	if (!HasVisualMask())
	{
		if (!ImportVisualMaskFromTag())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has no VisualMask, tag import failed as well.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			bSuccess = false;
		}
		return false;
	}

	VisualMaskDynamicMaterial->SetVectorParameterValue(FName("Color"), FColor::FromHex(VisualMask));

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's load failed.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	return bSuccess;
}