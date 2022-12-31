/* test_arithmeticcast.cpp
Copyright (c) 2020 by an anonymous author

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "es-test.hpp"

// Include only the tested class's header.
#include "../../../source/ArithmeticCast.h"

#include <cmath>
#include <cstdint>

// ... and any system includes needed for the test file.

namespace { // test namespace
// #region mock data
// #endregion mock data



// #region unit tests
TEST_CASE( "ArithmeticCast Basics", "[ArithmeticCast]" ) {
}

SCENARIO( "Constructing an ArithmeticCast<double>", "[ArithmeticCast][double]" ) {
	typedef double target;
	GIVEN( "a double" ) {
		WHEN( "it is NaN" ) {
			THEN( "the result is NaN" ) {
				CHECK_FALSE( ArithmeticCast<target>(std::numeric_limits<target>::quiet_NaN())
					== ArithmeticCast<target>(std::numeric_limits<target>::quiet_NaN()) );
				CHECK( std::isnan(ArithmeticCast<target>(std::numeric_limits<target>::quiet_NaN())) );
				CHECK_FALSE( std::isfinite(ArithmeticCast<target>(std::numeric_limits<target>::quiet_NaN())) );
				CHECK_FALSE( std::isinf(ArithmeticCast<target>(std::numeric_limits<target>::quiet_NaN())) );
			}
		}
		WHEN( "it is -infinity" ) {
			THEN( "the result is -infinity" ) {
				CHECK( ArithmeticCast<target>(-std::numeric_limits<target>::infinity()) < 0 );
				CHECK_FALSE( std::isfinite(ArithmeticCast<target>(-std::numeric_limits<target>::infinity())) );
				CHECK_FALSE( std::isnan(ArithmeticCast<target>(-std::numeric_limits<target>::infinity())) );
				CHECK( std::isinf(ArithmeticCast<target>(-std::numeric_limits<target>::infinity())) );
			}
		}
		WHEN( "it is infinity" ) {
			THEN( "the result is infinity" ) {
				CHECK( ArithmeticCast<target>(std::numeric_limits<target>::infinity()) > 0 );
				CHECK_FALSE( std::isfinite(ArithmeticCast<target>(std::numeric_limits<target>::infinity())) );
				CHECK_FALSE( std::isnan(ArithmeticCast<target>(std::numeric_limits<target>::infinity())) );
				CHECK( std::isinf(ArithmeticCast<target>(std::numeric_limits<target>::infinity())) );
			}
		}
		WHEN( "it is finite" ) {
			THEN( "the result is identical" ) {
				target value = 1.31313e19;
				CHECK( ArithmeticCast<target>(value) == value );
			}
		}
	}
	GIVEN( "an int64_t" ) {
		typedef int64_t source;
		WHEN( "it is within range of a double" ) {
			THEN( "the representation is exact" ) {
				source from = -4503599627370495ll; // -2**52+1
				CHECK( static_cast<source>(ArithmeticCast<target>(from)) == from);
			}
		}
		WHEN( "it is numeric_limits<int64_t>::max()" ) {
			THEN( "the result is above 2**62" ) {
				target big = std::pow(2,62);
				source from = std::numeric_limits<source>::max();
				CHECK( ArithmeticCast<target>(from) > big );
			}
		}
		WHEN( "it is numeric_limits<int64_t>::min()" ) {
			THEN( "the result is below -2**62" ) {
				target neg = -std::pow(2,62);
				source from = std::numeric_limits<source>::min();
				CHECK( ArithmeticCast<target>(from) < neg );
			}
		}
	}
}

SCENARIO( "Constructing an ArithmeticCast<bool>", "[ArithmeticCast][bool]" ) {
	typedef bool target;
	GIVEN( "a bool" ) {
		THEN( "the result is unchanged" ) {
			CHECK( ArithmeticCast<target>(true) );
			CHECK_FALSE( ArithmeticCast<target>(false) );
		}
	}
	GIVEN( "an int64_t" ) {
		typedef int64_t source;
		WHEN( "it is zero" ) {
			THEN( "the result is false" ) {
				CHECK_FALSE( ArithmeticCast<target>(static_cast<source>(0)) );
			}
		}
		WHEN( "it is positive" ) {
			THEN( "the result is true" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(1)) );
				CHECK( ArithmeticCast<target>(std::numeric_limits<source>::max()) );
			}
		}
		WHEN( "it is negative" ) {
			THEN( "the result is true" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(1)) );
				CHECK( ArithmeticCast<target>(std::numeric_limits<source>::min()) );
			}
		}
	}
	GIVEN( "a double" ) {
		typedef double source;
		WHEN( "it is NaN" ) {
			THEN( "the result is false" ) {
				CHECK_FALSE( ArithmeticCast<target>(std::numeric_limits<source>::quiet_NaN()) );
			}
		}
		WHEN( "it is -infinity" ) {
			THEN( "the result is true" ) {
				CHECK( ArithmeticCast<target>(-std::numeric_limits<source>::infinity()) );
			}
		}
		WHEN( "it is infinity" ) {
			THEN( "the result is true" ) {
				CHECK( ArithmeticCast<target>(std::numeric_limits<source>::infinity()) );
			}
		}
		WHEN( "it is identically zero" ) {
			THEN( "the result is false" ) {
				CHECK_FALSE( ArithmeticCast<target>(source(0)) );
			}
		}
		WHEN( "it is the lowest possible representable value" ) {
			THEN( "the result is true" ) {
				CHECK( ArithmeticCast<target>(std::numeric_limits<source>::lowest()) );
			}
		}
		WHEN( "it is the minimum representable finite value" ) {
			THEN( "the result is false" ) {
				CHECK_FALSE( ArithmeticCast<target>(std::numeric_limits<source>::min()) );
			}
		}
		WHEN( "it is the maximum representable finite value" ) {
			THEN( "the result is true" ) {
				CHECK( ArithmeticCast<target>(std::numeric_limits<source>::max()) );
			}
		}
		WHEN( "it is the smallest possible positive subnormal value" ) {
			THEN( "the result is false" ) {
				CHECK_FALSE( ArithmeticCast<target>(std::numeric_limits<source>::denorm_min()) );
			}
		}
		WHEN( "it is epsilon" ) {
			THEN( "the result is false" ) {
				CHECK_FALSE( ArithmeticCast<target>(std::numeric_limits<source>::epsilon()) );
			}
		}
		WHEN( "it is -epsilon" ) {
			THEN( "the result is false" ) {
				CHECK_FALSE( ArithmeticCast<target>(-std::numeric_limits<source>::epsilon()) );
			}
		}
		WHEN( "it is 0.01" ) {
			THEN( "the result is true" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(0.01)) );
			}
		}
		WHEN( "it is -0.01" ) {
			THEN( "the result is true" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(-0.01)) );
			}
		}
	}
}

SCENARIO( "Constructing an ArithmeticCast<int32_t>", "[ArithmeticCast][uint32_t]" ) {
	typedef int32_t target;
	GIVEN( "an int32_t" ) {
		THEN( "the result is exact" ) {
			CHECK( ArithmeticCast<target>(static_cast<target>(0))
				== static_cast<target>(0) );
			CHECK( ArithmeticCast<target>(std::numeric_limits<target>::max())
				== std::numeric_limits<target>::max() );
			CHECK( ArithmeticCast<target>(std::numeric_limits<target>::min())
				== std::numeric_limits<target>::min() );
		}
	}
	GIVEN( "a double" ) {
		typedef double source;
		WHEN( "it is NaN" ) {
			THEN( "the result is 0" ) {
				CHECK( ArithmeticCast<target>(std::numeric_limits<source>::quiet_NaN())
					== static_cast<target>(0) );
			}
		}
		WHEN( "it is -infinity" ) {
			THEN( "the result is numeric_limits<int32_t>::min()" ) {
				CHECK( ArithmeticCast<target>(-std::numeric_limits<source>::infinity())
					== static_cast<target>(std::numeric_limits<int32_t>::min()) );
			}
		}
		WHEN( "it is infinity" ) {
			THEN( "the result is numeric_limits<int32_t>::max()" ) {
				CHECK( ArithmeticCast<target>(std::numeric_limits<source>::infinity())
					== static_cast<target>(std::numeric_limits<int32_t>::max()) );
			}
		}
		WHEN( "it is negative and beyond the range of an int32_t" ) {
			THEN( "the result is numeric_limits<int32_t>::min()" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(-9e20))
					== static_cast<target>(std::numeric_limits<int32_t>::min()) );
			}
		}
		WHEN( "it is positive and beyond the range of an int32_t" ) {
			THEN( "the result is numeric_limits<int32_t>::max()" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(9e20))
					== static_cast<target>(std::numeric_limits<int32_t>::max()) );
			}
		}
		WHEN( "it is positive, within the limits of an int32_t, and within the precision of a double" ) {
			THEN( "the result is exact" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<int32_t>::max()))
					== static_cast<target>(std::numeric_limits<int32_t>::max()) );
			}
		}
		WHEN( "it is negative, within the limits of an int32_t, and within the precision of a double" ) {
			THEN( "the result is exact" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(-1))
					== static_cast<target>(-1) );
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<int32_t>::min()))
					== static_cast<target>(std::numeric_limits<int32_t>::min()) );
			}
		}
	}

	GIVEN( "an int64_t" ) {
		typedef int64_t source;
		WHEN( "it is negative and within the range of int32_t" ) {
			THEN( "the representation is exact" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(-1))
					== static_cast<target>(-1) );
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<target>::min()))
					== static_cast<target>(std::numeric_limits<target>::min()) );
			}
		}
		WHEN( "it is positive and within the range of int32_t" ) {
			THEN( "the representation is exact" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(1))
					== static_cast<target>(1) );
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<target>::max()))
					== static_cast<target>(std::numeric_limits<target>::max()) );
			}
		}
		WHEN( "it is negative and beyond the range of an int32_t" ) {
			THEN( "the result is numeric_limits<int32_t>::min" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<source>::min()))
					== static_cast<target>(std::numeric_limits<target>::min()) );
			}
		}
		WHEN( "it is positive and beyond the range of an int32_t" ) {
			THEN( "the result is numeric_limits<int32_t>::max" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<source>::max()))
					== static_cast<target>(std::numeric_limits<target>::max()) );
			}
		}
	}

	GIVEN( "a uint32_t" ) {
		typedef uint32_t source;
		WHEN( "it is within the range of int32_t" ) {
			THEN( "the representation is exact" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<target>::max()))
					== static_cast<target>(std::numeric_limits<target>::max()) );
			}
		}
		WHEN( "it is beyond the range of an int32_t" ) {
			THEN( "the result is numeric_limits<int32_t>::max" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<source>::max()))
					== static_cast<target>(std::numeric_limits<target>::max()) );
			}
		}
	}

	GIVEN( "a uint16_t" ) {
		typedef uint16_t source;
		THEN( "the representation is exact" ) {
			CHECK( ArithmeticCast<target>(static_cast<source>(0))
				== static_cast<target>(0) );
			CHECK( ArithmeticCast<target>(static_cast<source>(1))
				== static_cast<target>(1) );
			CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<source>::max()))
				== static_cast<target>(std::numeric_limits<source>::max()));
		}
	}
}

SCENARIO( "Constructing an ArithmeticCast<uint32_t>", "[ArithmeticCast][uint32_t]" ) {
	typedef uint32_t target;
	GIVEN( "a uint32_t" ) {
		THEN( "the result is exact" ) {
			CHECK( ArithmeticCast<target>(static_cast<target>(0))
				== static_cast<target>(0) );
			CHECK( ArithmeticCast<target>(static_cast<target>(1))
				== static_cast<target>(1) );
			CHECK( ArithmeticCast<target>(std::numeric_limits<target>::max())
				== std::numeric_limits<target>::max() );
		}
	}
	GIVEN( "a double" ) {
		typedef double source;
		WHEN( "it is NaN" ) {
			THEN( "the result is 0" ) {
				CHECK( ArithmeticCast<target>(std::numeric_limits<source>::quiet_NaN())
					== static_cast<target>(0) );
			}
		}
		WHEN( "it is -infinity" ) {
			THEN( "the result is 0" ) {
				CHECK( ArithmeticCast<target>(-std::numeric_limits<source>::infinity())
					== static_cast<target>(0) );
			}
		}
		WHEN( "it is infinity" ) {
			THEN( "the result is numeric_limits<uint32_t>::max" ) {
				CHECK( ArithmeticCast<target>(std::numeric_limits<source>::infinity())
					== static_cast<target>(std::numeric_limits<target>::max()) );
			}
		}
		WHEN( "it is negative and finite" ) {
			THEN( "the result is 0" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(-9e20))
					== static_cast<target>(0) );
			}
		}
		WHEN( "it is positive and beyond the range of a uint32_t" ) {
			THEN( "the result is numeric_limits<uint32_t>::max" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(9e20))
					== static_cast<target>(std::numeric_limits<uint32_t>::max()) );
			}
		}
		WHEN( "it is positive, within the limits of a uint32_t, and within the precision of a double" ) {
			THEN( "the result is exact" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<uint32_t>::max()))
					== static_cast<target>(std::numeric_limits<uint32_t>::max()) );
			}
		}
	}

	GIVEN( "an int64_t" ) {
		typedef int64_t source;
		WHEN( "it is negative" ) {
			THEN( "the result is 0" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(-1))
					== static_cast<target>(0) );
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<source>::min()))
					== static_cast<target>(0) );
			}
		}
		WHEN( "it is positive and within the range of a uint32_t" ) {
			THEN( "the representation is exact" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<target>::max()))
					== static_cast<target>(std::numeric_limits<target>::max()) );
			}
		}
		WHEN( "it is positive and beyond the range of a uint32_t" ) {
			THEN( "the result is numeric_limits<uint32_t>::max" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<source>::max()))
					== static_cast<target>(std::numeric_limits<target>::max()) );
			}
		}
	}

	GIVEN( "an int16_t" ) {
		typedef int64_t source;
		WHEN( "it is negative" ) {
			THEN( "the result is 0" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(-1))
					== static_cast<target>(0) );
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<source>::min()))
					== static_cast<target>(0) );
			}
		}
		WHEN( "it is positive" ) {
			THEN( "the representation is exact" ) {
				CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<source>::max()))
					== static_cast<target>(std::numeric_limits<source>::max()) );
			}
		}
	}

	GIVEN( "a uint16_t" ) {
		typedef uint16_t source;
		THEN( "the representation is exact" ) {
			CHECK( ArithmeticCast<target>(static_cast<source>(0))
				== static_cast<target>(0) );
			CHECK( ArithmeticCast<target>(static_cast<source>(1))
				== static_cast<target>(1) );
			CHECK( ArithmeticCast<target>(static_cast<source>(std::numeric_limits<source>::max()))
				== static_cast<target>(std::numeric_limits<source>::max()) );
		}
	}
}
// #endregion unit tests



} // test namespace
