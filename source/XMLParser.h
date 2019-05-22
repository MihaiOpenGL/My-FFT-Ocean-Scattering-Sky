/* Author: BAIRAC MIHAI */

#ifndef XML_PARSER_H
#define XML_PARSER_H

#include "CustomTypes.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <string>
#include <vector>
#include <map>

/*

 Small helper class to mange geenric types available in my XML confif file
 Supported types:
 bool, int, float, vec2, vec3, customs ones (see below)

*/

class XMLGenericType
{
public:
	XMLGenericType(void);
	XMLGenericType(const std::string& i_Obj);
	XMLGenericType(const XMLGenericType& i_Obj);
	~XMLGenericType(void);

	XMLGenericType& operator = (const XMLGenericType& i_Val);
	XMLGenericType& operator = (const std::string& i_Val);

	bool ToBool(void);
	int ToInt(void);
	float ToFloat(void);
	glm::vec2 ToVec2(void);
	glm::vec3 ToVec3(void);

	CustomTypes::Sky::ModelType ToSkyModelType(void);
	CustomTypes::Ocean::ComputeFFTType ToOceanComputeFFTType(void);
	CustomTypes::Ocean::NormalGradientFoldingType ToOceanNormalGradientFoldingType(void);
	CustomTypes::Ocean::SpectrumType ToOceanSpectrumType(void);
	CustomTypes::Ocean::GridType ToOceanGridType(void);
	CustomTypes::PostProcessing::EffectType ToPostProcessingEffectType(void);

private:
	///// Methods /////
	float ConvertToFloat(const std::string& i_Token);

	//// Variables /////
	std::string m_Value;
};

//////////////////////////////////

/*
 Simple XML parser that traverses the XML config file creating a XML tree
 Later on the XML tree is used by the GlobalConfig class to setup the internal config structs
*/

class XMLParser
{
	friend XMLGenericType;

public:
	typedef struct XMLNode
	{
		std::string name;
		XMLGenericType value;
		XMLNode* pParent;
		std::vector<XMLNode> children;
	} XMLNode;

	XMLParser(void);
	XMLParser(const std::string& i_FileName);
	~XMLParser(void);

	void PopulateKeyMap(std::map<std::string, XMLGenericType>& o_KeyMap);

	XMLGenericType ExtractValue(const std::string& i_Key);

private:
	///// Methods /////
	void Initialize(const std::string& i_FileName);

	static void Tokenize(const std::string& i_Content, const std::string& i_Delimiters, std::vector<std::string>& o_Elements, bool i_SkipTabElement = false);

	void AddNode(XMLNode& io_Parent, unsigned short i_LineIndex, const std::vector<std::string>& i_Lines);

	void PopulateKey(const XMLNode& i_Node, const std::string& i_KeyPath, std::map<std::string, XMLGenericType>& o_KeyMap);

	bool FindComplexKey(const std::string& i_ExpectedKeyPath, XMLGenericType& o_Value);
	bool FindKey(const XMLNode& i_Node, const std::string& i_Key, const std::string& i_ExpectedKeyPath, std::string& o_ActualKeyPath, XMLGenericType& o_Value);

	void Destroy(void);

	///// Variables /////
	XMLNode m_Tree;
};

#endif /* XML_PARSER_H */