#include <Windows.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <thread>

#include "Generators/CppGenerator.h"
#include "Generators/MappingGenerator.h"
#include "Generators/IDAMappingGenerator.h"
#include "Generators/DumpspaceGenerator.h"

#include "Generators/Generator.h"

enum class EFortToastType : uint8
{
        Default                        = 0,
        Subdued                        = 1,
        Impactful                      = 2,
        EFortToastType_MAX             = 3,
};

DWORD MainThread(HMODULE Module)
{
	OutputDebugStringA("[Dumper-7] MainThread started.\n");

	try
	{
		// Moved console allocation earlier to see if it fails
		if (AllocConsole())
		{
			OutputDebugStringA("[Dumper-7] AllocConsole successful.\n");
			FILE* Dummy;
			// Redirect stdout, stdin, stderr to the new console
			freopen_s(&Dummy, "CONOUT$", "w", stdout);
			freopen_s(&Dummy, "CONIN$", "r", stdin);
			freopen_s(&Dummy, "CONOUT$", "w", stderr);

			// Set console title
			SetConsoleTitleA("Dumper-7");
			std::cout << "[Dumper-7] Console Allocated!" << std::endl;
		}
		else
		{
			DWORD error = GetLastError();
			char buffer[256];
			sprintf_s(buffer, "[Dumper-7] AllocConsole failed. Error code: %lu\n", error);
			OutputDebugStringA(buffer);
			// Don't exit immediately, try to continue if possible, or log more info
		}

		OutputDebugStringA("[Dumper-7] Initializing EngineCore...\n");
		Generator::InitEngineCore();
		OutputDebugStringA("[Dumper-7] Initializing Internal...\n");
		Generator::InitInternal();

		if (Settings::Generator::GameName.empty() && Settings::Generator::GameVersion.empty())
		{
			// Only Possible in Main()
			FString Name;
			FString Version;
			UEClass Kismet = ObjectArray::FindClassFast("KismetSystemLibrary");
			UEFunction GetGameName = Kismet.GetFunction("KismetSystemLibrary", "GetGameName");
			UEFunction GetEngineVersion = Kismet.GetFunction("KismetSystemLibrary", "GetEngineVersion");

			Kismet.ProcessEvent(GetGameName, &Name);
			Kismet.ProcessEvent(GetEngineVersion, &Version);

			Settings::Generator::GameName = Name.ToString();
			Settings::Generator::GameVersion = Version.ToString();
		}

		std::cout << "GameName: " << Settings::Generator::GameName << "\n";
		std::cout << "GameVersion: " << Settings::Generator::GameVersion << "\n\n";

		OutputDebugStringA("[Dumper-7] Starting generation...\n");
		std::cout << "[Dumper-7] Started Generation [Dumper-7]!\n";

		Generator::Generate<CppGenerator>();
		Generator::Generate<MappingGenerator>();
		Generator::Generate<IDAMappingGenerator>();
		Generator::Generate<DumpspaceGenerator>();

		std::cout << "[Dumper-7] Finished Generation!" << std::endl;
		OutputDebugStringA("[Dumper-7] Finished generation.\n");

		auto t_C = std::chrono::high_resolution_clock::now();

		auto ms_int_ = std::chrono::duration_cast<std::chrono::milliseconds>(t_C - std::chrono::high_resolution_clock::now());
		std::chrono::duration<double, std::milli> ms_double_ = t_C - std::chrono::high_resolution_clock::now();

		std::cout << "\n\nGenerating SDK took (" << ms_double_.count() << "ms)\n\n\n";
	}
	catch (const std::exception& e)
	{
		char buffer[512];
		sprintf_s(buffer, "[Dumper-7] EXCEPTION: %s\n", e.what());
		OutputDebugStringA(buffer);
		std::cout << "[Dumper-7] EXCEPTION: " << e.what() << std::endl;
	}
	catch (...)
	{
		OutputDebugStringA("[Dumper-7] UNKNOWN EXCEPTION occurred.\n");
		std::cout << "[Dumper-7] UNKNOWN EXCEPTION occurred." << std::endl;
	}

	std::cout << "[Dumper-7] Press F6 to exit!" << std::endl;
	OutputDebugStringA("[Dumper-7] Entering wait loop (F6 to exit).\n");

	while (true)
	{
		if (GetAsyncKeyState(VK_F6) & 1)
			break;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	OutputDebugStringA("[Dumper-7] F6 pressed. Unloading...\n");
	// Cleanup console and free the library
	fclose(stdout);
	fclose(stdin);
	fclose(stderr);
	FreeConsole();
	FreeLibraryAndExitThread(Module, 0);

	return 0; // Should not be reached
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		OutputDebugStringA("[Dumper-7] DLL_PROCESS_ATTACH\n");
		DisableThreadLibraryCalls(hModule);
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));
		break;
	case DLL_PROCESS_DETACH:
		OutputDebugStringA("[Dumper-7] DLL_PROCESS_DETACH\n");
		break;
	}

	return TRUE;
}