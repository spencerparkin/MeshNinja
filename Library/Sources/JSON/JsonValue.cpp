#include "JsonValue.h"

using namespace MeshNinja;

//-------------------------------- JsonValue --------------------------------

JsonValue::JsonValue()
{
}

/*virtual*/ JsonValue::~JsonValue()
{
}

/*static*/ JsonValue* JsonValue::ParseJson(const std::string & jsonString)
{
	std::vector<Token*> tokenArray;
	if (!Tokenize(jsonString, tokenArray))
		return nullptr;

	if (tokenArray.size() == 0)
		return nullptr;

	JsonValue* jsonValue = ValueFactory(*tokenArray[0]);
	if (jsonValue)
		jsonValue->ParseTokens(tokenArray);

	for (Token* token : tokenArray)
		delete token;

	return jsonValue;
}

/*static*/ JsonValue* JsonValue::ValueFactory(const Token& token)
{
	if (token.type == Token::OPEN_SQUARE_BRACKET)
		return new JsonArray();
	else if (token.type == Token::OPEN_CURLY_BRACKET)
		return new JsonObject();
	else if (token.type == Token::STRING)
		return new JsonString();
	else if (token.type == Token::NUMBER)
		return new JsonNumber();
	else if (token.type == Token::BOOLEAN)
		return new JsonBool();

	return nullptr;
}

/*static*/ bool JsonValue::Tokenize(const std::string& jsonString, std::vector<Token*>& tokenArray)
{
	int i = 0;
	while (i < (signed)jsonString.length())
	{
		Token* token = new Token();
		if (!token->Eat(jsonString.c_str(), i))
		{
			delete token;
			return false;
		}

		tokenArray.push_back(token);
	}

	return true;
}

bool JsonValue::Token::Eat(const char* buffer, int& i)
{
	while (::isspace(buffer[i]))
		i++;

	const char* tokenText = &buffer[i];
	if (tokenText[0] == '"')
	{
		int j = 0;
		while (tokenText[j] != '\0' && tokenText[j] != '"')
		{
			this->buffer[j] = tokenText[j];
			j++;
		}

		if (tokenText[j] == '\0')
			return false;

		this->type = Token::STRING;
		return true;
	}
	else if (tokenText[0] == '-' || ::isdigit(tokenText[0]))
	{
		int j = 0;
		while (tokenText[j] != '\0' && (tokenText[0] == '-' || tokenText[0] == '.' || ::isdigit(tokenText[0])))
		{
			this->buffer[j] = tokenText[j];
			j++;
		}

		this->type = Token::NUMBER;
		return true;
	}
	else if (tokenText[0] == '{')
	{
		this->type = Token::OPEN_CURLY_BRACKET;
		i++;
		return true;
	}
	else if (tokenText[0] == '}')
	{
		this->type = Token::CLOSE_CURLY_BRACKET;
		i++;
		return true;
	}
	else if (tokenText[0] == '[')
	{
		this->type = Token::OPEN_SQUARE_BRACKET;
		i++;
		return true;
	}
	else if (tokenText[0] == ']')
	{
		this->type = Token::CLOSE_SQUARE_BRACKET;
		i++;
		return true;
	}
	else if (tokenText[0] == ':')
	{
		this->type = Token::COLON;
		i++;
		return true;
	}
	else if (tokenText[0] == ',')
	{
		this->type = Token::COMMA;
		i++;
		return true;
	}

	return false;
}

/*static*/ std::string JsonValue::MakeTabs(int tabCount)
{
	std::string tabString;
	for (int i = 0; i < tabCount; i++)
		tabString += "\t";
	return tabString;
}

//-------------------------------- JsonString --------------------------------

JsonString::JsonString()
{
	this->value = new std::string();
}

/*virtual*/ JsonString::~JsonString()
{
	delete this->value;
}

/*virtual*/ bool JsonString::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	jsonString = "\"" + *this->value + "\"";
	return true;
}

/*virtual*/ bool JsonString::ParseTokens(const std::vector<Token*>& tokenArray)
{
	if (tokenArray.size() != 1 || tokenArray[0]->type != Token::STRING)
		return false;

	this->SetValue(tokenArray[0]->buffer);
	return true;
}

const std::string& JsonString::GetValue() const
{
	return *this->value;
}

void JsonString::SetValue(const std::string& value)
{
	*this->value = value;
}

//-------------------------------- JsonNumber --------------------------------

JsonNumber::JsonNumber()
{
	this->value = 0.0;
}

/*virtual*/ JsonNumber::~JsonNumber()
{
}

/*virtual*/ bool JsonNumber::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	char buffer[128];
	sprintf_s(buffer, sizeof(buffer), "%f", this->value);
	jsonString = buffer;
	return true;
}

/*virtual*/ bool JsonNumber::ParseTokens(const std::vector<Token*>& tokenArray)
{
	if (tokenArray.size() != 1 || tokenArray[0]->type != Token::NUMBER)
		return false;

	this->SetValue(::atof(tokenArray[0]->buffer));
	return true;
}

double JsonNumber::GetValue() const
{
	return value;
}

void JsonNumber::SetValue(double value)
{
	this->value = value;
}

//-------------------------------- JsonObject --------------------------------

JsonObject::JsonObject()
{
	this->valueMap = new JsonValueMap();
}

/*virtual*/ JsonObject::~JsonObject()
{
	this->Clear();

	delete this->valueMap;
}

/*virtual*/ bool JsonObject::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	jsonString += MakeTabs(tabLevel) + "{\n";
	
	int i = 0;

	for (std::pair<std::string, JsonValue*> pair : *this->valueMap)
	{
		if (i++ > 0)
			jsonString += ",\n";

		jsonString += MakeTabs(tabLevel + 1) + pair.first + ":";
		
		std::string jsonSubString;
		if (!pair.second->PrintJson(jsonSubString, tabLevel + 1))
			return false;

		if (dynamic_cast<JsonArray*>(pair.second) || dynamic_cast<JsonObject*>(pair.second))
			jsonString += "\n";
		else
			jsonString += " ";

		// I suppose this is not very efficient.
		jsonString += jsonSubString;
	}

	jsonString += MakeTabs(tabLevel) + "}";

	return true;
}

/*virtual*/ bool JsonObject::ParseTokens(const std::vector<Token*>& tokenArray)
{
	if (tokenArray.size() < 2)
		return false;

	if (tokenArray[0]->type != Token::OPEN_CURLY_BRACKET)
		return false;

	std::vector<Token*> subTokenArray;
	unsigned int level = 0;
	for (unsigned int i = 0; i < tokenArray.size(); i++)
	{
		const Token* token = tokenArray[i];
		switch (token->type)
		{
			case Token::OPEN_CURLY_BRACKET:
			case Token::OPEN_SQUARE_BRACKET:
			{
				level++;
				break;
			}
			case Token::CLOSE_CURLY_BRACKET:
			case Token::CLOSE_SQUARE_BRACKET:
			{
				if (level == 0)
					return false;

				level--;
				break;
			}
			case Token::COMMA:
			{
				if (level == 1)
				{
					if (subTokenArray.size() < 3)
						return false;

					if (subTokenArray[0]->type != Token::STRING || subTokenArray[1]->type != Token::COLON)
						return false;

					std::string key = subTokenArray[0]->buffer;
					if (this->GetValue(key))
						return false;

					// Hmmm...this is expensive for an array.
					subTokenArray.erase(subTokenArray.begin());
					subTokenArray.erase(subTokenArray.begin());

					JsonValue* jsonValue = ValueFactory(*subTokenArray[0]);
					if (!jsonValue)
						return false;

					if (!this->SetValue(key, jsonValue))
						return false;
				}

				break;
			}
			default:
			{
				subTokenArray.push_back(const_cast<Token*>(token));
				break;
			}
		}
	}

	if (level != 0)
		return false;

	return true;
}

void JsonObject::Clear()
{
	for (std::pair<std::string, JsonValue*> pair : *this->valueMap)
		delete pair.second;

	this->valueMap->clear();
}

unsigned int JsonObject::GetSize() const
{
	return this->valueMap->size();
}

const JsonValue* JsonObject::GetValue(const std::string& key) const
{
	return const_cast<JsonObject*>(this)->GetValue(key);
}

JsonValue* JsonObject::GetValue(const std::string& key)
{
	JsonValueMap::iterator iter = this->valueMap->find(key);
	if (iter == this->valueMap->end())
		return nullptr;

	return iter->second;
}

bool JsonObject::SetValue(const std::string& key, JsonValue* value)
{
	this->DeleteValue(key);
	this->valueMap->insert(std::pair<std::string, JsonValue*>(key, value));
	return true;
}

bool JsonObject::DeleteValue(const std::string& key)
{
	JsonValueMap::iterator iter = this->valueMap->find(key);
	if (iter == this->valueMap->end())
		return false;

	delete iter->second;
	this->valueMap->erase(iter);
	return true;
}

//-------------------------------- JsonArray --------------------------------

JsonArray::JsonArray()
{
	this->valueArray = new JsonValueArray();
}

/*virtual*/ JsonArray::~JsonArray()
{
	this->Clear();

	delete this->valueArray;
}

/*virtual*/ bool JsonArray::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	jsonString += MakeTabs(tabLevel) + "[\n";

	for (int i = 0; i < (signed)this->valueArray->size(); i++)
	{
		if (i > 0)
			jsonString += ",\n";

		std::string jsonSubString;
		if (!(*this->valueArray)[i]->PrintJson(jsonSubString, tabLevel + 1))
			return false;

		// Again, this isn't terribly efficient.
		jsonString += jsonSubString;
	}

	jsonString += MakeTabs(tabLevel) + "]";

	return true;
}

/*virtual*/ bool JsonArray::ParseTokens(const std::vector<Token*>& tokenArray)
{
	if (tokenArray.size() < 2)
		return false;

	if (tokenArray[0]->type != Token::OPEN_SQUARE_BRACKET)
		return false;

	std::vector<Token*> subTokenArray;
	unsigned int level = 0;
	for (unsigned int i = 0; i < tokenArray.size(); i++)
	{
		const Token* token = tokenArray[i];
		switch (token->type)
		{
			case Token::OPEN_CURLY_BRACKET:
			case Token::OPEN_SQUARE_BRACKET:
			{
				level++;
				break;
			}
			case Token::CLOSE_CURLY_BRACKET:
			case Token::CLOSE_SQUARE_BRACKET:
			{
				if (level == 0)
					return false;

				level--;
				break;
			}
			case Token::COMMA:
			{
				if (level == 1)
				{
					if (subTokenArray.size() == 0)
						return false;

					JsonValue* jsonValue = ValueFactory(*subTokenArray[0]);
					if (!jsonValue)
						return false;

					this->PushValue(jsonValue);

					if (!jsonValue->ParseTokens(subTokenArray))
						return false;

					subTokenArray.clear();
				}

				break;
			}
			default:
			{
				subTokenArray.push_back(const_cast<Token*>(token));
				break;
			}
		}
	}

	if (level != 0)
		return false;

	return true;
}

void JsonArray::Clear()
{
	for (JsonValue* value : *this->valueArray)
		delete value;

	this->valueArray->clear();
}

unsigned int JsonArray::GetSize() const
{
	return this->valueArray->size();
}

const JsonValue* JsonArray::GetValue(unsigned int i) const
{
	return const_cast<JsonArray*>(this)->GetValue(i);
}

JsonValue* JsonArray::GetValue(unsigned int i)
{
	if (i >= this->valueArray->size())
		return nullptr;

	return (*this->valueArray)[i];
}

bool JsonArray::SetValue(unsigned int i, JsonValue* value)
{
	return false;
}

bool JsonArray::RemoveValue(unsigned int i)
{
	return false;
}

bool JsonArray::InsertValue(unsigned int i, JsonValue* value)
{
	return false;
}

void JsonArray::PushValue(JsonValue* value)
{
	this->valueArray->push_back(value);
}

JsonValue* JsonArray::PopValue()
{
	if (this->valueArray->size() == 0)
		return nullptr;

	JsonValue* jsonValue = this->valueArray->back();
	this->valueArray->pop_back();
	return jsonValue;
}

//-------------------------------- JsonBool --------------------------------

JsonBool::JsonBool()
{
	this->value = false;
}

/*virtual*/ JsonBool::~JsonBool()
{
}

/*virtual*/ bool JsonBool::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	return true;
}

/*virtual*/ bool JsonBool::ParseTokens(const std::vector<Token*>& tokenArray)
{
	if (tokenArray.size() != 1 || tokenArray[0]->type != Token::BOOLEAN)
		return false;

	if (0 == ::strcmp(tokenArray[0]->buffer, "true"))
		this->SetValue(true);
	else if (0 == ::strcmp(tokenArray[0]->buffer, "false"))
		this->SetValue(false);
	else
		return false;

	return true;
}

bool JsonBool::GetValue() const
{
	return this->value;
}

void JsonBool::SetValue(bool value)
{
	this->value = value;
}

//-------------------------------- JsonNull --------------------------------

JsonNull::JsonNull()
{
}

/*virtual*/ JsonNull::~JsonNull()
{
}

/*virtual*/ bool JsonNull::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	return true;
}

/*virtual*/ bool JsonNull::ParseTokens(const std::vector<Token*>& tokenArray)
{
	if (tokenArray.size() != 1 || tokenArray[0]->type != Token::STRING)
		return false;

	if (0 == ::strcmp(tokenArray[0]->buffer, "null"))
		return true;

	return true;
}