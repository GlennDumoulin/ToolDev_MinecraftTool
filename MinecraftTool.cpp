#include <direct.h> // _getwcwd
#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>
#include <sstream>
#include <vector>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/stream.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"

bool AreEqual(const float a, const float b, const float epsilon = FLT_EPSILON);

struct Vector3f
{
	float x;
	float y;
	float z;

	Vector3f(float x, float y, float z)
		: x{ x }
		, y{ y }
		, z{ z }
	{
	}
	Vector3f(int x, int y, int z)
		: Vector3f(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z))
	{
	}
	
	bool IsEqual(const Vector3f& other) const
	{
		return (
			AreEqual(x, other.x)
			&& AreEqual(y, other.y)
			&& AreEqual(z, other.z)
		);
	}
};

struct Block
{
	const char* layerName;
	bool isOpaque;
	Vector3f pos;
};

enum class OpaqueNeighbourPos
{
	FRONT,
	BACK,
	LEFT,
	RIGHT,
	TOP,
	BOTTOM,
};

void WriteVertices(FILE* pOFile, const Vector3f& blockPos);
void WriteFaces(FILE* pOFile, const std::vector<Block>& blocks);
bool IsValidFileArg(const wchar_t* arg, const wchar_t* extension);
std::vector<OpaqueNeighbourPos> CheckOpaqueNeighbours(const Block& blockToCheck, const std::vector<Block>& blocks);

void PrintUsageMsg();
void PrintErrorMsg(const std::wstring& customError = L"");

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	const std::wstring cmdName{ L"minecrafttool" };
	const std::wstring helpArg{ L"help" };

	if (argc == 1)
	{
		if (cmdName.compare(argv[0]) == 0)
		{
			PrintUsageMsg();
			return 0;
		}
	}
	else if (argc == 2)
	{
		if (cmdName.compare(argv[0]) == 0 && helpArg.compare(argv[1]) == 0)
		{
			PrintUsageMsg();
			return 0;
		}
	}
	else if (argc == 3 || argc == 5)
	{
		//Check arguments
		if (cmdName.compare(argv[0]) == 0)
		{
			const std::wstring inputArg{ L"-i" };
			const std::wstring outputArg{ L"-o" };

			std::wstring inputFilename{ L"" };
			std::wstring outputFilename{ L"" };

			bool areArgsValid{ true };

			for (int i{ 1 }; i < argc; i += 2)
			{
				if (inputArg.compare(argv[i]) == 0) //Check input args
				{
					if (inputFilename.compare(L"") == 0)
					{
						if (IsValidFileArg(argv[i + 1], L"json"))
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
						if (IsValidFileArg(argv[i + 1], L"obj"))
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

				if (std::ifstream is{ inputFilename })
				{
					using namespace rapidjson;

					IStreamWrapper isw{ is };
					Document sceneDoc;
					sceneDoc.ParseStream(isw);

					if (sceneDoc.IsArray())
					{
						FILE* pOFile = nullptr;
						_wfopen_s(&pOFile, outputFilename.c_str(), L"w+,ccs=UTF-8");
						if (pOFile != nullptr) //File was succesfully created
						{
							//Initialize file with comment
							const wchar_t* text = L"#Minecraft Scene\n\n";
							fwrite(text, wcslen(text) * sizeof(wchar_t), 1, pOFile);

							//Declare materials
							fwprintf_s(pOFile, L"mtllib Resources/minecraftMats.mtl\n\n");

							//Add normals
							fwprintf_s(pOFile, L"vn %.4f %.4f %.4f\n", 0.f, 0.f, 1.f);
							fwprintf_s(pOFile, L"vn %.4f %.4f %.4f\n", 0.f, 0.f, -1.f);
							fwprintf_s(pOFile, L"vn %.4f %.4f %.4f\n", 0.f, 1.f, 0.f);
							fwprintf_s(pOFile, L"vn %.4f %.4f %.4f\n", 0.f, -1.f, 0.f);
							fwprintf_s(pOFile, L"vn %.4f %.4f %.4f\n", 1.f, 0.f, 0.f);
							fwprintf_s(pOFile, L"vn %.4f %.4f %.4f\n", -1.f, 0.f, 0.f);

							//Add texture coordinates
							fwprintf_s(pOFile, L"vt %.4f %.4f\n", 0.f, 0.f);
							fwprintf_s(pOFile, L"vt %.4f %.4f\n", 1.f, 0.f);
							fwprintf_s(pOFile, L"vt %.4f %.4f\n", 0.f, 1.f);
							fwprintf_s(pOFile, L"vt %.4f %.4f\n", 1.f, 1.f);
							fwprintf_s(pOFile, L"\n");

							//Write blocks
							std::vector<Block> blocks{};

							for (Value::ConstValueIterator layerIt = sceneDoc.Begin(); layerIt != sceneDoc.End(); ++layerIt)
							{
								const Value& layer{ *layerIt };

								if (layer.HasMember("layer") && layer.HasMember("opaque") && layer.HasMember("positions"))
								{
									const Value& layerName{ layer["layer"] };
									const Value& isOpaque{ layer["opaque"] };
									const Value& positions{ layer["positions"] };

									if (layerName.IsString() && isOpaque.IsBool() && positions.IsArray())
									{
										//Add blocks
										for (Value::ConstValueIterator blockIt = positions.Begin(); blockIt != positions.End(); ++blockIt)
										{
											const Value& pos{ *blockIt };

											if (pos.IsArray() && pos.Size() == 3)
											{
												const Value& x{ pos[1] };
												const Value& y{ pos[2] };
												const Value& z{ pos[0] };

												if (x.IsInt() && y.IsInt() && z.IsInt())
												{
													//Create block
													Block block{ layerName.GetString(), isOpaque.GetBool(), Vector3f{ x.GetInt(), y.GetInt(), z.GetInt() } };
													blocks.push_back(block);

													//Add vertices
													WriteVertices(pOFile, block.pos);
												}
												else
												{
													wprintf_s(L"Failed to parse block!\n");
													continue;
												}
											}
											else
											{
												wprintf_s(L"Failed to parse block!\n");
												continue;
											}
										}
									}
									else
									{
										wprintf_s(L"Failed to parse layer!\n");
										continue;
									}
								}
								else
								{
									wprintf_s(L"Failed to parse layer!\n");
									continue;
								}
							}

							//Add faces
							WriteFaces(pOFile, blocks);

							fclose(pOFile);
							wprintf_s(L"Output file was succesfully created!\n");
							return 0;
						}
						else {
							wprintf_s(L"Failed to create output file!\n");
							return -1;
						}
					}
				}
				else
				{
					wprintf_s(L"Couldn't find input file!\n");
					return -1;
				}
			}
			else
			{
				PrintErrorMsg(L"Input file is missing!");
				return -1;
			}
		}
	}

	PrintErrorMsg();
	return -1;
}

bool AreEqual(const float a, const float b, const float epsilon)
{
	return fabs(a - b) < epsilon;
}

void WriteVertices(FILE* pOFile, const Vector3f& blockPos)
{
	float xPos{ blockPos.x };
	float yPos{ blockPos.y };
	float zPos{ blockPos.z };

	//Vertices
	fwprintf_s(pOFile, L"v %.4f %.4f %.4f\n", xPos, yPos, zPos);
	fwprintf_s(pOFile, L"v %.4f %.4f %.4f\n", xPos, yPos, zPos + 1.f);
	fwprintf_s(pOFile, L"v %.4f %.4f %.4f\n", xPos, yPos + 1.f, zPos);
	fwprintf_s(pOFile, L"v %.4f %.4f %.4f\n", xPos, yPos + 1.f, zPos + 1.f);
	fwprintf_s(pOFile, L"v %.4f %.4f %.4f\n", xPos + 1.f, yPos, zPos);
	fwprintf_s(pOFile, L"v %.4f %.4f %.4f\n", xPos + 1.f, yPos, zPos + 1.f);
	fwprintf_s(pOFile, L"v %.4f %.4f %.4f\n", xPos + 1.f, yPos + 1.f, zPos);
	fwprintf_s(pOFile, L"v %.4f %.4f %.4f\n", xPos + 1.f, yPos + 1.f, zPos + 1.f);
}
void WriteFaces(FILE* pOFile, const std::vector<Block>& blocks)
{
	std::vector<OpaqueNeighbourPos> opaqueNeighbours{};
	std::string currentLayer{};

	for (int i{ 0 }; i < static_cast<int>(blocks.size()); ++i)
	{
		int idxOffset{ i * 8 };
		const Block currentBlock{ blocks[i] };

		//Check layer
		if (currentLayer.compare(currentBlock.layerName) != 0)
		{
			currentLayer = currentBlock.layerName;

			//Set material
			const size_t layerNameStrLen{ static_cast<size_t>(currentLayer.length()) };
			wchar_t* layerNameWStr = new wchar_t[layerNameStrLen + 1];
			mbstowcs_s(NULL, layerNameWStr, layerNameStrLen + 1, currentLayer.c_str(), layerNameStrLen);
			layerNameWStr[0] -= 32; //Capitalize first character

			fwprintf_s(pOFile, L"\n");
			fwprintf_s(pOFile, L"usemtl %s\n", layerNameWStr);
		}

		//Get opaque neighbours
		opaqueNeighbours = CheckOpaqueNeighbours(currentBlock, blocks);

		//Faces
		//Left
		if (std::find(opaqueNeighbours.begin(), opaqueNeighbours.end(), OpaqueNeighbourPos::LEFT) == opaqueNeighbours.end())
		{
			fwprintf_s(pOFile, L"f %d/%d/%d %d/%d/%d %d/%d/%d\n", idxOffset + 1, 3, 2, idxOffset + 7, 2, 2, idxOffset + 5, 4, 2);
			fwprintf_s(pOFile, L"f %d/%d/%d %d/%d/%d %d/%d/%d\n", idxOffset + 1, 3, 2, idxOffset + 3, 1, 2, idxOffset + 7, 2, 2);
		}

		//Front
		if (std::find(opaqueNeighbours.begin(), opaqueNeighbours.end(), OpaqueNeighbourPos::FRONT) == opaqueNeighbours.end())
		{
			fwprintf_s(pOFile, L"f %d/%d/%d %d/%d/%d %d/%d/%d\n", idxOffset + 1, 4, 6, idxOffset + 4, 1, 6, idxOffset + 3, 2, 6);
			fwprintf_s(pOFile, L"f %d/%d/%d %d/%d/%d %d/%d/%d\n", idxOffset + 1, 4, 6, idxOffset + 2, 3, 6, idxOffset + 4, 1, 6);
		}

		//Top
		if (std::find(opaqueNeighbours.begin(), opaqueNeighbours.end(), OpaqueNeighbourPos::TOP) == opaqueNeighbours.end())
		{
			fwprintf_s(pOFile, L"f %d/%d/%d %d/%d/%d %d/%d/%d\n", idxOffset + 3, 3, 3, idxOffset + 8, 2, 3, idxOffset + 7, 4, 3);
			fwprintf_s(pOFile, L"f %d/%d/%d %d/%d/%d %d/%d/%d\n", idxOffset + 3, 3, 3, idxOffset + 4, 1, 3, idxOffset + 8, 2, 3);
		}

		//Back
		if (std::find(opaqueNeighbours.begin(), opaqueNeighbours.end(), OpaqueNeighbourPos::BACK) == opaqueNeighbours.end())
		{
			fwprintf_s(pOFile, L"f %d/%d/%d %d/%d/%d %d/%d/%d\n", idxOffset + 5, 3, 5, idxOffset + 7, 1, 5, idxOffset + 8, 2, 5);
			fwprintf_s(pOFile, L"f %d/%d/%d %d/%d/%d %d/%d/%d\n", idxOffset + 5, 3, 5, idxOffset + 8, 2, 5, idxOffset + 6, 4, 5);
		}

		//Bottom
		if (std::find(opaqueNeighbours.begin(), opaqueNeighbours.end(), OpaqueNeighbourPos::BOTTOM) == opaqueNeighbours.end())
		{
			fwprintf_s(pOFile, L"f %d/%d/%d %d/%d/%d %d/%d/%d\n", idxOffset + 1, 1, 4, idxOffset + 5, 2, 4, idxOffset + 6, 4, 4);
			fwprintf_s(pOFile, L"f %d/%d/%d %d/%d/%d %d/%d/%d\n", idxOffset + 1, 1, 4, idxOffset + 6, 4, 4, idxOffset + 2, 3, 4);
		}

		//Right
		if (std::find(opaqueNeighbours.begin(), opaqueNeighbours.end(), OpaqueNeighbourPos::RIGHT) == opaqueNeighbours.end())
		{
			fwprintf_s(pOFile, L"f %d/%d/%d %d/%d/%d %d/%d/%d\n", idxOffset + 2, 4, 1, idxOffset + 6, 3, 1, idxOffset + 8, 1, 1);
			fwprintf_s(pOFile, L"f %d/%d/%d %d/%d/%d %d/%d/%d\n", idxOffset + 2, 4, 1, idxOffset + 8, 1, 1, idxOffset + 4, 2, 1);
		}
	}
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

std::vector<OpaqueNeighbourPos> CheckOpaqueNeighbours(const Block& blockToCheck, const std::vector<Block>& blocks)
{
	std::vector<OpaqueNeighbourPos> opaqueNeighbours{};

	//Check if the block we are checking is transparant
	if (blockToCheck.isOpaque == false) return opaqueNeighbours;

	const Vector3f posToCheck{ blockToCheck.pos };
	for (const Block& block : blocks)
	{
		//ignore transparant blocks
		if (block.isOpaque == false) continue;

		//Check if the block is neighbouring
		const Vector3f blockPos{ block.pos };
		
		if (blockPos.IsEqual(Vector3f{ posToCheck.x, posToCheck.y, posToCheck.z - 1.f })) opaqueNeighbours.push_back(OpaqueNeighbourPos::LEFT);
		else if (blockPos.IsEqual(Vector3f{ posToCheck.x, posToCheck.y, posToCheck.z + 1.f })) opaqueNeighbours.push_back(OpaqueNeighbourPos::RIGHT);
		else if (blockPos.IsEqual(Vector3f{ posToCheck.x, posToCheck.y - 1.f, posToCheck.z })) opaqueNeighbours.push_back(OpaqueNeighbourPos::BOTTOM);
		else if (blockPos.IsEqual(Vector3f{ posToCheck.x, posToCheck.y + 1.f, posToCheck.z })) opaqueNeighbours.push_back(OpaqueNeighbourPos::TOP);
		else if (blockPos.IsEqual(Vector3f{ posToCheck.x - 1.f, posToCheck.y, posToCheck.z })) opaqueNeighbours.push_back(OpaqueNeighbourPos::FRONT);
		else if (blockPos.IsEqual(Vector3f{ posToCheck.x + 1.f, posToCheck.y, posToCheck.z })) opaqueNeighbours.push_back(OpaqueNeighbourPos::BACK);
	}

	return opaqueNeighbours;
}

void PrintUsageMsg()
{
	wprintf_s(L"Usage:\n");
	wprintf_s(L"\t(help): minecrafttool\n");
	wprintf_s(L"\t(help): minecrafttool help\n");
	wprintf_s(L"\t(default output): minecrafttool -i <inputFile>.json\n");
	wprintf_s(L"\t(given output): minecrafttool -i <inputFile>.json -o <outputFile>.obj\n");
	wprintf_s(L"\t(given output): minecrafttool -o <outputFile>.obj -i <inputFile>.json\n");
	wprintf_s(L"\n");
}
void PrintErrorMsg(const std::wstring& customError)
{
	wprintf_s(L"Error, incorrect usage!\n");
	if (customError != L"")
	{
		wprintf_s(L"Message: %s\n", customError.c_str());
	}
	PrintUsageMsg();
}
