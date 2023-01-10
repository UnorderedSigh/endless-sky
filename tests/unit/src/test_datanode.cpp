/* test_datanode.cpp
Copyright (c) 2020 by Benjamin Hauch

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
#include "../../../source/DataNode.h"

// Include a helper for creating well-formed DataNodes.
#include "datanode-factory.h"
#include "output-capture.hpp"

// ... and any system includes needed for the test file.
#include <string>
#include <vector>

namespace { // test namespace
// #region mock data

// Insert file-local data here, e.g. classes, structs, or fixtures that will be useful
// to help test this class/method.

// #endregion mock data



// #region unit tests
TEST_CASE( "DataNode Basics", "[DataNode]" ) {
	using T = DataNode;
	SECTION( "Class Traits" ) {
		CHECK_FALSE( std::is_trivial<T>::value );
		// The class layout apparently satisfies StandardLayoutType when building/testing for Steam, but false otherwise.
		// This may change in the future, with the expectation of false everywhere (due to the list<DataNode> field).
		// CHECK_FALSE( std::is_standard_layout<T>::value );
		CHECK( std::is_nothrow_destructible<T>::value );
		CHECK_FALSE( std::is_trivially_destructible<T>::value );
	}
	SECTION( "Construction Traits" ) {
		CHECK( std::is_default_constructible<T>::value );
		CHECK_FALSE( std::is_trivially_default_constructible<T>::value );
		// We explicitly allocate some memory when default-constructing DataNodes.
		CHECK_FALSE( std::is_nothrow_default_constructible<T>::value );
		CHECK( std::is_copy_constructible<T>::value );
		// We have work to do when copy-constructing, including allocations.
		CHECK_FALSE( std::is_trivially_copy_constructible<T>::value );
		CHECK_FALSE( std::is_nothrow_copy_constructible<T>::value );
		CHECK( std::is_move_constructible<T>::value );
		// We have work to do when move-constructing.
		CHECK_FALSE( std::is_trivially_move_constructible<T>::value );
		CHECK( std::is_nothrow_move_constructible<T>::value );
	}
	SECTION( "Copy Traits" ) {
		CHECK( std::is_copy_assignable<T>::value );
		// The class data can be spread out due to vector + list contents.
		CHECK_FALSE( std::is_trivially_copyable<T>::value );
		// We have work to do when copying.
		CHECK_FALSE( std::is_trivially_copy_assignable<T>::value );
		CHECK_FALSE( std::is_nothrow_copy_assignable<T>::value );
	}
	SECTION( "Move Traits" ) {
		CHECK( std::is_move_assignable<T>::value );
		CHECK_FALSE( std::is_trivially_move_assignable<T>::value );
		CHECK( std::is_nothrow_move_assignable<T>::value );
	}
}

SCENARIO( "Creating a DataNode", "[DataNode]") {
	OutputSink traces(std::cerr);
	const DataNode root;
	GIVEN( "When created" ) {
		THEN( "it has the correct default properties" ) {
			CHECK( root.Size() == 0 );
			CHECK_FALSE( root.HasChildren() );
			CHECK( root.Tokens().empty() );
		}
		THEN( "it preallocates capacity for tokens" ) {
			CHECK( root.Tokens().capacity() == 4 );
		}
	}
	GIVEN( "When created without a parent" ) {
		THEN( "it prints its token trace at the correct level" ) {
			CHECK( root.PrintTrace() == 0 );
		}
	}
	GIVEN( "When created with a parent" ) {
		const DataNode child(&root);
		THEN( "it still has the correct default properties" ) {
			CHECK( child.Size() == 0 );
			CHECK_FALSE( child.HasChildren() );
			CHECK( child.Tokens().empty() );
		}
		THEN( "it prints its token trace at the correct level" ) {
			CHECK( child.PrintTrace() == 2 );
		}
		THEN( "no automatic registration with the parent is done" ) {
			CHECK_FALSE( root.HasChildren() );
		}
	}
	GIVEN( "A DataNode with child nodes" ) {
		DataNode parent = AsDataNode("parent\n\tchild\n\t\tgrand");
		WHEN( "Copying by assignment" ) {
			DataNode partner;
			partner = parent;
			parent = DataNode();
			THEN( "the child nodes are copied" ) {
				REQUIRE( partner.HasChildren() );
				const DataNode &child = *partner.begin();
				REQUIRE( child.HasChildren() );
				const DataNode &grand = *child.begin();
				CHECK_FALSE( grand.HasChildren() );

				AND_THEN( "The copied children have the correct tokens" ) {
					REQUIRE( partner.Size() == 1 );
					CHECK( partner.Token(0) == "parent" );
					REQUIRE( child.Size() == 1 );
					CHECK( child.Token(0) == "child" );
					REQUIRE( grand.Size() == 1 );
					CHECK( grand.Token(0) == "grand" );
				}

				AND_THEN( "The copied children print correct traces" ) {
					CHECK( partner.PrintTrace() == 0 );
					CHECK( traces.Flush() == "parent\n" );
					CHECK( child.PrintTrace() == 2 );
					CHECK( traces.Flush() == "parent\nL2:   child\n");
					CHECK( grand.PrintTrace() == 4 );
					CHECK( traces.Flush() == "parent\nL2:   child\nL3:     grand\n");
				}
			}
		}
		WHEN( "Copy-constructing" ) {
			DataNode partner(parent);
			parent = DataNode();
			THEN( "the children are copied" ) {
				REQUIRE( partner.HasChildren() );
				const DataNode &child = *partner.begin();
				REQUIRE( child.HasChildren() );
				const DataNode &grand = *child.begin();
				CHECK_FALSE( grand.HasChildren() );

				AND_THEN( "The copied children have the correct tokens" ) {
					REQUIRE( partner.Size() == 1 );
					CHECK( partner.Token(0) == "parent" );
					REQUIRE( child.Size() == 1 );
					CHECK( child.Token(0) == "child" );
					REQUIRE( grand.Size() == 1 );
					CHECK( grand.Token(0) == "grand" );
				}

				AND_THEN( "The copied children print correct traces" ) {
					CHECK( partner.PrintTrace() == 0 );
					CHECK( traces.Flush() == "parent\n" );
					CHECK( child.PrintTrace() == 2 );
					CHECK( traces.Flush() == "parent\nL2:   child\n");
					CHECK( grand.PrintTrace() == 4 );
					CHECK( traces.Flush() == "parent\nL2:   child\nL3:     grand\n");
				}
			}
		}
		WHEN( "Transferring via move assignment" ) {
			DataNode moved;
			moved = std::move(parent);
			parent = DataNode();
			THEN( "the children are moved" ) {
				REQUIRE( moved.HasChildren() );
				const DataNode &child = *moved.begin();
				REQUIRE( child.HasChildren() );
				const DataNode &grand = *child.begin();
				CHECK_FALSE( grand.HasChildren() );

				AND_THEN( "The moved children have the correct tokens" ) {
					REQUIRE( moved.Size() == 1 );
					CHECK( moved.Token(0) == "parent" );
					REQUIRE( child.Size() == 1 );
					CHECK( child.Token(0) == "child" );
					REQUIRE( grand.Size() == 1 );
					CHECK( grand.Token(0) == "grand" );
				}

				AND_THEN( "The moved children print correct traces" ) {
					CHECK( moved.PrintTrace() == 0 );
					CHECK( traces.Flush() == "parent\n" );
					CHECK( child.PrintTrace() == 2 );
					CHECK( traces.Flush() == "parent\nL2:   child\n");
					CHECK( grand.PrintTrace() == 4 );
					CHECK( traces.Flush() == "parent\nL2:   child\nL3:     grand\n");
				}
			}
		}
		WHEN( "Transferring via move construction" ) {
			DataNode moved(std::move(parent));
			parent = DataNode();
			THEN( "the children are moved" ) {
				REQUIRE( moved.HasChildren() );
				const DataNode &child = *moved.begin();
				REQUIRE( child.HasChildren() );
				const DataNode &grand = *child.begin();
				CHECK_FALSE( grand.HasChildren() );

				AND_THEN( "The moved children have the correct tokens" ) {
					REQUIRE( moved.Size() == 1 );
					CHECK( moved.Token(0) == "parent" );
					REQUIRE( child.Size() == 1 );
					CHECK( child.Token(0) == "child" );
					REQUIRE( grand.Size() == 1 );
					CHECK( grand.Token(0) == "grand" );
				}

				AND_THEN( "The moved children print correct traces" ) {
					CHECK( moved.PrintTrace() == 0 );
					CHECK( traces.Flush() == "parent\n" );
					CHECK( child.PrintTrace() == 2 );
					CHECK( traces.Flush() == "parent\nL2:   child\n");
					CHECK( grand.PrintTrace() == 4 );
					CHECK( traces.Flush() == "parent\nL2:   child\nL3:     grand\n");
				}
			}
		}
	}
}

SCENARIO( "Determining if a token is numeric", "[IsNumber][Parsing][DataNode]" ) {
	GIVEN( "An integer string" ) {
		THEN( "IsNumber returns true" ) {
			auto strNum = GENERATE(as<std::string>{}
				, "1"
				, "10"
				, "100"
				, "1000000000000000"
			);
			CAPTURE( strNum ); // Log the value if the assertion fails.
			CHECK( DataNode::IsNumber(strNum) );
		}
	}
}

SCENARIO( "Checking for keywords", "[CheckForKeywords][Parsing][DataNode]" ) {
	OutputSink traces(std::cerr);
	GIVEN( "a DataNode" ) {
		//                            0     1     2       3     4     5       6     7     8
		DataNode parent = AsDataNode("alpha bravo charlie alpha bravo charlie alpha bravo charlie");
		WHEN( "Querying past the list end" ) {
			THEN( "CheckForKeywords returns false" ) {
				CHECK_FALSE( parent.CheckForKeywords(9, { "alpha", "bravo", "charlie" }) );
			}
		}
		WHEN( "A querying overlaps the list end" ) {
			THEN( "CheckForKeywords returns false" ) {
				CHECK_FALSE( parent.CheckForKeywords(7, { "alpha", "bravo", "charlie" }) );
			}
		}
		WHEN( "A matching querying ends at the end of the list" ) {
			THEN( "CheckForKeywords returns true" ) {
				CHECK( parent.CheckForKeywords(6, { "alpha", "bravo", "charlie" }) );
			}
		}
		WHEN( "Querying a negative index" ) {
			THEN( "CheckForKeywords returns false" ) {
				CHECK_FALSE( parent.CheckForKeywords(-1, { "alpha", "bravo", "charlie" }) );
			}
		}
		WHEN( "A matching querying begins at the beginning of the list" ) {
			THEN( "CheckForKeywords returns true" ) {
				CHECK( parent.CheckForKeywords(0, { "alpha", "bravo", "charlie" }) );
			}
		}
		WHEN( "A matching querying doesn't overlap the beginning or end of the list" ) {
			THEN( "CheckForKeywords returns true" ) {
				CHECK( parent.CheckForKeywords(3, { "alpha", "bravo", "charlie" }) );
			}
		}
		WHEN( "A non-matching querying doesn't overlap the beginning or end of the list" ) {
			THEN( "CheckForKeywords returns false" ) {
				CHECK_FALSE( parent.CheckForKeywords(1, { "alpha", "bravo", "charlie" }) );
				CHECK_FALSE( parent.CheckForKeywords(2, { "alpha", "bravo", "charlie" }) );
				CHECK_FALSE( parent.CheckForKeywords(4, { "alpha", "bravo", "charlie" }) );
				CHECK_FALSE( parent.CheckForKeywords(5, { "alpha", "bravo", "charlie" }) );
			}
		}
	}
}

SCENARIO( "Using ExpectNumber<int>", "[ExpectNumber<int>][Parsing][DataNode]" ) {
	OutputSink traces(std::cerr);
	GIVEN( "a DataNode" ) {
		//                          0     1  2       3       4
		DataNode node = AsDataNode("0     1  charlie 0.25    44");
		const int initial = 999;
		WHEN( "querying past the list end" ) {
			int result = initial;
			THEN( "result does not change" ) {
				CHECK_FALSE( node.ExpectNumber(100, "result", result) );
				CHECK( result == initial );
			}
		}
		WHEN( "querying at the end of the list" ) {
			int result = initial;
			THEN( "the result is equal to the value at the end of the list" ) {
				CHECK( node.ExpectNumber(4, "result", result) );
				CHECK( result == 44 );
			}
		}
		WHEN( "querying a negative index" ) {
			int result = initial;
			THEN( "result does not change" ) {
				CHECK_FALSE( node.ExpectNumber(-100, "result", result) );
				CHECK( result == initial );
			}
		}
		WHEN( "querying at the beginning of the list" ) {
			int result = initial;
			THEN( "result is equal to the beginning of the list" ) {
				CHECK( node.ExpectNumber(0, "result", result) );
				CHECK( result == 0 );
			}
		}
		WHEN( "querying a number that is not at the ends of the list" ) {
			int result = initial;
			THEN( "result is equal to that number" ) {
				CHECK( node.ExpectNumber(1, "result", result) );
				CHECK( result == 1 );
			}
		}
		WHEN( "querying a non-integer number" ) {
			int result = initial;
			THEN( "result is equal to static_cast<int> of that number" ) {
				CHECK( node.ExpectNumber(3, "result", result) );
				CHECK( result == 0 );
			}
		}
		WHEN( "querying a non-number" ) {
			int result = initial;
			THEN( "result does not change" ) {
				CHECK_FALSE( node.ExpectNumber(2, "result", result) );
				CHECK( result == initial );
			}
		}
	}
}
// #endregion unit tests



} // test namespace
