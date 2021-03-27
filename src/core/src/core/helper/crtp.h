#pragma once

#include "core\memory\memory.h"

namespace Cjing3D
{
	/**
	* CRTP: ����ĵݹ�ģ��ģʽ
	* 1.�̳���ģ���ࡣ
	* 2.�����ཫ������Ϊ��������ģ���ࡣ
	* https://stackoverflow.com/questions/18174441/crtp-and-multilevel-inheritance
	*/

	template<class DerivedT, bool enable_shared_from_this = true>
	class ObjectCRTP : public ENABLE_SHARED_FROM_THIS<DerivedT>
	{
	public:
		ObjectCRTP() = default;
		~ObjectCRTP() = default;

	protected:
		inline const DerivedT& Derived()const {
			return static_cast<const DerivedT&>(*this);
		}

		inline DerivedT& Derived() {
			return static_cast<DerivedT&>(*this);
		}
	};

	template<class DerivedT>
	class ObjectCRTP<DerivedT, false>
	{
	public:
		ObjectCRTP() = default;
		~ObjectCRTP() = default;

	protected:
		inline const DerivedT& Derived()const {
			return static_cast<const DerivedT&>(*this);
		}

		inline DerivedT& Derived() {
			return static_cast<DerivedT&>(*this);
		}
	};
}