// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define SOL_CHECK_ARGUMENTS 1
#include <sol.hpp>

#include <vector>
#include <iostream>

int main(int, char**) {
	std::cout << "=== containers example ===" << std::endl;

	sol::state lua;
	lua.open_libraries();

	lua.script(R"(
function f (x)
	print("container has:")
	for k=1,#x do
		v = x[k]
		print("\t", k, v)
	end
	print()
end
	)");

	// Have the function we 
	// just defined in Lua
	sol::function f = lua["f"];

	// Set a global variable called 
	// "arr" to be a vector of 5 lements
	lua["arr"] = std::vector<int>{ 2, 4, 6, 8, 10 };

	// Call it, see 5 elements
	// printed out
	f(lua["arr"]);

	// Mess with it in C++
	// Containers are stored as userdata, unless you
	// use `sol::as_table()` and `sol::as_table_t`.
	std::vector<int>& reference_to_arr = lua["arr"];
	reference_to_arr.push_back(12);

	// Call it, see *6* elements
	// printed out
	f(lua["arr"]);

	lua.script(R"(
arr:add(28)
	)");

	// Call it, see *7* elements
	// printed out
	f(lua["arr"]);

	lua.script(R"(
arr:clear()
	)");

	// Now it's empty
	f(lua["arr"]);
	
	std::cout << std::endl;

	return 0;
}