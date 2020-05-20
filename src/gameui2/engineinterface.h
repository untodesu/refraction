#ifndef ENGINEINTERFACE_H
#define ENGINEINTERFACE_H
#ifdef _WIN32
#pragma once
#endif

#include "cdll_int.h"
#include "engine/IEngineSound.h"
#include "ienginevgui.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "ivrenderview.h"
#include "materialsystem/imaterialsystem.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "filesystem.h"

extern IVEngineClient *             g_pEngine;
extern IEngineSound *               g_pEngineSound;
extern IEngineVGui *                g_pEngineVGui;
extern ISoundEmitterSystemBase *    g_pSoundEmitterSystem;
extern IVRenderView *               g_pRenderView;
extern IMaterialSystem *            g_pMaterialSystem;
extern IMatSystemSurface *          g_pMatSystemSurface;
extern IFileSystem *                g_pFileSystem;

#endif
