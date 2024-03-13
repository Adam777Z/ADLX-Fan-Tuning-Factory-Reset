//
// Copyright (c) 2021 - 2023 Advanced Micro Devices, Inc. All rights reserved.
//
//-------------------------------------------------------------------------------------------------

/// \file main.cpp
/// \brief Restore GPU tuning status to factory default with ADLX.

#include "SDK/ADLXHelper/Windows/Cpp/ADLXHelper.h"
#include "SDK/Include/IGPUManualFanTuning.h"
#include "SDK/Include/IGPUTuning.h"
#include <iostream>

// Use ADLX namespace
using namespace adlx;

// ADLXHelper instance
// No outstanding interfaces from ADLX must exist when ADLX is destroyed.
// Use global variables to ensure validity of the interface.
static ADLXHelper g_ADLXHelp;

// Wait for exit with error message
int WaitAndExit(const char* msg, const int retCode);

// Restore GPU tuning status to factory default
void RestoreGPUTuningToFactoryStatus(IADLXGPUTuningServicesPtr gpuTuningService, IADLXGPUListPtr gpus);

int main()
{
	ADLX_RESULT res = ADLX_FAIL;

	// Initialize ADLX
	res = g_ADLXHelp.Initialize();

	if (ADLX_SUCCEEDED(res))
	{
		IADLXGPUTuningServicesPtr gpuTuningService;
		res = g_ADLXHelp.GetSystemServices()->GetGPUTuningServices(&gpuTuningService);
		if (ADLX_FAILED(res))
		{
			// Destroy ADLX
			res = g_ADLXHelp.Terminate();
			std::cout << "Destroy ADLX res: " << res << std::endl;
			return WaitAndExit("\tGet GPU tuning services failed", 0);
		}
		IADLXGPUListPtr gpus;
		res = g_ADLXHelp.GetSystemServices()->GetGPUs(&gpus);
		if (ADLX_FAILED(res))
		{
			// Destroy ADLX
			res = g_ADLXHelp.Terminate();
			std::cout << "Destroy ADLX res: " << res << std::endl;
			return WaitAndExit("\tGet GPU list failed", 0);
		}
		IADLXGPUPtr oneGPU;
		res = gpus->At(0, &oneGPU);
		if (ADLX_FAILED(res) || oneGPU == nullptr)
		{
			// Destroy ADLX
			res = g_ADLXHelp.Terminate();
			std::cout << "Destroy ADLX res: " << res << std::endl;
			return WaitAndExit("\tGet GPU failed", 0);
		}
		adlx_bool supported = false;
		res = gpuTuningService->IsSupportedManualFanTuning(oneGPU, &supported);
		if (ADLX_FAILED(res) || supported == false)
		{
			// Destroy ADLX
			res = g_ADLXHelp.Terminate();
			std::cout << "Destroy ADLX res: " << res << std::endl;
			return WaitAndExit("\tThis GPU doesn't supported manual fan tuning", 0);
		}
		IADLXInterfacePtr manualFanTuningIfc;
		res = gpuTuningService->GetManualFanTuning(oneGPU, &manualFanTuningIfc);
		if (ADLX_FAILED(res) || manualFanTuningIfc == nullptr)
		{
			// Destroy ADLX
			res = g_ADLXHelp.Terminate();
			std::cout << "Destroy ADLX res: " << res << std::endl;
			return WaitAndExit("\tGet manual fan tuning interface failed", 0);
		}

		IADLXManualFanTuningPtr manualFanTuning(manualFanTuningIfc);
		if (manualFanTuning == nullptr)
		{
			// Destroy ADLX
			res = g_ADLXHelp.Terminate();
			std::cout << "Destroy ADLX res: " << res << std::endl;
			return WaitAndExit("\tGet manual fan tuning failed", 0);
		}

		// Restore GPU tuning status to factory default
		RestoreGPUTuningToFactoryStatus(gpuTuningService, gpus);
	}
	else
	{
		return WaitAndExit("\tg_ADLXHelp initialize failed", 0);
	}

	// Destroy ADLX
	res = g_ADLXHelp.Terminate();
	//std::cout << "Destroy ADLX res: " << res << std::endl;

	// Pause to see the print out
	//system("pause");

	return 0;
}

// Wait for exit with error message
int WaitAndExit(const char* msg, const int retCode)
{
	// Printout the message and pause to see it before returning the desired code
	if (nullptr != msg)
	{
		std::cout << msg << std::endl;
	}

	system("pause");
	return retCode;
}

// Reset the current GPU tuning status to factory default
void RestoreGPUTuningToFactoryStatus(IADLXGPUTuningServicesPtr gpuTuningService, IADLXGPUListPtr gpus)
{
	IADLXGPUPtr oneGPU;
	for (adlx_uint crt = gpus->Begin(); crt != gpus->End(); ++crt)
	{
		ADLX_RESULT res = gpus->At(crt, &oneGPU);
		if (ADLX_FAILED(res) || oneGPU == nullptr)
		{
			std::cout << "\tGet the GPU at location" << crt << " failed" << std::endl;
			break;
		}

		// Check if current GPU tuning status is set to factory default
		adlx_bool isFactory = false;
		res = gpuTuningService->IsAtFactory(oneGPU, &isFactory);

		// Only set if current is different
		if (!isFactory)
		{
			res = gpuTuningService->ResetToFactory(oneGPU);
			if (ADLX_FAILED(res))
			{
				std::cout << "\tReset the GPU at location" << crt << " to default failed" << std::endl;
			}
			//else
			//{
			//	std::cout << "\tReset the GPU at location" << crt << " to default successful" << std::endl;
			//}
		}
	}
}