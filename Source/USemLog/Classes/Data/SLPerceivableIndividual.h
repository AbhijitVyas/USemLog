// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/SLBaseIndividual.h"
#include "SLPerceivableIndividual.generated.h"

/**
 * 
 */
UCLASS()
class USEMLOG_API USLPerceivableIndividual : public USLBaseIndividual
{
	GENERATED_BODY()

public:
    // Ctor
    USLPerceivableIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
    virtual void PostInitProperties() override;

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset = false);

    // Check if individual is initialized
    virtual bool IsInit() const;

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset = false);

    // Check if semantic data is succesfully loaded
    virtual bool IsLoaded() const;

    // Save data to owners tag
    virtual bool ExportToTag(bool bOverwrite = false) override;

    // Load data from owners tag
    virtual bool ImportFromTag(bool bOverwrite = false) override;

    // Apply visual mask material
    bool ApplyVisualMaskMaterials(bool bReload = false);

    // Apply original materials
    bool ApplyOriginalMaterials();

    // Toggle between the visual mask and the origina materials
    bool ToggleMaterials();
    
    /* Visual mask */
    void SetVisualMask(const FString& InVisualMask, bool bReload = true, bool bClearCalibratedValue = true);
    FString GetVisualMask() const { return VisualMask; };
    bool HasVisualMask() const { return !VisualMask.IsEmpty(); };

    /* Calibrated visual mask */
    void SetCalibratedVisualMask(const FString& InCalibratedVisualMask) { CalibratedVisualMask = InCalibratedVisualMask; };
    FString GetCalibratedVisualMask() const { return CalibratedVisualMask; };
    bool HasCalibratedVisualMask() const { return !CalibratedVisualMask.IsEmpty(); };

private:
    // Import visual mask from tag, true if new value is written
    bool ImportVisualMaskFromTag(bool bOverwrite = false);

    // Import calibrated visual mask from tag, true if new value is written
    bool ImportCalibratedVisualMaskFromTag(bool bOverwrite = false);

    // Private init implementation
    bool InitImpl();

    // Private load implementation
    bool LoadImpl();

    // Apply color to the dynamic material
    bool ApplyVisualMaskColorToDynamicMaterial();

protected:
    // Mask color as hex
    UPROPERTY(EditAnywhere, Category = "SL")
    FString VisualMask;

    // Runtime calibrated mask color as hex
    UPROPERTY(EditAnywhere, Category = "SL")
    FString CalibratedVisualMask;

    // Material template used for creating dynamic materials
    UPROPERTY()
    class UMaterial* VisualMaskMaterial;

    // Dynamic material used for setting various mask colors
    UPROPERTY()
    class UMaterialInstanceDynamic* VisualMaskDynamicMaterial;

    // Cached original materials
    UPROPERTY()
    TArray<class UMaterialInterface*> OriginalMaterials;

    // True if the visual masks are currently active on the semantic owner
    UPROPERTY(EditAnywhere, Category = "SL")
    bool bMaskMaterialOn;

private:
    // The visual component of the owner
    UPROPERTY()
    class UStaticMeshComponent* VisualSMC;

    //// State of the individual
    //uint8 bIsInitPrivate : 1;
    //uint8 bIsLoadedPrivate : 1;
};