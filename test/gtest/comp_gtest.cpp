#include <gtest/gtest.h>

#include "comp.h"
#include <unordered_map>
#include <iostream>
#include <filesystem>


void on_frame(Comp comp, int frame)
{
	std::cout << "Frame: " << frame << std::endl;

	++frame;
}


// need to do a bit of work to test this properly

TEST(CompTest, Test1) 
{
	std::filesystem::path projectDir(CMAKE_PROJECT_DIR);
	std::filesystem::path compPath = projectDir / "example" / "TopChopDatIO.tox";

	//std::cout << "Absolute file path: " << compPath.string() << std::endl;

	//Comp comp{ compPath.string()};
	//std::shared_ptr<std::unordered_map<std::string, int>> info = std::make_shared<std::unordered_map<std::string, int>>();
	//info->insert({ "frame", 0 });

	//comp.runUpdateLoop(true);

}