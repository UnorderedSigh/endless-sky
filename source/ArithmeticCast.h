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

#ifndef ARITHMETICCAST_H_
#define ARITHMETICCAST_H_


#include <limits>
#include <type_traits>



// A type trait indicating that A can represent bigger positive numbers than B.
// Most of the complexity is necessitated by g++ -pedantic-errors
template <typename A, typename B>
struct HigherMaxValue
{
	static constexpr bool value =
		std::is_floating_point<A>::value ?
			(
				// Floating-point values have a higher max value than integers
				!std::is_floating_point<B>::value
				// Larger floating-point values fit a higher max value
				|| sizeof(A) > sizeof(B)
			)
		:
			// Type A is an integer.

			// If B is a floating-point value, then A has a lower max value.
			!std::is_floating_point<B>::value

			// If we get here, then we handle integer vs. integer.
			&& (
				// Integers with more bits store higher max values
				sizeof(A) > sizeof(B)
				|| (
					// Unsigned integers have a higher max value.
					sizeof(A) == sizeof(B)
					&& std::is_unsigned<A>::value
					&& !std::is_unsigned<B>::value
				)
			);
};

template <class A>
struct HigherMaxValue<A, bool>
{
	static constexpr bool value = true;
};

template <class B>
struct HigherMaxValue<bool, B>
{
	static constexpr bool value = false;
};

template <>
struct HigherMaxValue<bool, bool>
{
	static constexpr bool value = false;
};


// Detect bool in templates without needing complicated specialization magic
template<class T>
struct IsBool
{
	static const bool value = false;
};

template<>
struct IsBool<bool>
{
	static const bool value = true;
};


// Cast to unsigned from floating-point
template <typename To, typename From,
	typename std::enable_if<!IsBool<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_unsigned<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_floating_point<From>::value>::type* = nullptr
>
To ArithmeticCast(From from) noexcept
{
	static const From max = static_cast<From>(std::numeric_limits<To>::max());
	if(from < 0)
		return 0;
	else if(!(from < max))
	{
		if(from == from)
			return std::numeric_limits<To>::max(); // infinity
		else
			return 0; // NaN
	}
	return from;
}


// Cast to unsigned from larger unsigned
template <typename To, typename From,
	typename std::enable_if<!IsBool<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_unsigned<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_unsigned<From>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<From>::value>::type* = nullptr,
	typename std::enable_if<HigherMaxValue<From, To>::value>::type* = nullptr
>
To ArithmeticCast(From from) noexcept
{
	if(from <= static_cast<From>(std::numeric_limits<To>::max()))
		return static_cast<To>(from);
	else
		return std::numeric_limits<To>::max();
}

// Cast to unsigned from an unsigned of same size or less
template <typename To, typename From,
	typename std::enable_if<!IsBool<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_unsigned<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_unsigned<From>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<From>::value>::type* = nullptr,
	typename std::enable_if<!HigherMaxValue<From, To>::value>::type* = nullptr
>
To ArithmeticCast(From from) noexcept
{
	return static_cast<To>(from);
}


// Cast to unsigned from signed of same size or less
template <typename To, typename From,
	typename std::enable_if<!IsBool<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_unsigned<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_unsigned<From>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<From>::value>::type* = nullptr,
	typename std::enable_if<!HigherMaxValue<From, To>::value>::type* = nullptr
>
To ArithmeticCast(From from) noexcept
{
	if(from >= 0)
		return static_cast<To>(from);
	else
		return 0;
}


// Cast to unsigned from a larger signed integer
template <typename To, typename From,
	typename std::enable_if<!IsBool<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_unsigned<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<From>::value>::type* = nullptr,
	typename std::enable_if<!std::is_unsigned<From>::value>::type* = nullptr,
	typename std::enable_if<std::is_arithmetic<From>::value>::type* = nullptr,
	typename std::enable_if<HigherMaxValue<From, To>::value>::type* = nullptr
>
To ArithmeticCast(From from) noexcept
{
	static const From max = static_cast<From>(std::numeric_limits<To>::max());
	if(from < 0)
		return 0;
	else if(from > max)
		return std::numeric_limits<To>::max();
	else
		return static_cast<To>(from);
}


// Cast to signed integer from floating-point
template <typename To, typename From,
	typename std::enable_if<!IsBool<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_unsigned<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_arithmetic<From>::value>::type* = nullptr,
	typename std::enable_if<std::is_floating_point<From>::value>::type* = nullptr
>
To ArithmeticCast(From from) noexcept
{
	static const From max = static_cast<From>(std::numeric_limits<To>::max());
	static const From min = static_cast<From>(std::numeric_limits<To>::min());
	if(from > max)
		return std::numeric_limits<To>::max(); // infinity or out-of-bounds
	else if(from < min)
		return std::numeric_limits<To>::min(); // -infinity or negative out-of-bounds
	else if(from == from)
		return from;
	else
		return 0; // NaN
}


// Cast to signed integer from larger signed integer
template <typename To, typename From,
	typename std::enable_if<!IsBool<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_unsigned<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_arithmetic<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<From>::value>::type* = nullptr,
	typename std::enable_if<!std::is_unsigned<From>::value>::type* = nullptr,
	typename std::enable_if<std::is_arithmetic<From>::value>::type* = nullptr,
	typename std::enable_if<HigherMaxValue<From, To>::value>::type* = nullptr
>
To ArithmeticCast(From from) noexcept
{
	static const From max = static_cast<From>(std::numeric_limits<To>::max());
	static const From min = static_cast<From>(std::numeric_limits<To>::min());
	if(from > max)
		return std::numeric_limits<To>::max();
	else if(from < min)
		return std::numeric_limits<To>::min();
	else
		return static_cast<To>(from);
}


// Cast to signed integer from an unsigned integer of same size or larger.
template <typename To, typename From,
	typename std::enable_if<!IsBool<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_unsigned<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_arithmetic<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<From>::value>::type* = nullptr,
	typename std::enable_if<std::is_unsigned<From>::value>::type* = nullptr,
	typename std::enable_if<std::is_arithmetic<From>::value>::type* = nullptr,
	typename std::enable_if<HigherMaxValue<From, To>::value>::type* = nullptr
>
To ArithmeticCast(From from) noexcept
{
	static const From max = static_cast<From>(std::numeric_limits<To>::max());
	if(from > max)
		return std::numeric_limits<To>::max();
	else
		return static_cast<To>(from);
}


// Cast to signed integer from smaller integer
template <typename To, typename From,
	typename std::enable_if<!IsBool<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_unsigned<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_arithmetic<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<From>::value>::type* = nullptr,
	typename std::enable_if<std::is_arithmetic<From>::value>::type* = nullptr,
	typename std::enable_if<!HigherMaxValue<From, To>::value>::type* = nullptr
>
To ArithmeticCast(From from) noexcept
{
	return static_cast<To>(from);
}


// Cast to bool from floating-point
template <typename To, typename From,
	typename std::enable_if<IsBool<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_floating_point<From>::value>::type* = nullptr
>
To ArithmeticCast(From from) noexcept
{
	// Use about half the precision of the type when comparing it to zero.
	static const From epsilon = sqrtl(std::numeric_limits<From>::epsilon()*2);
	return from > epsilon || from < -epsilon;
	// NaN is false because it compares as false to everything, including itself.
}


// Cast to bool from integer
template <typename To, typename From,
	typename std::enable_if<IsBool<To>::value>::type* = nullptr,
	typename std::enable_if<!std::is_floating_point<From>::value>::type* = nullptr,
	typename std::enable_if<std::is_arithmetic<From>::value>::type* = nullptr
>
To ArithmeticCast(From from) noexcept
{
	return static_cast<To>(from);
}


// Cast to floating-point from any arithmetic type without any special handling
template <typename To, typename From,
	typename std::enable_if<std::is_floating_point<To>::value>::type* = nullptr,
	typename std::enable_if<std::is_arithmetic<From>::value>::type* = nullptr
>
To ArithmeticCast(From from) noexcept
{
	return static_cast<To>(from);
}


#endif
