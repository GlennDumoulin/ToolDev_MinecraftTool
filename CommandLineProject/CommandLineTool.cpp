#include <direct.h> // _getwcwd
#include <windows.h>
#include <sstream>

#include "CommonCode.h"

#include <map>
#include <algorithm>

enum class OutputLocationStatus
{
	UNDEFINED,
	CMD,
	INPUT,
};

bool IsValidFileArg(const wchar_t* arg, const wchar_t* extension);

void PrintUsageMsg();
void PrintArgsMsg();
void PrintErrorMsg(const std::wstring& customError = L"");

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	if (argc <= 2)
	{
		const std::wstring helpArg{ L"help" };
		const std::wstring argsArg{ L"args" };

		if (argc == 2)
		{
			if (helpArg.compare(argv[1]) == 0) //Print help message
			{
				PrintUsageMsg();
				return 0;
			}
			else if (argsArg.compare(argv[1]) == 0) //Print arguments message
			{
				PrintArgsMsg();
				return 0;
			}
			
			//Print error message
			PrintErrorMsg();
			return -1;
		}

		//Print help message
		PrintUsageMsg();
		return 0;
	}
	else if (argc % 2 == 1)
	{
		//Check arguments
		const std::wstring inputArg{ L"-i" };
		const std::wstring outputArg{ L"-o" };
		const std::wstring locationArg{ L"-l" };
		const std::wstring reportArg{ L"-r" };

		std::wstring inputFilename{ L"" };
		std::wstring outputFilename{ L"" };
		
		OutputLocationStatus locationStatus{ OutputLocationStatus::UNDEFINED };
		commonCode::ReportStatus reportStatus{ commonCode::ReportStatus::UNDEFINED };

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
						PrintErrorMsg(L"Input has to be .json and filename must contain at least 1 character!");
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
						PrintErrorMsg(L"Output has to be .obj and filename must contain at least 1 character!");
						return -1;
					}
				}
				else
				{
					PrintErrorMsg(L"Multiple outputs were given!");
					return -1;
				}
			}
			else if (locationArg.compare(argv[i]) == 0) //Check location args
			{
				if (locationStatus == OutputLocationStatus::UNDEFINED)
				{
					//Check argument value
					const std::wstring cmdValue{ L"cmd" };
					const std::wstring inputValue{ L"input" };

					if (cmdValue.compare(argv[i + 1]) == 0) //Handle cmd value
					{
						locationStatus = OutputLocationStatus::CMD;
					}
					else if (inputValue.compare(argv[i + 1]) == 0) //Handle input value
					{
						locationStatus = OutputLocationStatus::INPUT;
					}
					else //Handle other values
					{
						PrintErrorMsg(L"Unknown location value!");
						return -1;
					}
				}
				else
				{
					PrintErrorMsg(L"Multiple locations were given!");
					return -1;
				}
			}
			else if (reportArg.compare(argv[i]) == 0) //Check report args
			{
				if (reportStatus == commonCode::ReportStatus::UNDEFINED)
				{
					//Check argument value
					const std::wstring blocksValue{ L"blocks" };
					const std::wstring layersValue{ L"layers" };

					if (blocksValue.compare(argv[i + 1]) == 0) //Handle blocks value
					{
						reportStatus = commonCode::ReportStatus::BLOCKS;
					}
					else if (layersValue.compare(argv[i + 1]) == 0) //Handle layers value
					{
						reportStatus = commonCode::ReportStatus::LAYERS;
					}
					else //Handle other values
					{
						PrintErrorMsg(L"Unknown report value!");
						return -1;
					}
				}
				else
				{
					PrintErrorMsg(L"Multiple reports were given!");
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
			//Correct slashes into backslashes
			std::replace(inputFilename.begin(), inputFilename.end(), '/', '\\');

			if (outputFilename.compare(L"") == 0)
			{
				//Set default output
				outputFilename = inputFilename;

				const std::wstring jsonExtension{ L".json" };
				const size_t extensionIdx{ outputFilename.rfind(jsonExtension.c_str()) };
				outputFilename.replace(extensionIdx, jsonExtension.length(), L".obj");
			}
			else
			{
				//Correct slashes into backslashes
				std::replace(outputFilename.begin(), outputFilename.end(), '/', '\\');
			}

			//Set output location
			switch (locationStatus)
			{
			case OutputLocationStatus::CMD:
			{
				const size_t lastSlashIdx{ outputFilename.rfind(L"\\") };
				if (lastSlashIdx != std::wstring::npos)
					outputFilename.replace(0, lastSlashIdx + 1, L"");
				break;
			}
			case OutputLocationStatus::INPUT:
			{
				const size_t lastInputSlashIdx{ inputFilename.rfind(L"\\") };
				const std::wstring inputLocationStr{ inputFilename.substr(
					0,
					(lastInputSlashIdx != std::wstring::npos) ? lastInputSlashIdx + 1 : 0
				) };

				const size_t lastOutputSlashIdx{ outputFilename.rfind(L"\\") };
				outputFilename.replace(
					0,
					(lastOutputSlashIdx != std::wstring::npos) ? lastOutputSlashIdx + 1 : 0,
					inputLocationStr
				);

				break;
			}

			case OutputLocationStatus::UNDEFINED:
			default:
				break;
			}

			//Handle file conversion
			std::vector<commonCode::Block> blocks{};
			wchar_t* message{ L"" };
			if (commonCode::ConvertJsonToObj(inputFilename, outputFilename, blocks, message) == -1)
			{
				wprintf_s(message);
				return -1;
			}
			else
			{
				wprintf_s(message);
			}

			//Handle reporting
			switch (reportStatus)
			{
			case commonCode::ReportStatus::BLOCKS: //Report blocks
			{
				wprintf_s(L"\nReport:\n");

				int blockIdx{ 0 };
				for (const commonCode::Block& b : blocks)
				{
					wprintf_s(
						L"id: %d\tlayer: %s\topaque: %s\tposition: %.4f, %.4f, %.4f\n",
						blockIdx, b.layerName, b.isOpaque ? L"true" : L"false", b.pos.x, b.pos.y, b.pos.z
					);
					++blockIdx;
				}

				wprintf_s(L"\n");

				break;
			}

			case commonCode::ReportStatus::LAYERS: //Report layers
			{
				std::map<const std::wstring, int> layers{};
				for (const commonCode::Block& b : blocks)
				{
					const std::wstring layername{ b.layerName };

					if (layers.find(layername) != layers.end())
					{
						++layers[layername];
					}
					else
					{
						layers[layername] = 1;
					}
				}

				wprintf_s(L"\nReport:\n");

				int layerIdx{ 0 };
				for (const auto& layerIt : layers)
				{
					wprintf_s(L"id: %d\t layer name: %s\tnr of blocks: %d\n", layerIdx, layerIt.first.c_str(), layerIt.second);
					++layerIdx;
				}
				
				wprintf_s(L"\n");

				break;
			}

			case commonCode::ReportStatus::UNDEFINED: //Report nothing
			default:
				break;
			}

			return 0;
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

	if (extensionIdx != std::wstring::npos && extensionIdx != 0)
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
	wprintf_s(L"\t(arguments): cmdMinecraftTool args\n");
	wprintf_s(L"\t(command structure): cmdMinecraftTool [<arg_identifier> <arg_value>]\n");
	wprintf_s(L"\n");

	wprintf_s(L"\t(required arguments):\n");
	wprintf_s(L"\t\t-i <inputFile>.json\n");
	wprintf_s(L"\t\t\tinputFile --> name of input file, can also include path to different directory\n");

	wprintf_s(L"\t(optional arguments):\n");
	wprintf_s(L"\t\tcmdMinecraftTool args\n");
	wprintf_s(L"\t\t\texecute this command to get an overview of all available arguments\n");
	wprintf_s(L"\n");
}
void PrintArgsMsg()
{
	wprintf_s(L"\t(required arguments):\n");
	wprintf_s(L"\t\t-i <inputFile>.json\n");
	wprintf_s(L"\t\t\tinputFile --> name of input file, can also include path to different directory\n");

	wprintf_s(L"\t(optional arguments):\n");
	wprintf_s(L"\t\t-o <outputFile>.obj\n");
	wprintf_s(L"\t\t\toutputFile --> name of output file, can also include path to different directory\n");
	wprintf_s(L"\t\t\t\tnot defined --> outputFile == inputFile, also copies path to input file directory\n");
	wprintf_s(L"\t\t-l <cmd|input>\n");
	wprintf_s(L"\t\t\tcmd --> output file location == current cmd location\n");
	wprintf_s(L"\t\t\tinput --> output file location == input file location\n");
	wprintf_s(L"\t\t\t\tnot defined --> output file location is unchanged\n");
	wprintf_s(L"\t\t-r <blocks|layers>\n");
	wprintf_s(L"\t\t\tblocks --> report blocks info\n");
	wprintf_s(L"\t\t\tlayers --> report layer info\n");
	wprintf_s(L"\t\t\t\tnot defined --> no report\n");
	wprintf_s(L"\n");

	wprintf_s(L"Examples:\n");
	wprintf_s(L"\tcmdMinecraftTool -i ..\\myInput.json\n");
	wprintf_s(L"\t\tresulting output: ..\\myInput.obj\n");
	wprintf_s(L"\tcmdMinecraftTool -i ..\\myInput.json -o myOutput.obj\n");
	wprintf_s(L"\t\tresulting output: myOutput.obj\n");
	wprintf_s(L"\tcmdMinecraftTool -i ..\\myInput.json -o myOutput.obj -l input\n");
	wprintf_s(L"\t\tresulting output: ..\\myOutput.obj\n");
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
