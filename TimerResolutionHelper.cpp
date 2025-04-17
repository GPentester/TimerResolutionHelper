#include "Timer.h"

int main()
{
	if (!IsRunningAsAdmin())
	{
		MessageBoxA(0, "¡Este programa debe ejecutarse como Administrador!", "Permiso requerido", MB_OK | MB_ICONERROR);
		return 1;
	}


	if (!CheckSystemTweaks())
	{
		int choice = MessageBoxA(0,
			"Tu sistema no tiene los ajustes correctos de latencia (bcdedit).\n¿Deseas que los configure automáticamente?\n\nSe requiere reiniciar.",
			"Ajustes incorrectos", MB_YESNO | MB_ICONWARNING);

		if (choice == IDYES)
		{
			FixSystemTweaks();
			MessageBoxA(0, "Ajustes aplicados correctamente. Por favor reinicia el sistema y vuelve a ejecutar este programa.", "Reinicio requerido", MB_OK | MB_ICONINFORMATION);
		}
		return 1;
	}

	int opcion;
	std::cout << "Ya tienes la clave de registro GlobalTimerResolutionRequests?" << std::endl;
	std::cout << "1. Si" << std::endl;
	std::cout << "2. No" << std::endl;
	std::cout << "-> ";
	std::cin >> opcion;

	if (opcion == 1)
	{
		std::cout << "Puede continuar con la ejecucion..." << std::endl;
	}
	if (opcion == 2)
	{
		SetGlobalTimerResolution();
	}
	

	Dependencies();
	const std::string tProc = "prime95.exe";
	const std::string scriptURL = "https://raw.githubusercontent.com/valleyofdoom/TimerResolution/refs/heads/main/micro-adjust-benchmark.ps1";
	std::cout << "Esperando a que Prime95.exe se este ejecutando en primer plano con estres..." << std::endl;
	while (true) 
	{
		if (IsProcessRunning(tProc))
		{
			RunPowerShellCommand(scriptURL);
			DisableIdleState();
			break;
		}
		Sleep(2000);
	}
	return 0;
}