/*!
	@file
	@author		Albert Semenov
	@date		07/2008
	@module
*/

#ifndef BASE_LAYOUT_H_
#define BASE_LAYOUT_H_

#include <MyGUI/MyGUI.h>

namespace attribute
{
	// =========
	// Attribute.h
	// =========

	// класс обертка для удаления данных из статического вектора
	template <typename Type>
	struct DataHolder
	{
		~DataHolder()
		{
			for (typename Type::iterator item = data.begin(); item != data.end(); ++item)
				delete (*item).first;
		}

		Type data;
	};

	// интерфейс для обертки поля
	template <typename OwnerType, typename SetterType>
	struct Field
	{
		virtual ~Field() { }
		virtual bool set(OwnerType* _target, typename SetterType::BaseValueType* _value) = 0;
		virtual const std::string& getFieldTypeName() = 0;
	};

	// шаблон для обертки поля
	template <typename OwnerType, typename FieldType, typename SetterType>
	struct FieldHolder : public Field<OwnerType, SetterType>
	{
		FieldHolder(FieldType* OwnerType::* offset) : m_offset(offset) {  }
		FieldType* OwnerType::* const m_offset;

		virtual bool set(OwnerType* _target, typename SetterType::BaseValueType* _value)
		{
			_target->*m_offset = SetterType::template convert<FieldType>(_value);
			return _target->*m_offset != 0;
		}
		virtual const std::string& getFieldTypeName()
		{
			return FieldType::getClassTypeName();
		}
	};

	// шаблон для атрибута поля
	template <typename OwnerType, typename ValueType, typename SetterType>
	struct AttributeField
	{
		typedef std::pair<Field<OwnerType, SetterType>*, ValueType> BindPair;
		typedef std::vector<BindPair> VectorBindPair;

		template <typename FieldType>
		AttributeField(FieldType* OwnerType::* _offset, const ValueType& _value)
		{
			getData().push_back(BindPair(new FieldHolder<OwnerType, FieldType, SetterType>(_offset), _value));
		}
		static VectorBindPair& getData()
		{
			static DataHolder<VectorBindPair> data;
			return data.data;
		}
	};

	// макрос для инстансирования атрибута поля
#define DECLARE_ATTRIBUTE_FIELD(_name, _type, _setter) \
	template <typename OwnerType, typename ValueType = _type, typename SetterType = _setter> \
	struct _name : public attribute::AttributeField<OwnerType, ValueType, SetterType> \
		{ \
		template <typename FieldType> \
		_name(FieldType* OwnerType::* _offset, const ValueType& _value) : \
			AttributeField<OwnerType, ValueType, SetterType>(_offset, _value) { } \
		}

	// макрос для инстансирования экземпляра атрибута
#define ATTRIBUTE_FIELD(_attribute, _class, _field, _value) \
	struct _attribute##_##_field \
		{ \
		_attribute##_##_field() \
			{ \
			static attribute::_attribute<_class> bind(&_class::_field, _value); \
			} \
		} _attribute##_##_field


	// шаблон для атрибута класса
	template <typename Type, typename ValueType>
	struct ClassAttribute
	{
		ClassAttribute(const ValueType& _value)
		{
			getData() = _value;
		}
		static ValueType& getData()
		{
			static ValueType data;
			return data;
		}
	};

	// макрос для инстансирования атрибута класса
#define DECLARE_ATTRIBUTE_CLASS(_name, _type) \
	template <typename Type, typename ValueType = _type> \
	struct _name : public attribute::ClassAttribute<_name<Type>, ValueType> \
		{ \
		_name(const ValueType& _value) : \
			ClassAttribute<_name<Type>, ValueType>(_value) { } \
		}

	// макрос для инстансирования экземпляра класса
#define ATTRIBUTE_CLASS(_attribute, _class, _value) \
	class _class; \
	static attribute::_attribute<_class> _attribute##_##_class(_value)

	// ========
	// WrapsAttribute.h
	// ========

	struct FieldSetterWidget
	{
		typedef MyGUI::Widget BaseValueType;

		template <typename Type>
		static Type* convert(BaseValueType* _value)
		{
			return _value == 0 ? 0 : _value->castType<Type>(false);
		}
	};

	DECLARE_ATTRIBUTE_FIELD(AttributeFieldWidgetName, std::string, FieldSetterWidget);

#define ATTRIBUTE_FIELD_WIDGET_NAME(_class, _field, _value) \
	ATTRIBUTE_FIELD(AttributeFieldWidgetName, _class, _field, _value)


	DECLARE_ATTRIBUTE_CLASS(AttributeSize, MyGUI::IntSize);

#define ATTRIBUTE_CLASS_SIZE(_class, _value) \
	ATTRIBUTE_CLASS(AttributeSize, _class, _value)


	DECLARE_ATTRIBUTE_CLASS(AttributeLayout, std::string);

#define ATTRIBUTE_CLASS_LAYOUT(_class, _value) \
	ATTRIBUTE_CLASS(AttributeLayout, _class, _value)

}

namespace wraps
{

	class BaseLayout
	{
	protected:
		BaseLayout();

		BaseLayout(const std::string& _layout, MyGUI::Widget* _parent = nullptr);

		template <typename T>
		void assignWidget(T * & _widget, const std::string& _name, bool _throw = true, bool _createFakeWidgets = true)
		{
			_widget = nullptr;
			for (MyGUI::VectorWidgetPtr::iterator iter = mListWindowRoot.begin(); iter != mListWindowRoot.end(); ++iter)
			{
				MyGUI::Widget* find = (*iter)->findWidget(mPrefix + _name);
				if (nullptr != find)
				{
					T* cast = find->castType<T>(false);
					if (nullptr != cast)
					{
						_widget = cast;
					}
					else
					{
						MYGUI_LOG(Warning, "Widget with name '" << _name << "' have wrong type ('" <<
							find->getTypeName() << "instead of '" << T::getClassTypeName() << "'). [" << mLayoutName << "]");
						MYGUI_ASSERT( ! _throw, "Can't assign widget with name '" << _name << "'. [" << mLayoutName << "]");
						if (_createFakeWidgets)
							_widget = _createFakeWidget<T>(mMainWidget);
					}

					return;
				}
			}
			MYGUI_LOG(Warning, "Widget with name '" << _name << "' not found. [" << mLayoutName << "]");
			MYGUI_ASSERT( ! _throw, "Can't assign widget with name '" << _name << "'. [" << mLayoutName << "]");
			if (_createFakeWidgets)
				_widget = _createFakeWidget<T>(mMainWidget);
		}

		template <typename T>
		void assignBase(T * & _widget, const std::string& _name, bool _throw = true, bool _createFakeWidgets = true)
		{
			_widget = nullptr;
			for (MyGUI::VectorWidgetPtr::iterator iter = mListWindowRoot.begin(); iter != mListWindowRoot.end(); ++iter)
			{
				MyGUI::Widget* find = (*iter)->findWidget(mPrefix + _name);
				if (nullptr != find)
				{
					_widget = new T(find);
					mListBase.push_back(_widget);
					return;
				}
			}

			MYGUI_LOG(Warning, "Widget with name '" << _name << "' not found. [" << mLayoutName << "]");
			MYGUI_ASSERT( ! _throw, "Can't assign base widget with name '" << _name << "'. [" << mLayoutName << "]");
			if (_createFakeWidgets)
			{
				_widget = new T(_createFakeWidget<MyGUI::Widget>(mMainWidget));
				mListBase.push_back(_widget);
			}
		}

		void initialise(const std::string& _layout, MyGUI::Widget* _parent = nullptr, bool _throw = true, bool _createFakeWidgets = true);

		void shutdown();

		template <typename Type>
		void initialiseByAttributes(Type* _owner, MyGUI::Widget* _parent = nullptr, bool _throw = true, bool _createFakeWidgets = true)
		{
			initialise(attribute::AttributeLayout<Type>::getData(), _parent, _throw, _createFakeWidgets);

			typename attribute::AttributeFieldWidgetName<Type>::VectorBindPair& data = attribute::AttributeFieldWidgetName<Type>::getData();
			for (typename attribute::AttributeFieldWidgetName<Type>::VectorBindPair::iterator item = data.begin(); item != data.end(); ++item)
			{
				MyGUI::Widget* value = nullptr;
				assignWidget(value, item->second, _throw, false);

				bool result = item->first->set(_owner, value);

				if (!result && _createFakeWidgets)
				{
					value = _createFakeWidgetT(item->first->getFieldTypeName(), mMainWidget);
					item->first->set(_owner, value);
				}
			}
		}

	private:
		std::string FindParentPrefix(MyGUI::Widget* _parent);

		void snapToParent(MyGUI::Widget* _child);

		template <typename T>
		T* _createFakeWidget(MyGUI::Widget* _parent)
		{
			return static_cast<T*>(_createFakeWidgetT(T::getClassTypeName(), _parent));
		}

		MyGUI::Widget* _createFakeWidgetT(const std::string& _typeName, MyGUI::Widget* _parent);

	public:
		virtual ~BaseLayout();

	protected:
		MyGUI::Widget* mMainWidget;

	private:
		std::string mPrefix;
		std::string mLayoutName;
		MyGUI::VectorWidgetPtr mListWindowRoot;
		typedef std::vector<BaseLayout*> VectorBasePtr;
		VectorBasePtr mListBase;
	};

} // namespace wraps

#endif // BASE_LAYOUT_H_
