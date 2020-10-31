#pragma once

#include "client\common\common.h"

#include "core\container\dynamicArray.h"

namespace Cjing3D
{
	class Module
	{
	public:
		Module() = default;
		virtual ~Module() = default;

		virtual void Initialize() {};
		virtual void Uninitialize() {};
		virtual void Update(F32 delta) {}

		void SetInitialized(bool isInitialized) { mIsInitialized = isInitialized; }
		bool IsInitialized()const { return mIsInitialized; }
		void SetName(const String& name) { mName = name; }
		const String& GetName()const { return mName; }
		void SetEnable(bool isEnable) { mIsEnable = isEnable; }
		bool IsEnable()const { return mIsEnable; }

	private:
		String mName;
		bool mIsEnable = true;
		bool mIsInitialized = false;
	};

	class ModuleManager
	{
	public:
		~ModuleManager();

		static ModuleManager& Instance();
		static void RegisterAllModules();

		void InitModules();
		void UninitModules();
		void UpdateModules(F32 delta);
		void Clear();

		template<typename T>
		void AddModule(const char* name)
		{
			Module* module = CJING_DOWN_CAST<Module>(CJING_NEW(T));
			module->SetName(name);
			module->SetEnable(true);

			AddModule(module);
		}

		void AddModule(Module* module);

		DynamicArray<Module*>& GetModules() { return mModules; }

	private:
		ModuleManager() = default;

		DynamicArray<Module*> mModules;
	};
}