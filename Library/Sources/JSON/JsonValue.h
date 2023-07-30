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
				FLOAT,
				INT,
				BOOLEAN,
				COLON
			};

			bool Eat(const char* givenBuffer, int& i);

			char buffer[128];
			Type type;
		};
	};

	class MESH_NINJA_API JsonString : public JsonValue
	{
	public:
		JsonString();
		JsonString(const std::string& value);
		virtual ~JsonString();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Token*>& tokenArray) override;

		const std::string& GetValue() const;
		void SetValue(const std::string& value);

	private:
		std::string* value;
	};

	class MESH_NINJA_API JsonFloat : public JsonValue
	{
	public:
		JsonFloat();
		JsonFloat(double value);
		virtual ~JsonFloat();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Token*>& tokenArray) override;

		double GetValue() const;
		void SetValue(double value);

	private:
		double value;
	};

	class MESH_NINJA_API JsonInt : public JsonValue
	{
	public:
		JsonInt();
		JsonInt(long value);
		virtual ~JsonInt();

		virtual bool PrintJson(std::string& jsonString, int tabLevel = 0) const override;
		virtual bool ParseTokens(const std::vector<Token*>& tokenArray) override;

		long GetValue() const;
		void SetValue(long value);

	private:
		long value;
	};

	class MESH_NINJA_API JsonStructure : public JsonValue
	{
	public:
		JsonStructure();
		virtual ~JsonStructure();

	protected:
		bool ParseTokensWithCallback(const std::vector<Token*>& tokenArray, std::function<bool(const std::vector<Token*>&)> callback);
	};

	class MESH_NINJA_API JsonObject : public JsonStructure
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

	class MESH_NINJA_API JsonArray : public JsonStructure
	{
	public:
		JsonArray();
		JsonArray(const std::vector<double>& floatArray);
		JsonArray(const std::vector<int>& intArray);
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
		JsonBool(bool value);
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