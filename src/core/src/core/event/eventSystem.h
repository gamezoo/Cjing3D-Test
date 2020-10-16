#pragma once

#include "eventDefine.h"
#include "core\common\definitions.h"
#include "core\helper\variant.h"

namespace Cjing3D
{
	// global event system
	namespace EventSystem
	{
		using CallbackFunc = std::function<void(const VariantArray&)>;

		Connection Register(EventType eventID, CallbackFunc func);
		void RegisterOnce(EventType eventID, CallbackFunc func);
		void Fire(EventType eventID, const VariantArray& variants);
	}
}