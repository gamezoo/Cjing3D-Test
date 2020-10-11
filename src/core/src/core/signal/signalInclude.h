#pragma once

#include "core\common\definitions.h"
#include "core\jobsystem\concurrency.h"

#include <memory>
#include <functional>
#include <optional>

namespace Cjing3D
{
	namespace SingalImpl
	{
		template <typename FUNC>
		class SignalBody;
	}

	template <typename FUNC>
	class Signal;
}