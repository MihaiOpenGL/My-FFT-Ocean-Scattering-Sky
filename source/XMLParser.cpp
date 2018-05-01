/* Author: BAIRAC MIHAI */

#include "XMLParser.h"

#include <fstream>

#include "ErrorHandler.h"

#include "glm/gtc/constants.hpp" //pi()

#define ELEMENT_START 0
#define ELEMENT_VALUE 1
#define ELEMENT_END 2


XMLParser::XMLParser ( void )
{}

XMLParser::XMLParser ( const std::string& i_FileName )
{
	Initialize(i_FileName);
}

XMLParser::~XMLParser ( void )
{
	Destroy();
}

void XMLParser::Destroy ( void )
{
	// free resources

	LOG("XMLParser has been destroyed successfully!");
}

void XMLParser::Initialize ( const std::string& i_FileName )
{
	////// Read the content of the file
	std::ifstream fileStream(i_FileName);

	if (!fileStream.good())
	{
		ERR("Failed to open " + i_FileName + " text file name!");
		return;
	}

	// read the whole file in a std::string
	std::string fileContent = std::string((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

	fileStream.close();

	if (fileContent.empty())
	{
		ERR("File is empty!");
		return;
	}

	////// Process the content

	// tokenize the file content into lines
	std::vector<std::string> lines;

	std::string delimiters = "\n";
	Tokenize(fileContent, delimiters, lines);

	// parse each line and build the XML tree
	m_Tree.name = "XMLDoc";
	m_Tree.pParent = nullptr;
	AddNode(m_Tree, 0, lines);

	LOG("XMLParser has been created successfully!");
}

void XMLParser::AddNode ( XMLNode& io_Parent, unsigned short i_LineIndex, const std::vector<std::string>& i_Lines )
{
	if (i_Lines.empty())
	{
		ERR("Invalid content!");
		return;
	}

	if (i_LineIndex >= i_Lines.size())
	{
		ERR("Invalid line index!");
		return;
	}

	if (i_LineIndex + 1 == i_Lines.size()) // stop recursion
	{
		return;
	}

	// tokenize the current line of the file
	std::string delimiters = "<>";
	std::vector<std::string> lineElements;

	Tokenize(i_Lines[i_LineIndex], delimiters, lineElements, true);

	XMLNode newNode;
	newNode.name = lineElements[ELEMENT_START];
	newNode.pParent = &io_Parent;

	if (lineElements.size() == 1) // parent - node with children
	{
		if (lineElements[ELEMENT_START][0] == '/') // end element
		{
			if (io_Parent.name != lineElements[ELEMENT_START].substr(1, lineElements[ELEMENT_START].length()))
			{
				ERR("Parent start and end elements don't match!");
				ERR("Start: " + io_Parent.name + "\nEnd: " + lineElements[ELEMENT_START]);
				return;
			}

			// end element for parent
			if (io_Parent.pParent)
			{
				AddNode(*(io_Parent.pParent), i_LineIndex + 1, i_Lines);
			}
		}
		else // start element
		{
			AddNode(newNode, i_LineIndex + 1, i_Lines);
		}
	}
	else if (lineElements.size() == 3) // child - leaf node
	{
		if (lineElements[ELEMENT_END][0] != '/' || lineElements[ELEMENT_END].size() == 1)
		{
			ERR("Invalid end element!");
			ERR("End: " + lineElements[ELEMENT_END]);
			return;
		}

		if (lineElements[ELEMENT_START] != lineElements[ELEMENT_END].substr(1, lineElements[ELEMENT_END].length()))
		{
			ERR("Start and end elements don't match!");
			ERR("Start: " + lineElements[ELEMENT_START] + "\nEnd: " + lineElements[ELEMENT_END]);
			return;
		}

		newNode.value = lineElements[ELEMENT_VALUE];

		AddNode(io_Parent, i_LineIndex + 1, i_Lines);
	}
	
	if (lineElements.size() != 1 && lineElements.size() != 3)
	{
		ERR("Invalid element! Element: " + i_Lines[i_LineIndex]);
		ERR("Element must be composed of 1 or 3 tokens!");
		return;
	}

	// add node to children if it's not an end element
	if (lineElements[ELEMENT_START][0] != '/')
	{
		io_Parent.children.push_back(newNode);
	}
}

void XMLParser::Tokenize ( const std::string& i_Content, const std::string& i_Delimiters, std::vector<std::string>& o_Elements, bool i_SkipTabElement )
{
	size_t crrIter;
	size_t nextIter = -1;
	std::string element;
	do
	{
		// skip empty elements
		nextIter = i_Content.find_first_not_of(i_Delimiters, nextIter + 1);
		if (nextIter == std::string::npos)
		{
			break;
		}
		nextIter -= 1;

		// get data
		crrIter = nextIter + 1;
		nextIter = i_Content.find_first_of(i_Delimiters, crrIter);
		element = i_Content.substr(crrIter, nextIter - crrIter);

		// skip tab elements
		if (i_SkipTabElement && element[0] == '\t')
		{
			continue;
		}

		// save the element in vector
		o_Elements.push_back(element);
	} while (nextIter != std::string::npos);
}

void XMLParser::PopulateKeyMap ( std::map<std::string, XMLGenericType>& o_KeyMap )
{
	std::string keyPath("");
	PopulateKey(m_Tree.children[0], keyPath, o_KeyMap);
}

void XMLParser::PopulateKey ( const XMLNode& i_Node, const std::string& i_KeyPath, std::map<std::string, XMLGenericType>& o_KeyMap )
{
	std::string keyPath(i_KeyPath + i_Node.name);

	if (i_Node.children.size() > 0) // parent node - has children
	{
		std::vector<XMLNode>::const_iterator it;
		for (it = i_Node.children.begin(); it != i_Node.children.end(); ++it)
		{
			PopulateKey(*it, keyPath + ".", o_KeyMap);
		}
	}
	else // child node
	{
		o_KeyMap[keyPath] = i_Node.value; // add entry to map
	}
}

bool XMLParser::FindComplexKey ( const std::string& i_ExpectedKeyPath, XMLGenericType& o_Value )
{
	std::size_t pos = i_ExpectedKeyPath.find_last_of(".");
	std::string key = i_ExpectedKeyPath.substr(pos + 1);
	std::string actualKeyPath("");
	std::string expectedKeyPath(m_Tree.children[0].name + "." + i_ExpectedKeyPath);

	return FindKey(m_Tree.children[0], key, expectedKeyPath, actualKeyPath, o_Value);
}

bool XMLParser::FindKey ( const XMLNode& i_Node, const std::string& i_Key, const std::string& i_ExpectedKeyPath, std::string& o_ActualKeyPath, XMLGenericType& o_Value )
{
	o_ActualKeyPath += i_Node.name + ".";

	if (i_Key == i_Node.name)  // child node
	{
		if (o_ActualKeyPath == i_ExpectedKeyPath)
		{
			LOG("Key path is correct!");
		}
		else
		{
			LOG("Key path is incorrect!");
			LOG("Actual key path: " + o_ActualKeyPath);
			LOG("Expected key path: " + i_ExpectedKeyPath);
			return false;
		}

		o_Value = i_Node.value;

		LOG("A match key has been found!");
		LOG("Key path: " + i_ExpectedKeyPath);
		return true;
	}

	if (i_Node.children.size() > 0) // parent - node with children
	{
		std::vector<XMLNode>::const_iterator it;
		for (it = i_Node.children.begin(); it != i_Node.children.end(); ++ it)
		{
			bool ret = FindKey(*it, i_Key, i_ExpectedKeyPath, o_ActualKeyPath, o_Value);

			if (ret)
			{
				return true;
			}
		}
	}

	return false;
}

XMLGenericType XMLParser::ExtractValue ( const std::string& i_Key )
{
	XMLGenericType value;
	bool ret = FindComplexKey(i_Key, value);
	if (!ret)
	{
		ERR("Invalid key: " + i_Key);
	}

	return value;
}

////////////////////////////

XMLGenericType::XMLGenericType ( void )
{
}

XMLGenericType::XMLGenericType ( const std::string& i_Obj )
{
	m_Value = i_Obj;
}

XMLGenericType::XMLGenericType ( const XMLGenericType& i_Obj )
{
	m_Value = i_Obj.m_Value;
}

XMLGenericType::~XMLGenericType ( void )
{
}

XMLGenericType& XMLGenericType::operator= ( const XMLGenericType& i_Val )
{
	m_Value = i_Val.m_Value;

	return *this;
}

XMLGenericType& XMLGenericType::operator= ( const std::string& i_Val )
{
	m_Value = i_Val;

	return *this;
}

bool XMLGenericType::ToBool ( void )
{
	if (m_Value.empty())
	{
		ERR("Generic value is empty!");
		return false;
	}

	// tokenize the current generic value
	std::string delimiters = " ";
	std::vector<std::string> tokens;

	XMLParser::Tokenize(m_Value, delimiters, tokens);

	if (tokens.size() > 1)
	{
		ERR("Failed to convert to BOOL!\nToo many tokens");
		return false;
	}

	const std::string& token = tokens[0];
	bool val = false;

	if (token != "true" && token != "false")
	{
		ERR("Generic value has invalid value!\nThe accepted values are: true or false!");
		return false;
	}

	val = (token[0] == 't');

//	LOG("Bool value!");

	return val;
}

int XMLGenericType::ToInt ( void )
{
	if (m_Value.empty())
	{
		ERR("Generic value is empty!");
		return -1;
	}

	// tokenize the current generic value
	std::string delimiters = " ";
	std::vector<std::string> tokens;

	XMLParser::Tokenize(m_Value, delimiters, tokens);

	if (tokens.size() > 1)
	{
		ERR("Failed to convert to INT!\nToo many tokens");
		return -1;
	}

	const std::string& token = tokens[0];
	int val = 0;

	try
	{
		val = std::stoi(token);
	}
	catch (const std::invalid_argument& ia)
	{
		ERR("Invalid token: " << token << '\n');
		return -1;
	}

//	LOG("Int value!");

	return val;
}

float XMLGenericType::ToFloat ( void )
{
	if (m_Value.empty())
	{
		ERR("Generic value is empty!");
		return -1;
	}

	// tokenize the current generic value
	std::string delimiters = " ";
	std::vector<std::string> tokens;

	XMLParser::Tokenize(m_Value, delimiters, tokens);

	if (tokens.size() > 1)
	{
		ERR("Failed to convert to FLOAT!\nToo many tokens");
		return -1.0f;
	}

	const std::string& token = tokens[0];

	float val = 0.0f;
	if (token == "PI")
	{
		val = glm::pi<float>();
	}
	else
	{
		val = ConvertToFloat(token);
	}

//	LOG("Float value!");

	return val;
}
glm::vec2 XMLGenericType::ToVec2()
{
	if (m_Value.empty())
	{
		ERR("Generic value is empty!");
		return glm::vec2(-1.0f);
	}

	// tokenize the current generic value
	std::string delimiters = " ";
	std::vector<std::string> tokens;

	XMLParser::Tokenize(m_Value, delimiters, tokens);

	if (tokens.size() != 2)
	{
		ERR("Failed to convert to VEC2!\nWe need 2 tokens!");
		return glm::vec2(-1.0f);
	}

	glm::vec2 val(0.0f);
	val.x = ConvertToFloat(tokens[0]);
	val.y = ConvertToFloat(tokens[1]);

//	LOG("VEC2 value!");

	return val;
}

glm::vec3 XMLGenericType::ToVec3 ( void )
{
	if (m_Value.empty())
	{
		ERR("Generic value is empty!");
		return glm::vec3(-1.0f);
	}

	// tokenize the current generic value
	std::string delimiters = " ";
	std::vector<std::string> tokens;

	XMLParser::Tokenize(m_Value, delimiters, tokens);

	if (tokens.size() != 3)
	{
		ERR("Failed to convert to VEC3!\nWe need 3 tokens!");
		return glm::vec3(-1.0f);
	}

	glm::vec3 val(0.0f);
	val.x = ConvertToFloat(tokens[0]);
	val.y = ConvertToFloat(tokens[1]);
	val.z = ConvertToFloat(tokens[2]);

//	LOG("VEC3 value!");

	return val;
}


CustomTypes::Sky::ModelType XMLGenericType::ToSkyModelType ( void )
{
	if (m_Value == "SkyCubemap" || m_Value == "SkyScattering" || m_Value == "SkyPrecomputedScattering") // sky model
	{
		if (m_Value == "SkyCubemap")
			return CustomTypes::Sky::ModelType::MT_CUBE_MAP;

		if (m_Value == "SkyScattering")
			return CustomTypes::Sky::ModelType::MT_SCATTERING;

		if (m_Value == "SkyPrecomputedScattering")
			return CustomTypes::Sky::ModelType::MT_PRECOMPUTED_SCATTERING;
	}

	ERR("Invalid token: " << m_Value << "\n");
	return CustomTypes::Sky::ModelType::MT_COUNT;
}

CustomTypes::Ocean::ComputeFFTType XMLGenericType::ToOceanComputeFFTType ( void )
{
	if (m_Value == "FFTGpuFrag" || m_Value == "FFTGpuComp" || m_Value == "FFTCpuFFTW") // FFT compute
	{
		if (m_Value == "FFTGpuFrag")
			return CustomTypes::Ocean::ComputeFFTType::CFT_GPU_FRAG;

		if (m_Value == "FFTGpuComp")
			return CustomTypes::Ocean::ComputeFFTType::CFT_GPU_COMP;

		if (m_Value == "FFTCpuFFTW")
			return CustomTypes::Ocean::ComputeFFTType::CFT_CPU_FFTW;
	}

	ERR("Invalid token: " << m_Value << "\n");
	return CustomTypes::Ocean::ComputeFFTType::CFT_COUNT;
}

CustomTypes::Ocean::NormalGradientFoldingType XMLGenericType::ToOceanNormalGradientFoldingType ( void )
{
	if (m_Value == "NormalGpuFrag" || m_Value == "NormalGpuComp") // normal gradients + folding
	{
		if (m_Value == "NormalGpuFrag")
			return 	CustomTypes::Ocean::NormalGradientFoldingType::NGF_GPU_FRAG;

		if (m_Value == "NormalGpuComp")
			return 	CustomTypes::Ocean::NormalGradientFoldingType::NGF_GPU_COMP;
	}

	ERR("Invalid token: " << m_Value << "\n");
	return 	CustomTypes::Ocean::NormalGradientFoldingType::NGF_COUNT;
}


CustomTypes::Ocean::SpectrumType XMLGenericType::ToOceanSpectrumType ( void )
{
	if (m_Value == "SpectrumPhillips" || m_Value == "SpectrumUnified") // spctrum
	{
		if (m_Value == "SpectrumPhillips")
			return CustomTypes::Ocean::SpectrumType::ST_PHILLIPS;

		if (m_Value == "SpectrumUnified")
			return CustomTypes::Ocean::SpectrumType::ST_UNIFIED;
	}

	ERR("Invalid token: " << m_Value << "\n");
	return CustomTypes::Ocean::SpectrumType::ST_COUNT;
}

CustomTypes::Ocean::GridType XMLGenericType::ToOceanGridType ( void )
{
	if (m_Value == "GridWorldSpace" || m_Value == "GridScreenSpace") // grid
	{
		if (m_Value == "GridWorldSpace")
			return CustomTypes::Ocean::GridType::GT_WORLD_SPACE;

		if (m_Value == "GridScreenSpace")
			return CustomTypes::Ocean::GridType::GT_SCREEN_SPACE;
	}

	ERR("Invalid token: " << m_Value << "\n");
	return CustomTypes::Ocean::GridType::GT_COUNT;
}

CustomTypes::PostProcessing::EffectType XMLGenericType::ToPostProcessingEffectType ( void )
{
	if (m_Value == "EffectInvert" ||  m_Value == "EffectGrey" || m_Value == "EffectBlackWhite" ||
		m_Value == "EffectSepia" || m_Value == "EffectWavy" ||
		m_Value == "EffectBlur" || m_Value == "EffectEdgeDetection") // post processing effect
	{
		if (m_Value == "EffectInvert")
			return CustomTypes::PostProcessing::EffectType::PPET_Invert;

		if (m_Value == "EffectGrey")
			return CustomTypes::PostProcessing::EffectType::PPET_Grey;

		if (m_Value == "EffectBlackWhite")
			return CustomTypes::PostProcessing::EffectType::PPET_BlackWhite;

		if (m_Value == "EffectSepia")
			return CustomTypes::PostProcessing::EffectType::PPET_Sepia;

		if (m_Value == "EffectWavy")
			return CustomTypes::PostProcessing::EffectType::PPET_Wavy;

		if (m_Value == "EffectBlur")
			return CustomTypes::PostProcessing::EffectType::PPET_Blur;

		if (m_Value == "EffectEdgeDetection")
			return CustomTypes::PostProcessing::EffectType::PPET_EdgeDetection;
	}

	ERR("Invalid token: " << m_Value << "\n");
	return CustomTypes::PostProcessing::EffectType::PPET_COUNT;
}

float XMLGenericType::ConvertToFloat ( const std::string& i_Token )
{
	float val = 0.0f;
	try
	{
		val = std::stof(i_Token);
	}
	catch (const std::invalid_argument& ia)
	{
		ERR("Invalid token: " << i_Token << '\n');
		return -1.0f;
	}

	return val;
}