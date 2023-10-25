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

#include "StreamlineDLSSG.h"
#include "StreamlineCore.h"
#include "StreamlineCorePrivate.h"
#include "StreamlineAPI.h"
#include "StreamlineRHI.h"
#include "sl_helpers.h"
#include "sl_dlss_g.h"
#include "UIHintExtractionPass.h"

#include "CoreMinimal.h"
#include "Framework/Application/SlateApplication.h"
#include "RenderGraphBuilder.h"
#include "Runtime/Launch/Resources/Version.h"
#include "ScenePrivate.h"
#include "SystemTextures.h"

static FDelegateHandle OnPreRHIViewportCreateHandle;
static FDelegateHandle OnPostRHIViewportCreateHandle;
static FDelegateHandle OnSlateWindowDestroyedHandle;

static FDelegateHandle OnPreResizeWindowBackBufferHandle;
static FDelegateHandle OnPostResizeWindowBackBufferHandle;
static FDelegateHandle OnBackBufferReadyToPresentHandle;
static TAutoConsoleVariable<int32> CVarStreamlineDLSSGEnable(
	TEXT("r.Streamline.DLSSG.Enable"), 0,
	TEXT("Enable/disble DLSSG (default = 0)\n"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarStreamlineDLSSGAdjustMotionBlurTimeScale(
	TEXT("r.Streamline.DLSSG.AdjustMotionBlurTimeScale"), 1,
	TEXT("When DLSS-G is active, adjust the motion blur timescale based on the generated frames (default = 1)\n"),
	ECVF_Default);


static TAutoConsoleVariable<bool> CVarStreamlineTagUIColorAlpha(
	TEXT("r.Streamline.TagUIColorAlpha"),
	true,
	TEXT("Pass UI color and alpha into Streamline (default = true)\n"),
	ECVF_RenderThreadSafe);


static TAutoConsoleVariable<bool> CVarStreamlineEditorTagUIColorAlpha(
	TEXT("r.Streamline.Editor.TagUIColorAlpha"),
	false,
	TEXT("Experimental: Pass UI color and alpha into Streamline in Editor PIE windows (default = false)\n"),
	ECVF_RenderThreadSafe);


static int32 NumDLSSGInstances = 0;


BEGIN_SHADER_PARAMETER_STRUCT(FSLUIHintTagShaderParameters, )
SHADER_PARAMETER_RDG_TEXTURE(Texture2D, UIColorAndAlpha)

// Fake output to trigger pass execution
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION == 25)
SHADER_PARAMETER_RDG_TEXTURE(Texture2D, FakeOutput)
#else
RDG_TEXTURE_ACCESS(FakeOutput, ERHIAccess::UAVCompute)
#endif
END_SHADER_PARAMETER_STRUCT()



bool ShouldTagStreamlineBuffers()
{
	static bool bStreamlineForceTagging = FParse::Param(FCommandLine::Get(), TEXT("slforcetagging"));
	return bStreamlineForceTagging || IsDLSSGActive();
}

static FIntRect GetViewportRect(SWindow& InWindow)
{
	FIntRect ViewportRect = FIntRect(0,0,0,0);
	if (TSharedPtr<ISlateViewport> Viewport = InWindow.GetViewport())
	{
		if (TSharedPtr<SWidget> Widget = Viewport->GetWidget().Pin())
		{

			FGeometry Geom = Widget->GetPaintSpaceGeometry();

			FIntPoint Min = { int32(Geom.GetAbsolutePosition().X),int32(Geom.GetAbsolutePosition().Y) };
			FIntPoint Max = { int32((Geom.GetAbsolutePosition() + Geom.GetAbsoluteSize()).X),
								int32((Geom.GetAbsolutePosition() + Geom.GetAbsoluteSize()).Y) };

			ViewportRect = FIntRect(Min.X, Min.Y, Max.X, Max.Y);
		}
	}

	return ViewportRect;
}

static void DLSSGOnBackBufferReadyToPresent(SWindow& InWindow, const FTexture2DRHIRef& InBackBuffer)
{
	check(IsInRenderingThread());

	const bool bIsGameWindow = InWindow.GetType() == EWindowType::GameWindow;
#if WITH_EDITOR
	const bool bIsPIEWindow = GIsEditor && (InWindow.GetTitle().ToString().Contains(TEXT("Preview [NetMode:")));
#else
	const bool bIsPIEWindow = false;
#endif
	if (!(bIsGameWindow || bIsPIEWindow))
	{
		return;
	}

	if (!ShouldTagStreamlineBuffers())
	{
		return;
	}

	const bool bTagUIColorAlpha = GIsEditor ? CVarStreamlineEditorTagUIColorAlpha.GetValueOnRenderThread() : CVarStreamlineTagUIColorAlpha.GetValueOnRenderThread() ;

	// TODO maybe add a helper function to add the RDG pass to tag a resource and use that everywhere
	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();
	FRDGBuilder GraphBuilder(RHICmdList);

	FSLUIHintTagShaderParameters* PassParameters = GraphBuilder.AllocParameters<FSLUIHintTagShaderParameters>();
	FStreamlineRHI* RHIExtensions = FStreamlineCoreModule::GetStreamlineRHI();
	const uint32 ViewId = 0;

	if (bTagUIColorAlpha)
	{
		FIntPoint BackBufferDimension = { int32(InBackBuffer->GetTexture2D()->GetSizeX()), int32(InBackBuffer->GetTexture2D()->GetSizeY()) };
		
		const FIntRect ViewRect = GetViewportRect(InWindow);

		check(!ViewRect.IsEmpty());
		check(ViewRect.Width() <= BackBufferDimension.X);
		check(ViewRect.Height() <= BackBufferDimension.Y);
		check(ViewRect.Min.X >= 0);
		check(ViewRect.Min.Y >= 0);
		
		FRDGTextureRef UIHintTexture = AddStreamlineUIHintExtractionPass(GraphBuilder, InBackBuffer);
		PassParameters->UIColorAndAlpha = UIHintTexture;

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("Streamline UIColorAndAlpha Tag %dx%d [%d,%d -> %d,%d] Texture=%s",
				ViewRect.Width(), ViewRect.Height(),
				ViewRect.Min.X, ViewRect.Min.Y,
				ViewRect.Max.X, ViewRect.Max.Y
				,  *BackBufferDimension.ToString()),
			PassParameters,
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION == 25)
			ERDGPassFlags::Compute,
#else
			ERDGPassFlags::Compute | ERDGPassFlags::Raster | ERDGPassFlags::SkipRenderPass | ERDGPassFlags::NeverCull,
#endif
			[RHIExtensions, PassParameters, ViewRect, ViewId](FRHICommandListImmediate& RHICmdList) mutable
			{
				check(PassParameters->UIColorAndAlpha);
				PassParameters->UIColorAndAlpha->MarkResourceAsUsed();
				FRHITextureWithRect UIColorAndAlpha = { PassParameters->UIColorAndAlpha->GetRHI(), ViewRect };

				RHICmdList.EnqueueLambda(
					[RHIExtensions, UIColorAndAlpha, ViewId](FRHICommandListImmediate& Cmd) mutable
					{
						RHIExtensions->TagTexture(Cmd, UIColorAndAlpha, EStreamlineResource::UIColorAndAlpha, ViewId);
					});
			});
	}
	else
	{
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("Streamline UIColorAndAlpha Tag (Null)"),
			PassParameters,
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION == 25)
			ERDGPassFlags::Compute,
#else
			ERDGPassFlags::Compute | ERDGPassFlags::Raster | ERDGPassFlags::SkipRenderPass | ERDGPassFlags::NeverCull,
#endif
			[RHIExtensions, ViewId](FRHICommandListImmediate& RHICmdList) mutable
			{
				RHICmdList.EnqueueLambda(
					[RHIExtensions, ViewId](FRHICommandListImmediate& Cmd) mutable
					{
						RHIExtensions->TagNullTexture(EStreamlineResource::UIColorAndAlpha, ViewId);
					});
			});
	}


	GraphBuilder.Execute();
}

void RegisterStreamlineDLSSGHooks(FStreamlineRHI* InStreamlineRHI)
{
	UE_LOG(LogStreamline, Log, TEXT("%s Enter"), ANSI_TO_TCHAR(__FUNCTION__));

	check(IsStreamlineDLSSGSupported());

	{
		check(FSlateApplication::IsInitialized());
		FSlateRenderer* SlateRenderer = FSlateApplication::Get().GetRenderer();

		OnBackBufferReadyToPresentHandle = SlateRenderer->OnBackBufferReadyToPresent().AddStatic(&DLSSGOnBackBufferReadyToPresent);

		// ShutdownModule is too late for this
		FSlateApplication::Get().OnPreShutdown().AddLambda(
		[]()
		{
			UE_LOG(LogStreamline, Log, TEXT("Unregistering of OnPreRHIViewportCreate, OnPostRHIViewportCreate etc callbacks during FSlateApplication::OnPreShutdown"));
			FSlateRenderer* SlateRenderer = FSlateApplication::Get().GetRenderer();
			check(SlateRenderer);

			SlateRenderer->OnBackBufferReadyToPresent().Remove(OnBackBufferReadyToPresentHandle);
		}
		);

		
	}
	UE_LOG(LogStreamline, Log, TEXT("%s Leave"), ANSI_TO_TCHAR(__FUNCTION__));
}

void UnregisterStreamlineDLSSGHooks()
{
	// see  FSlateApplication::OnPreShutdown lambda in RegisterStreamlineDLSSGHooks
}

static bool IsStreamlineDLSSGSupportedInternal()
{
	static bool bStreamlineDLSSGSupportedInitialized = false;
	static bool bStreamlineDLSSGSupported = false;

	if (!bStreamlineDLSSGSupportedInitialized)
	{
		FStreamlineRHI* StreamlineRHI = GetPlatformStreamlineRHI();
		if (StreamlineRHI->IsDLSSGSupportedByRHI())
		{
			sl::Result Result = SLisFeatureSupported(sl::kFeatureDLSS_G, *StreamlineRHI->GetAdapterInfo());
			bStreamlineDLSSGSupported = (Result == sl::Result::eOk);
		}
		else
		{
			bStreamlineDLSSGSupported = false;
		}
		bStreamlineDLSSGSupportedInitialized = true;
	}
	return bStreamlineDLSSGSupported;
}

bool IsStreamlineDLSSGSupported()
{
	if (!FApp::CanEverRender())
	{
		return false;
	}

	if (!IsRHIDeviceNVIDIA())
	{
		return false;
	}

	if (!IsStreamlineSupported())
	{
		return false;
	}

	return IsStreamlineDLSSGSupportedInternal();
}

bool IsDLSSGActive()
{
	if (!IsStreamlineDLSSGSupportedInternal())
	{
		return false;
	}
	else
	{
		return CVarStreamlineDLSSGEnable.GetValueOnAnyThread() != 0 ? true : false;
	}
}

static int32 GetDLSSGNumFrames()
{
	// currently Streamline only supports 1
	return 1;
}

DECLARE_STATS_GROUP(TEXT("DLSS-G"), STATGROUP_DLSSG, STATCAT_Advanced);
DECLARE_DWORD_COUNTER_STAT(TEXT("DLSS-G: Frames Presented"), STAT_DLSSGFramesPresented, STATGROUP_DLSSG);
DECLARE_FLOAT_COUNTER_STAT(TEXT("DLSS-G: Average FPS"), STAT_DLSSGAverageFPS, STATGROUP_DLSSG);

namespace
{
	float GLastDLSSGFrameRate = 0.0f;
	int32 GLastDLSSGFramesPresented = 0;
}

void UpdateDLSSGFrameTime()
{
	extern ENGINE_API float GAverageFPS;

	GLastDLSSGFrameRate = GAverageFPS;
	GLastDLSSGFramesPresented = 1;

	if (IsStreamlineDLSSGSupported())
	{
		// INSERT AWKWARD MUPPET FACE HERE

		static const auto CVarStreamlineViewIdOverride = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streamline.ViewIdOverride"));
		checkf(CVarStreamlineViewIdOverride && CVarStreamlineViewIdOverride->GetInt() != 0, TEXT("r.Streamline.ViewIdOverride must be set to 1 since DLSS-G only supports a single viewport."));

		sl::ViewportHandle Viewport(0);

		sl::DLSSGState State;

		sl::DLSSGOptions StreamlineConstantsDLSSG;

		StreamlineConstantsDLSSG.mode = CVarStreamlineDLSSGEnable.GetValueOnAnyThread() != 0 ? sl::DLSSGMode::eOn : sl::DLSSGMode::eOff;
		StreamlineConstantsDLSSG.numFramesToGenerate = GetDLSSGNumFrames();

		CALL_SL_FEATURE_FN(sl::kFeatureDLSS_G, slDLSSGGetState, Viewport, State, &StreamlineConstantsDLSSG);

		GLastDLSSGFramesPresented = State.numFramesActuallyPresented;
		SET_DWORD_STAT(STAT_DLSSGFramesPresented, GLastDLSSGFramesPresented);

		GLastDLSSGFrameRate = GAverageFPS * GLastDLSSGFramesPresented;
		SET_FLOAT_STAT(STAT_DLSSGAverageFPS, GLastDLSSGFrameRate);
	}

}
STREAMLINECORE_API void GetStreamlineDLSSGFrameTiming(float& FrameRateInHertz, int32& FramesPresented)
{
	FrameRateInHertz = GLastDLSSGFrameRate;
	FramesPresented = GLastDLSSGFramesPresented;
}


void SetStreamlineDLSSGState(FRHICommandListImmediate& RHICmdList, uint32 ViewID)
{
	if (IsStreamlineDLSSGSupported())
	{
		RHICmdList.EnqueueLambda(
			[ViewID](FRHICommandListImmediate& Cmd) mutable
			{
				sl::DLSSGOptions StreamlineConstantsDLSSG;
				StreamlineConstantsDLSSG.mode = CVarStreamlineDLSSGEnable.GetValueOnAnyThread() != 0 ? sl::DLSSGMode::eOn : sl::DLSSGMode::eOff;
				StreamlineConstantsDLSSG.numFramesToGenerate = GetDLSSGNumFrames();
				CALL_SL_FEATURE_FN(sl::kFeatureDLSS_G, slDLSSGSetOptions, sl::ViewportHandle(ViewID), StreamlineConstantsDLSSG);
			});
	}
}

namespace

{

	BEGIN_SHADER_PARAMETER_STRUCT(FSLDLSSGShaderParameters, )
		// Fake output to trigger pass execution
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION == 25)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneColorAfterTonemap)
#else
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, RenderPassTriggerDummy)
#endif
		END_SHADER_PARAMETER_STRUCT()

}

void AddStreamlineDLSSGStateRenderPass(FRDGBuilder& GraphBuilder, uint32 ViewID)
{

	FSLDLSSGShaderParameters* PassParameters = GraphBuilder.AllocParameters<FSLDLSSGShaderParameters>();

#if (ENGINE_MAJOR_VERSION > 4) || (ENGINE_MINOR_VERSION > 26)
	PassParameters->RenderPassTriggerDummy = GSystemTextures.GetBlackDummy(GraphBuilder);
#else
	PassParameters->RenderPassTriggerDummy = GraphBuilder.RegisterExternalTexture(GSystemTextures.BlackDummy);
#endif

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("Streamline DLSS-G ViewID=%u", ViewID),
		PassParameters,
#if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION == 25)
		ERDGPassFlags::Compute,
#else
		ERDGPassFlags::Compute | ERDGPassFlags::Raster | ERDGPassFlags::SkipRenderPass | ERDGPassFlags::NeverCull,
#endif
		[PassParameters, ViewID](FRHICommandListImmediate& RHICmdList) mutable
		{
			// output dummy to trigger RDG pass exeution
			check(PassParameters->RenderPassTriggerDummy);
			PassParameters->RenderPassTriggerDummy->MarkResourceAsUsed();


			RHICmdList.EnqueueLambda(
				[ViewID](FRHICommandListImmediate& Cmd) mutable
				{
					SetStreamlineDLSSGState(Cmd, ViewID);
				});
		});
}

void BeginRenderViewFamilyDLSSG(FSceneViewFamily& InViewFamily)
{
	if(IsDLSSGActive() && CVarStreamlineDLSSGAdjustMotionBlurTimeScale.GetValueOnAnyThread() && InViewFamily.Views.Num())
	{
		for (int32 ViewIndex = 0; ViewIndex < InViewFamily.Views.Num(); ++ViewIndex)
		{
			if (FSceneViewStateInterface* ViewStateInterface = InViewFamily.Views[ViewIndex]->State)
			{
				// The things we do to avoid engine changes ...
				FSceneViewState* ViewState = static_cast<FSceneViewState*>(ViewStateInterface);

				const float TimeScaleCorrection = 1.0f / (1.0f + GetDLSSGNumFrames());

				float& MotionBlurTimeScale = ViewState->MotionBlurTimeScale;
				float& MotionBlurTargetDeltaTime = ViewState->MotionBlurTargetDeltaTime;

				MotionBlurTimeScale *= TimeScaleCorrection;
				MotionBlurTargetDeltaTime *= TimeScaleCorrection;
			}
		}
	}
}
