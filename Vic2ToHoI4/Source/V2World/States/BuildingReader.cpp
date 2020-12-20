#include "BuildingReader.h"
#include "ParserHelpers.h"

#include <string>
#include <unordered_set>

std::unordered_set<std::string> ersatzes = {
	 "ersatz_rubber_factory", "ersatz_iron_factory", "charcoal_factory",
	 "ersatz_timber_factory", "ersatz_sulphur_factory",
};

Vic2::BuildingReader::BuildingReader()
{
        registerKeyword("building", [this](const std::string& unused, std::istream& theStream) {
            std::string type = commonItems::singleString(theStream).getString();
            if (ersatzes.contains(type)) {
              ersatzType = type;
            }
        });
	registerKeyword("level", [this](const std::string& unused, std::istream& theStream) {
		level = commonItems::singleInt{theStream}.getInt();
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}


int Vic2::BuildingReader::getLevel(std::istream& theStream)
{
	level = 0;
        ersatzType = "";
	parseStream(theStream);
	return level;
}
