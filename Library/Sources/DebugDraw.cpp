#include "DebugDraw.h"
#include "JSON/JsonValue.h"

using namespace MeshNinja;

DebugDraw::DebugDraw()
{
	this->objectArray = new ObjectArray();
}

/*virtual*/ DebugDraw::~DebugDraw()
{
	this->Clear();

	delete this->objectArray;
}

void DebugDraw::Clear()
{
	for (Object* object : *this->objectArray)
		delete object;

	this->objectArray->clear();
}

/*virtual*/ bool DebugDraw::Draw() const
{
	for (const Object* object : *this->objectArray)
		if (!object->Draw())
			return false;

	return true;
}

/*virtual*/ bool DebugDraw::Load(const std::string& filePath)
{
	JsonValue* jsonValue = nullptr;
	bool success = false;

	do
	{
		this->Clear();

		std::ifstream fileStream(filePath, std::ios::in);
		if (!fileStream.is_open())
			break;

		std::stringstream stringStream;
		stringStream << fileStream.rdbuf();
		std::string jsonString = stringStream.str();

		jsonValue = JsonValue::ParseJson(jsonString);
		if (!jsonValue)
			break;

		const JsonArray* jsonArray = dynamic_cast<const JsonArray*>(jsonValue);
		if (!jsonArray)
			break;

		for (int i = 0; i < (signed)jsonArray->GetSize(); i++)
		{
			const JsonObject* jsonObject = dynamic_cast<const JsonObject*>(jsonArray->GetValue(i));
			if (!jsonObject)
				break;

			const JsonString* jsonTypeString = dynamic_cast<const JsonString*>(jsonObject->GetValue("type"));
			if (!jsonTypeString)
				break;

			Object* object = this->Factory(jsonTypeString->GetValue());
			if (!object)
				break;

			if (!object->Load(jsonObject))
			{
				delete object;
				break;
			}

			this->objectArray->push_back(object);
		}

		if (this->objectArray->size() != jsonArray->GetSize())
			break;

		success = true;
	} while (false);

	delete jsonValue;

	return success;
}

/*virtual*/ bool DebugDraw::Save(const std::string& filePath) const
{
	bool success = false;
	JsonArray* jsonArray = nullptr;

	do
	{
		std::ofstream fileStream(filePath, std::ios::out);
		if (!fileStream.is_open())
			break;

		jsonArray = new JsonArray();

		for (const Object* object : *this->objectArray)
		{
			JsonObject* jsonObject = new JsonObject();
			jsonArray->PushValue(jsonObject);
			if (!object->Save(jsonObject))
				break;
		}

		if (jsonArray->GetSize() != this->objectArray->size())
			break;

		std::string jsonString;
		if (!jsonArray->PrintJson(jsonString))
			break;

		fileStream << jsonString;
		fileStream.close();

		success = true;
	} while (false);

	delete jsonArray;

	return success;
}

/*virtual*/ DebugDraw::Object* DebugDraw::Factory(const std::string& objectType) const
{
	if (objectType == "point")
		return new Point();
	else if (objectType == "line")
		return new Line();

	return nullptr;
}

void DebugDraw::AddObject(Object* object)
{
	this->objectArray->push_back(object);
}

//------------------------------ DebugDraw::Object ------------------------------

DebugDraw::Object::Object()
{
	this->color = Vector3(1.0, 1.0, 1.0);
}

DebugDraw::Object::Object(const Vector3& color)
{
	this->color = color;
}

/*virtual*/ DebugDraw::Object::~Object()
{
}

/*virtual*/ bool DebugDraw::Object::Draw() const
{
	// This needs to be overridden.
	return false;
}

/*virtual*/ bool DebugDraw::Object::Save(JsonObject* jsonObject) const
{
	jsonObject->SetValue("type", new JsonString(this->GetType()));

	if (!this->SaveVector(jsonObject, "color", this->color))
		return false;

	return true;
}

/*virtual*/ bool DebugDraw::Object::Load(const JsonObject* jsonObject)
{
	if (!this->LoadVector(jsonObject, "color", this->color))
		return false;

	return true;
}

/*static*/ bool DebugDraw::Object::SaveVector(JsonObject* jsonObject, const std::string& key, const Vector3& vector)
{
	if (jsonObject->GetValue(key))
		return false;

	JsonObject* jsonVector = new JsonObject();
	jsonVector->SetValue("x", new JsonFloat(vector.x));
	jsonVector->SetValue("y", new JsonFloat(vector.y));
	jsonVector->SetValue("z", new JsonFloat(vector.z));

	return jsonObject->SetValue(key, jsonVector);
}

/*static*/ bool DebugDraw::Object::LoadVector(const JsonObject* jsonObject, const std::string& key, Vector3& vector)
{
	const JsonObject* jsonVector = dynamic_cast<const JsonObject*>(jsonObject->GetValue(key));
	if (!jsonVector)
		return false;

	const JsonFloat* jsonX = dynamic_cast<const JsonFloat*>(jsonVector->GetValue("x"));
	const JsonFloat* jsonY = dynamic_cast<const JsonFloat*>(jsonVector->GetValue("y"));
	const JsonFloat* jsonZ = dynamic_cast<const JsonFloat*>(jsonVector->GetValue("z"));

	if (!jsonX || !jsonY || !jsonZ)
		return false;

	vector.x = jsonX->GetValue();
	vector.y = jsonY->GetValue();
	vector.z = jsonZ->GetValue();

	return true;
}

//------------------------------ DebugDraw::Point ------------------------------

DebugDraw::Point::Point()
{
	this->vertex = Vector3(0.0, 0.0, 0.0);
}

DebugDraw::Point::Point(const Vector3& vertex, const Vector3& color) : Object(color)
{
	this->vertex = vertex;
}

/*virtual*/ DebugDraw::Point::~Point()
{
}

/*static*/ std::string DebugDraw::Point::GetStaticType()
{
	return "point";
}

/*virtual*/ std::string DebugDraw::Point::GetType() const
{
	return "point";
}

/*virtual*/ bool DebugDraw::Point::Save(JsonObject* jsonObject) const
{
	if (!Object::Save(jsonObject))
		return false;

	if (!this->SaveVector(jsonObject, "vertex", this->vertex))
		return false;

	return true;
}

/*virtual*/ bool DebugDraw::Point::Load(const JsonObject* jsonObject)
{
	if (!Object::Load(jsonObject))
		return false;

	if (!this->LoadVector(jsonObject, "vertex", this->vertex))
		return false;

	return true;
}

//------------------------------ DebugDraw::Line ------------------------------

DebugDraw::Line::Line()
{
	this->vertex[0] = Vector3(0.0, 0.0, 0.0);
	this->vertex[1] = Vector3(1.0, 0.0, 0.0);
}

DebugDraw::Line::Line(const Vector3& pointA, const Vector3& pointB, const Vector3& color) : Object(color)
{
	this->vertex[0] = pointA;
	this->vertex[1] = pointB;
}

/*virtual*/ DebugDraw::Line::~Line()
{
}

/*static*/ std::string DebugDraw::Line::GetStaticType()
{
	return "line";
}

/*virtual*/ std::string DebugDraw::Line::GetType() const
{
	return "line";
}

/*virtual*/ bool DebugDraw::Line::Save(JsonObject* jsonObject) const
{
	if (!Object::Save(jsonObject))
		return false;

	if (!this->SaveVector(jsonObject, "vertexA", this->vertex[0]))
		return false;

	if (!this->SaveVector(jsonObject, "vertexB", this->vertex[1]))
		return false;

	return true;
}

/*virtual*/ bool DebugDraw::Line::Load(const JsonObject* jsonObject)
{
	if (!Object::Load(jsonObject))
		return false;

	if (!this->LoadVector(jsonObject, "vertexA", this->vertex[0]))
		return false;

	if (!this->LoadVector(jsonObject, "vertexB", this->vertex[1]))
		return false;

	return true;
}