#include <direct.h> // _getwcwd
#include <windows.h>
#include <sstream>

#include "CommonCode.h"

bool IsValidFileArg(const wchar_t* arg, const wchar_t* extension);

void PrintUsageMsg();
void PrintErrorMsg(const std::wstring& customError = L"");

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	if (argc <= 2)
	{
		const std::wstring helpArg{ L"help" };
		
		if (argc == 2 && helpArg.compare(argv[1]) != 0)
		{
			PrintErrorMsg();
			return -1;
		}

		PrintUsageMsg();
		return 0;
	}
	else if (argc % 2 == 1)
	{
		//Check arguments
		const std::wstring inputArg{ L"-i" };
		const std::wstring outputArg{ L"-o" };

		std::wstring inputFilename{ L"" };
		std::wstring outputFilename{ L"" };

		for (int i{ 1 }; i < argc; i += 2)
		{
			if (inputArg.compare(argv[i]) == 0) //Check input args
			{
				if (inputFilename.compare(L"") == 0)
				{
					if (IsValidFileArg(argv[i + 1], L".json"))
					{
						inputFilename = argv[i + 1];
					}
					else
					{
						PrintErrorMsg(L"Input has to be .json!");
						return -1;
					}
				}
				else
				{
					PrintErrorMsg(L"Multiple inputs were given!");
					return -1;
				}
			}
			else if (outputArg.compare(argv[i]) == 0) //Check output args
			{
				if (outputFilename.compare(L"") == 0)
				{
					if (IsValidFileArg(argv[i + 1], L".obj"))
					{
						outputFilename = argv[i + 1];
					}
					else
					{
						PrintErrorMsg(L"Output has to be .obj!");
						return -1;
					}
				}
				else
				{
					PrintErrorMsg(L"Multiple outputs were given!");
					return -1;
				}
			}
			else
			{
				std::wstringstream errorMsg;
				errorMsg << L"Unknown argument identifier (";
				errorMsg << argv[i];
				errorMsg << L")!";

				PrintErrorMsg(errorMsg.str());
				return -1;
			}
		}

		//Check file names
		if (inputFilename.compare(L"") != 0)
		{
			if (outputFilename.compare(L"") == 0)
			{
				//Set default output
				outputFilename = inputFilename;

				const std::wstring jsonExtension{ L".json" };
				const size_t extensionIdx{ outputFilename.rfind(jsonExtension.c_str()) };
				outputFilename.replace(extensionIdx, jsonExtension.length(), L".obj");
			}

			return commonCode::ConvertJsonToObj(inputFilename, outputFilename);
		}
		else
		{
			PrintErrorMsg(L"Input file is missing!");
			return -1;
		}
	}

	PrintErrorMsg();
	return -1;
}

bool IsValidFileArg(const wchar_t* arg, const wchar_t* extension)
{
	const std::wstring argStr{ arg };
	const size_t extensionIdx{ argStr.rfind(extension) };

	if (extensionIdx != std::wstring::npos)
	{
		const std::wstring extensionStr{ argStr.substr(extensionIdx) };
		return extensionStr.compare(extension) == 0;
	}

	return false;
}

void PrintUsageMsg()
{
	wprintf_s(L"Usage:\n");
	wprintf_s(L"\t(help): cmdMinecraftTool\n");
	wprintf_s(L"\t(help): cmdMinecraftTool help\n");
	wprintf_s(L"\t(command structure): cmdMinecraftTool [<arg_identifier> <arg_value>]\n");
	wprintf_s(L"\t(required arguments):\n");
	wprintf_s(L"\t\t-i <inputFile>.json\n");
	wprintf_s(L"\t(optional arguments):\n");
	wprintf_s(L"\t\t-o <outputFile>.obj\n");
	wprintf_s(L"\n");
	wprintf_s(L"Argument order does not matter!\n");
	wprintf_s(L"\n");
}
void PrintErrorMsg(const std::wstring& customError)
{
	wprintf_s(L"Error, incorrect usage!\n");
	if (customError != L"")
	{
		wprintf_s(L"Message: %s\n", customError.c_str());
	}
	wprintf_s(L"\n");
	PrintUsageMsg();
}
