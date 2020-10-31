#include "module.h"

namespace Cjing3D
{
	void ModuleManager::RegisterAllModules()
	{
	}

	////////////////////////////////////////////////////////////////////////
	/// Moduler Mangaer
	/// 
	ModuleManager::~ModuleManager()
	{
		Clear();
	}

	ModuleManager& ModuleManager::Instance()
	{
		static ModuleManager instance;
		return instance;
	}

	void ModuleManager::InitModules()
	{
		for (auto module : mModules)
		{
			if (!module->IsInitialized()) 
			{
				module->SetInitialized(true);
				module->Initialize();
			}
		}
	}

	void ModuleManager::UninitModules()
	{
		for (auto module : mModules)
		{
			if (module->IsInitialized())
			{
				module->SetInitialized(false);
				module->Uninitialize();
			}
		}
	}

	void ModuleManager::UpdateModules(F32 delta)
	{
		for (auto module : mModules)
		{
			if (module->IsEnable()) {
				module->Update(delta);
			}
		}
	}

	void ModuleManager::Clear()
	{
		for (auto module : mModules)
		{
			if (module->IsInitialized()) 
			{
				module->SetInitialized(false);
				module->Uninitialize();
			}
			CJING_SAFE_DELETE(module);
		}
		mModules.clear();
	}

	void ModuleManager::AddModule(Module* module)
	{
		if (!module) {
			return;
		}
		mModules.push(module);
	}
}