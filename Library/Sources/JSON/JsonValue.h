#pragma once

#include "../Common.h"

namespace MeshNinja
{
	class MESH_NINJA_API JsonValue
	{
	public:
		struct Token;

		JsonValue();
		virtual ~JsonValue();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const = 0;
		virtual bool ParseTokens(const std::vector<Token*>& tokenArray) = 0;

		static JsonValue* ParseJson(const std::string& jsonString);
		static bool Tokenize(const std::string& jsonString, std::vector<Token*>& tokenArray);
		static JsonValue* ValueFactory(const Token& token);
		static std::string MakeTabs(int tabCount);

		struct Token
		{
			enum Type
			{
				OPEN_CURLY_BRACKET,
				CLOSE_CURLY_BRACKET,
				OPEN_SQUARE_BRACKET,
				CLOSE_SQUARE_BRACKET,
				COMMA,
				STRING,
				NUMBER,
				BOOLEAN,
				COLON
			};

			bool Eat(const char* buffer, int& i);

			char buffer[128];
			Type type;
		};
	};

	class MESH_NINJA_API JsonString : public JsonValue
	{
	public:
		JsonString();
		virtual ~JsonString();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Token*>& tokenArray) override;

		const std::string& GetValue() const;
		void SetValue(const std::string& value);

	private:
		std::string* value;
	};

	class MESH_NINJA_API JsonNumber : public JsonValue
	{
	public:
		JsonNumber();
		virtual ~JsonNumber();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Token*>& tokenArray) override;

		double GetValue() const;
		void SetValue(double value);

	private:
		double value;
	};

	class MESH_NINJA_API JsonObject : public JsonValue
	{
	public:
		JsonObject();
		virtual ~JsonObject();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Token*>& tokenArray) override;

		void Clear();
		unsigned int GetSize() const;
		const JsonValue* GetValue(const std::string& key) const;
		JsonValue* GetValue(const std::string& key);
		bool SetValue(const std::string& key, JsonValue* value);
		bool DeleteValue(const std::string& key);

		// TODO: Provide iteration.

	private:
		typedef std::map<std::string, JsonValue*> JsonValueMap;
		JsonValueMap* valueMap;
	};

	class MESH_NINJA_API JsonArray : public JsonValue
	{
	public:
		JsonArray();
		virtual ~JsonArray();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Token*>& tokenArray) override;

		void Clear();
		unsigned int GetSize() const;
		const JsonValue* GetValue(unsigned int i) const;
		JsonValue* GetValue(unsigned int i);
		bool SetValue(unsigned int i, JsonValue* value);
		bool RemoveValue(unsigned int i);
		bool InsertValue(unsigned int i, JsonValue* value);
		void PushValue(JsonValue* value);
		JsonValue* PopValue();

	private:
		typedef std::vector<JsonValue*> JsonValueArray;
		JsonValueArray* valueArray;
	};

	class MESH_NINJA_API JsonBool : public JsonValue
	{
	public:
		JsonBool();
		virtual ~JsonBool();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Token*>& tokenArray) override;

		bool GetValue() const;
		void SetValue(bool value);

	private:
		bool value;
	};

	class MESH_NINJA_API JsonNull : public JsonValue
	{
	public:
		JsonNull();
		virtual ~JsonNull();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Token*>& tokenArray) override;
	};
}