
#include <iostream>

#include "math\maths.h"
#include "core\filesystem\path.h"
#include "core\memory\memTracker.h"
#include "core\memory\memory.h"
#include "core\string\string.h"
#include "core\container\span.h"
#include "core\container\dynamicArray.h"
#include "core\container\map.h"
#include "core\helper\enumTraits.h"

using namespace Cjing3D;

enum EnumTest
{
    Enum1, 
    Enum2, 
    Enum3,
    Enum4 = 10,
};

int main()
{
    std::cout << EnumTraits::Impl::EnumRange<EnumTest>() << std::endl;
    EnumTraits::Impl::IndexT<EnumTest> a = 10;

    auto str = EnumTraits::EnumToName(Enum4);
    std::cout << str << std::endl;

    return 0;
}