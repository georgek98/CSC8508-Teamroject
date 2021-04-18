//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2019 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

// ****************************************************************************
// This snippet illustrates simple use of physx
//
// It creates a number of box stacks on a plane, and if rendering, allows the
// user to create new stacks and fire a ball from the camera position
// ****************************************************************************
#pragma once
#include "../../Common/GameTimer.h"
#include <ctype.h>
#include "PxPhysicsAPI.h"
#include "PxPrint.h"
#include "PxPVD.h"
#include "GameObject.h"
#include "GameWorld.h"
#define PX_RELEASE(x)	if(x)	{ x->release(); x = NULL;	}

namespace NCL
{
	namespace CSC8503
	{
		const int GRAVITTY = -9.81 * 4;
		using namespace physx;
		class PxPhysicsSystem
		{
		public:
			PxPhysicsSystem();
			~PxPhysicsSystem();

			void ResetPhysics();

			void StepPhysics(float dt);

			PxPhysics* GetGPhysics()
			{
				return gPhysics;
			}

			PxMaterial* GetGMaterial()
			{
				return gMaterial;
			}

			PxScene* GetGScene()
			{
				return gScene;
			}

		private:
			PxDefaultAllocator		gAllocator;
			PxDefaultErrorCallback	gErrorCallback;

			PxFoundation* gFoundation = NULL;
			PxPhysics* gPhysics = NULL;

			PxDefaultCpuDispatcher* gDispatcher = NULL;
			PxScene* gScene = NULL;

			PxMaterial* gMaterial = NULL;

			PxPvd* gPvd = NULL;

			PxReal stackZ = 10.0f;
		};
	}
}

