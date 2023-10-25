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

#include "StreamlineLibrary.h"
#include "StreamlineLibraryPrivate.h"
#include "StreamlineLibraryReflex.h"
#include "StreamlineLibraryDLSSG.h"


#define LOCTEXT_NAMESPACE "FStreammlineBlueprintModule"
DEFINE_LOG_CATEGORY(LogStreamlineBlueprint);

void FStreamlineBlueprintModule::StartupModule()
{
	UStreamlineLibraryReflex::Startup();
	UStreamlineLibraryDLSSG::Startup();
}

void FStreamlineBlueprintModule::ShutdownModule()
{
	UStreamlineLibraryDLSSG::Shutdown();
	UStreamlineLibraryReflex::Shutdown();
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStreamlineBlueprintModule, StreamlineBlueprint)

