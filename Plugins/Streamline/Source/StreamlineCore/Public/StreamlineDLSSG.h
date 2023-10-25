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
class FStreamlineRHI;
void RegisterStreamlineDLSSGHooks(FStreamlineRHI* InStreamlineRHI);
void UnregisterStreamlineDLSSGHooks();

bool IsDLSSGActive();

extern STREAMLINECORE_API bool IsStreamlineDLSSGSupported();

extern STREAMLINECORE_API void GetStreamlineDLSSGFrameTiming(float& FrameRateInHertz, int32& FramesPresented);


class FRHICommandListImmediate;
struct FRHIStreamlineArguments;
class FSceneViewFamily;
class FRDGBuilder;
void AddStreamlineDLSSGStateRenderPass(FRDGBuilder& GraphBuilder, uint32 ViewID);
void SetStreamlineDLSSGState(FRHICommandListImmediate& RHICmdList, uint32 ViewID);
void BeginRenderViewFamilyDLSSG(FSceneViewFamily& InViewFamily);
void UpdateDLSSGFrameTime();