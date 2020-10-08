#pragma once

#if __has_include("DirectXMath.h")
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
#else
#include "directXMath\DirectXMath.h"
#include "directXMath\DirectXPackedVector.h"
#include "directXMath\DirectXCollision.h"
#endif

using namespace DirectX;
using namespace DirectX::PackedVector;