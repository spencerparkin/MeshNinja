#pragma once

#include "Math/Vector3.h"

namespace MeshNinja
{
	class JsonObject;

	class MESH_NINJA_API DebugDraw
	{
	public:
		DebugDraw();
		virtual ~DebugDraw();

		class Object;

		virtual bool Draw() const;
		virtual bool Load(const std::string& filePath);
		virtual bool Save(const std::string& filePath) const;
		virtual Object* Factory(const std::string& objectType) const;

		template<typename T>
		T* New()
		{
			Object* object = this->Factory(T::GetStaticType());
			
			T* t = dynamic_cast<T*>(object);
			if (!t)
				delete object;

			return t;
		}

		template<typename T>
		static T* Cast(Object* object)
		{
			if (T::GetStaticType() == object->GetType())
				return (T*)object;

			return nullptr;
		}

		void Clear();
		void AddObject(Object* object);

		class MESH_NINJA_API Object
		{
		public:
			Object();
			Object(const Vector3& color);
			virtual ~Object();

			virtual std::string GetType() const = 0;
			virtual bool Draw() const;
			virtual bool Save(JsonObject* jsonObject) const;
			virtual bool Load(const JsonObject* jsonObject);

			static bool SaveVector(JsonObject* jsonObject, const std::string& key, const Vector3& vector);
			static bool LoadVector(const JsonObject* jsonObject, const std::string& key, Vector3& vector);

			Vector3 color;
		};

		class MESH_NINJA_API Point : public Object
		{
		public:
			Point();
			Point(const Vector3& vertex, const Vector3& color);
			virtual ~Point();

			static std::string GetStaticType();
			virtual std::string GetType() const override;
			virtual bool Save(JsonObject* jsonObject) const override;
			virtual bool Load(const JsonObject* jsonObject) override;

			Vector3 vertex;
		};

		class MESH_NINJA_API Line : public Object
		{
		public:
			Line();
			Line(const Vector3& pointA, const Vector3& pointB, const Vector3& color);
			virtual ~Line();

			static std::string GetStaticType();
			virtual std::string GetType() const override;
			virtual bool Save(JsonObject* jsonObject) const override;
			virtual bool Load(const JsonObject* jsonObject) override;

			Vector3 vertex[2];
		};

	protected:

		typedef std::vector<Object*> ObjectArray;
		ObjectArray* objectArray;
	};
}