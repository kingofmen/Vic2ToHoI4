#ifndef PROVINCE_FACTORY_H
#define PROVINCE_FACTORY_H



#include "Parser.h"
#include "Province.h"
#include <memory>



namespace Vic2
{

class RGOFactory: commonItems::parser
{
  public:
        explicit RGOFactory();
        Vic2::Province::RGO getRGO(std::istream& theStream);
  private:
        Province::RGO rgo;
};

class Province::Factory: commonItems::parser
{
  public:
	explicit Factory(std::unique_ptr<Pop::Factory>&& _popFactory);
	std::unique_ptr<Province> getProvince(int number, std::istream& theStream);

  private:
	std::unique_ptr<Province> province;
	std::unique_ptr<Pop::Factory> popFactory;
        std::unique_ptr<RGOFactory> rgoFactory;
};

} // namespace Vic2



#endif // PROVINCE_FACTORY_H
