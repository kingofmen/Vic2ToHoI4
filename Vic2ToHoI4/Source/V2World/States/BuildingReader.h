#ifndef VIC2_BUILDING_READER_H
#define VIC2_BUILDING_READER_H



#include "Parser.h"
#include <string>


namespace Vic2
{

class BuildingReader: commonItems::parser
{
  public:
	BuildingReader();

	int getLevel(std::istream& theStream);
        const std::string& ersatz() const { return ersatzType; }

  private:
	int level = 0;
        std::string ersatzType;
};

} // namespace Vic2



#endif // VIC2_BUILDING_READER_H
