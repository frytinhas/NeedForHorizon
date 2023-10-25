/*
* Copyright (c) 2022 - 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
*
* NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
* property and proprietary rights in and to this material, related
* documentation and any modifications thereto. Any use, reproduction,
* disclosure or distribution of this material and related documentation
* without an express license agreement from NVIDIA CORPORATION or
* its affiliates is strictly prohibited.
*/
#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/CoreDelegates.h"

#include "StreamlineLibraryDLSSG.generated.h"



class FDelegateHandle;

#ifdef __INTELLISENSE__
#define WITH_STREAMLINE 1
#endif

UENUM(BlueprintType)
enum class UStreamlineDLSSGSupport : uint8
{
	Supported UMETA(DisplayName = "Supported"),

	NotSupported UMETA(DisplayName = "Not Supported"),
	NotSupportedIncompatibleHardware UMETA(DisplayName = "Incompatible Hardware", ToolTip = "DLSS-G requires an NVIDIA RTX GPU"),
	NotSupportedDriverOutOfDate UMETA(DisplayName = "Driver Out of Date", ToolTip = "The driver is outdated. Also see GetDLSSGMinimumDriverVersion"),
	NotSupportedOperatingSystemOutOfDate UMETA(DisplayName = "Operating System Out of Date", ToolTip = "DLSS requires at least Windows 10 Fall 2017 Creators Update 64-bit, (v1709, build 16299)"),
	NotSupportedByPlatformAtBuildTime UMETA(DisplayName = "Platform Not Supported At Build Time", ToolTip = "This platform doesn't not support DLSS-G at build time. Currently DLSS-G is only supported on Windows 64"),
	NotSupportedIncompatibleAPICaptureToolActive UMETA(DisplayName = "Incompatible API Capture Tool Active", ToolTip = "DLSS-G is not compatible with an active API capture tool such as RenderDoc.")
};

UENUM(BlueprintType)
enum class UStreamlineDLSSGMode : uint8
{
	Off UMETA(DisplayName = "Off"),
	On UMETA(DisplayName = "On"),
};

UCLASS(MinimalAPI)
class  UStreamlineLibraryDLSSG : public UBlueprintFunctionLibrary
{
	friend class FStreamlineBlueprintModule;
	GENERATED_BODY()
public:

	/** Checks whether DLSS-G is supported by the current GPU. Further details can be retrieved via QueryDLSSGSupport*/
	UFUNCTION(BlueprintPure, Category = "DLSS-G (Streamline)", meta = (DisplayName = "Is NVIDIA DLSS-G Supported"))
	static STREAMLINEBLUEPRINT_API bool IsDLSSGSupported();

	/** Checks whether DLSS-G is supported by the current GPU	*/
	UFUNCTION(BlueprintPure, Category = "DLSS-G (Streamline)", meta = (DisplayName = "Query NVIDIA DLSS-G Support"))
	static STREAMLINEBLUEPRINT_API UStreamlineDLSSGSupport QueryDLSSGSupport();

	/** If QueryDLSS-G Support returns "NotSupportedDriverOutOfDate", then MinDriverVersionMajor and MinDriverVersionMinor contains the required driver version.*/
	UFUNCTION(BlueprintPure, Category = "DLSS-G (Streamline)", meta = (DisplayName = "Get DLSS-G Minimum DriverVersion"))
	static STREAMLINEBLUEPRINT_API void GetDLSSGMinimumDriverVersion(int32& MinDriverVersionMajor, int32& MinDriverVersionMinor);

	/** Checks whether a DLSS-G mode is supported */
	UFUNCTION(BlueprintPure, Category = "DLSS-G (Streamline)", meta = (DisplayName = "Is DLSS-G Mode Supported"))
	static STREAMLINEBLUEPRINT_API bool IsDLSSGModeSupported(UStreamlineDLSSGMode DLSSGMode);

	/** Retrieves all supported DLSS-G modes. Can be used to populate UI */
	UFUNCTION(BlueprintPure, Category = "DLSS-G (Streamline)", meta = (DisplayName = "Get Supported DLSS-G Modes"))
	static STREAMLINEBLUEPRINT_API TArray<UStreamlineDLSSGMode> GetSupportedDLSSGModes();

	/** Sets the console variables to enable/disable DLSS-G*/
	UFUNCTION(BlueprintCallable, Category = "DLSS-G (Streamline)", meta = (DisplayName = "Set DLSS-G Mode"))
	static STREAMLINEBLUEPRINT_API void SetDLSSGMode(UStreamlineDLSSGMode DLSSGMode);

	/* Reads the console variables to infer the current DLSS-G mode*/
	UFUNCTION(BlueprintPure, Category = "DLSS-G (Streamline)", meta = (DisplayName = "Get DLSS-G Mode"))
	static STREAMLINEBLUEPRINT_API UStreamlineDLSSGMode GetDLSSGMode();

	/* Find a reasonable default DLSS-G mode based on current hardware */
	UFUNCTION(BlueprintPure, Category = "DLSS-G (Streamline)", meta = (DisplayName = "Get Default DLSS-G Mode"))
	static STREAMLINEBLUEPRINT_API UStreamlineDLSSGMode GetDefaultDLSSGMode();

	/* Returns the actual framerate and number of frames presented, whether DLSS-G is active or not */
	UFUNCTION(BlueprintPure, Category = "DLSS-G (Streamline)", meta = (DisplayName = "Get DLSS-G frame frame rate and presented frames"))
	static STREAMLINEBLUEPRINT_API void GetDLSSGFrameTiming(float& FrameRateInHertz, int32& FramesPresented);


	static void Startup();
	static void Shutdown();
private:
	static UStreamlineDLSSGSupport DLSSGSupport;

#if WITH_STREAMLINE
	static int32 MinDLSSGDriverVersionMinor;
	static int32 MinDLSSGDriverVersionMajor;
	static bool bDLSSGLibraryInitialized;

	static bool TryInitDLSSGLibrary();

#if !UE_BUILD_SHIPPING
	struct FDLSSErrorState
	{
		bool bIsDLSSGModeUnsupported = false;
		UStreamlineDLSSGMode InvalidDLSSGMode = UStreamlineDLSSGMode::Off;
	};

	static FDLSSErrorState DLSSErrorState;

	static void GetDLSSOnScreenMessages(TMultiMap<FCoreDelegates::EOnScreenMessageSeverity, FText>& OutMessages);
	static FDelegateHandle DLSSOnScreenMessagesDelegateHandle;
#endif




#endif
};

