/* Condition.h
Copyright (c) 2022 by an anonymous author.

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CONDITION_H_
#define CONDITION_H_


#include <cmath>
#include <limits>
#include <string>
#include <type_traits>



// This class stores either:
//   1. A condition's value and name
//   2. A literal value (name is empty)
//
// The ValueType (V in the template) is first in the template since
// (nearly?)  all Condition classes will use a std::string name (K). It
// should be an arithmetic type, like double, int64_t, int, or
// unsigned. A bool should work too, but that's untested.
//
// The NameType (K in the template) is a template parameter so we can
// allow storage of scope, wstring, etc. in the future without
// rewriting this class.
//
// Condition assumes its NameType has an empty() function, and that the
// default constructor produces an empty() NameType. Also, the Getter
// must have a HasGet method that receives the NameType. An std::string
// satisfies all of these requirements, and it is the default.

template < class V >
class Condition {
public:
	using ValueType = V;
	using NameType = std::string;

	static_assert(std::is_arithmetic<ValueType>::value, "Condition value type must be arithmetic.");
	static_assert(std::is_class<NameType>::value, "Condition name type must be a class.");
	static_assert(&NameType::empty, "Condition name must have an empty function.");

	constexpr Condition() : value(), name() {}
	explicit constexpr Condition(const V &value) : value(value), name() {}
	constexpr Condition(const V &value, const NameType &name) : value(value), name(name) {}

	template <class V2>
	Condition(const Condition<V2> &other);

	template <class V2>
	Condition &operator=(const Condition<V2> &other);

	template <class T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
	Condition &operator=(const T &t) { value = static_cast<ValueType>(t); return *this; }

	// Update the value from a scope that contains it
	template<class Getter>
	const ValueType &UpdateConditions(const Getter &getter);

	// Update the value from a scope that contains it, but use
	// ValueType() if Validator(weight) is false.
	template<class Getter, class Validator>
	const ValueType &UpdateConditions(const Getter &getter, Validator validator);

	// Accessors and mutators

	ValueType Value() const { return store ? GetFromStore() : value; }
	const NameType &Name() const { return name; }

	// Does this Condition come from the same place as the other one?
	// If it was a condition, the name must be the same (value doesn't matter)
	// If it was a literal (no name) then the value must be the same.
	// If one is literal and the other is conditional, the result is false.
	bool SameOrigin(const Condition &o);

	// Does this originate from a condition?
	bool HasConditions() const { return store; }

	// Floating-point values are false if they're within half the
	// type's precision of 0 while any other types are passed
	// through static_cast<bool>
	explicit operator bool() const;

	// Allow the Condition to be treated as its value in most contexts.
	operator ValueType() const { return value; }



private:
	ValueType GetFromStore() const;
	ValueType SetInStore(ValueType v) const;
	std::shared_ptr<ConditionsStore::ConditionElement> ElementFromStore();


private:
	ValueType value;
	NameType name;

	mutable std::shared_ptr<ConditionsStore> store;
	mutable std::weak_ptr<ConditionsStore::ConditionElement> element;
};


// Allow construction and assignment between Condition types to
// facilitate type conversion.
template <class V>
template <class V2>
Condition<V>::Condition(const Condition<V2> &other):
	value(static_cast<ValueType>(other.Value())),
	name(static_cast<NameType>(other.Name()))
{
}


// Allow construction and assignment between Condition types to
// facilitate type conversion.
template <class V>
template <class V2>
Condition<V> &Condition<V>::operator=(const Condition<V2> &other)
{
	value = static_cast<ValueType>(other.Value());
	name = other.Name();
	return *this;
}


// Update the value from a scope that contains it
template <class V>
template <class Getter>
const V &Condition<V>::UpdateConditions(const Getter &getter)
{
	// If this was a literal, there is nothing to update.
	if(HasConditions())
	{
		auto got = getter.HasGet(name);
		// Assumes: got.first = true iff getter has name
		// got.second = value iff got.first
		if(got.first)
			value = static_cast<ValueType>(got.second);
	}
	return value;
}


// Update the value from a scope that contains it, if the new value passes a validator.
template <class V>
template <class Getter, class Validator>
const V &Condition<V>::UpdateConditions(const Getter &getter, Validator validator)
{
	// If this was a literal, there is nothing to update.
	if(HasConditions())
	{
		auto got = getter.HasGet(name);
		// Assumes: got.first = true iff getter has name
		// got.second = value iff got.first
		if(got.first && validator(got.second))
		{
			value = static_cast<ValueType>(got.second);
			return value;
		}
	}
	if(!validator(value))
		value = ValueType();
	return value;
}


template <class V>
bool Condition<V>::SameOrigin(const Condition<V> &o)
{
	if(HasConditions())
		return name == o.Name();
	else if(o.HasConditions())
		return false;
	else
		return value == o.Value();
}


template < class T, typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr >
bool NotNearZero(T number)
{
	// Use about half the precision of the type when comparing it to zero.
	static const T epsilon = sqrtl(std::numeric_limits<T>::epsilon()*2);
	// Inf and -Inf are NotNearZero but NaN isn't. This is because
	// it is not a number, so it can't be near a number. The
	// consequence is that Condition(NaN) is false in a bool context.
	return number > epsilon || number < -epsilon;
}


template < class T, typename std::enable_if<!std::is_floating_point<T>::value>::type* = nullptr >
bool NotNearZero(T number)
{
	return static_cast<bool>(number);
}


template <class V>
Condition<V>::operator bool() const {
	return NotNearZero(value);
}


template <class V>
V Condition<V>::GetFromStore() const {
	auto it = GetEntryFromStore();
	if(!it)
		return V();
	else if(!it->provider)
		return ArithmeticCast<V>(it->value);
	else
		return ArithmeticCast<V>(it->provider->getFunction(name));
}



template <class V>
V Condition<V>::SetInStore(V old) const
{
	ConditionsStore::ValueType storeValue = ArithmeticCast<V>(old);
	auto it = EnsureEntryInStore();
	*it = storeValue;
	return ArithmeticCast<T>(storeValue);
}


template <class V>
std::shared_ptr<ConditionsStore::ConditionElement> GetEntryFromStore()
{
	try
	{
		return shared_ptr(entry);
	}
	catch(const bad_weak_ptr &bwp)
	{
		auto ptr = store.GetEntry(name);
		entry = ptr;
		return ptr;
	}
}


template <class V>
std::shared_ptr<ConditionsStore::ConditionElement> EnsureEntryInStore()
{
	try
	{
		return shared_ptr(entry);
	}
	catch(const bad_weak_ptr &bwp)
	{
		auto ptr = store.EnsureEntry(name);
		entry = ptr;
		return ptr;
	}
}


#endif
