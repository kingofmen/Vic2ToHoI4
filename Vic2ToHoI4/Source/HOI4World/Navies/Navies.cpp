#include "Navies.h"
#include "HOI4World/ProvinceDefinitions.h"
#include "Mappers/Provinces/ProvinceMapper.h"
#include "Log.h"


HoI4::Navies::Navies(const std::vector<std::string>& unitsToCreate,
	 int backupNavalLocation,
	 const UnitMappings& unitMap,
	 const MtgUnitMappings& mtgUnitMap,
	 const ShipVariants& theShipVariants,
	 const std::map<int, int>& provinceToStateIDMap,
	 std::map<int, State> states,
	 const std::string& tag,
	 const ProvinceDefinitions& provinceDefinitions,
	 const mappers::ProvinceMapper& provinceMapper)
{
        int navalLocation = backupNavalLocation;
        int base = backupNavalLocation;
        LegacyNavy newLegacyNavy("Glorious Navy", navalLocation, base);
        MtgNavy newMtgNavy("Glorious Navy", navalLocation, base);
        std::unordered_map<std::string, int> counter;
        for (const auto& unit : unitsToCreate)
        {
                if (unitMap.hasMatchingType(unit))
                {
                        for (const auto& unitInfo :
                             unitMap.getMatchingUnitInfo(unit))
                        {
                                if (unitInfo.getCategory() == "naval" &&
                                    theShipVariants.hasLegacyVariant(
                                        unitInfo.getVersion()))
                                {
                                        LegacyShip newLegacyShip(
                                            "Ship", unitInfo.getType(),
                                            unitInfo.getEquipment(), tag);
                                        newLegacyNavy.addShip(newLegacyShip);
                                }
                        }
                }
                else
                {
                        Log(LogLevel::Warning)
                            << "Unknown legacy unit type: " << unit;
                }
                if (mtgUnitMap.hasMatchingType(unit))
                {
                        counter[unit]++;
                        for (const auto& unitInfo :
                             mtgUnitMap.getMatchingUnitInfo(unit))
                        {
                                if ((unitInfo.getCategory() == "naval") &&
                                    theShipVariants.hasMtgVariant(
                                        unitInfo.getVersion()))
                                {
                                        for (int i = 0; i < unitInfo.getSize();
                                             ++i)
                                        {
                                                Log(LogLevel::Info)
                                                    << "  " << unit << " "
                                                    << counter[unit] << " -> "
                                                    << unitInfo.getType();
                                                MtgShip newMtgShip(
                                                    "Ship", unitInfo.getType(),
                                                    unitInfo.getEquipment(),
                                                    tag, unitInfo.getVersion(),
                                                    0);
                                                newMtgNavy.addShip(newMtgShip);
                                        }
                                }
                        }
                }
                else
                {
                        Log(LogLevel::Warning)
                            << "Unknown mtg unit type: " << unit;
                }
        }
        if (newLegacyNavy.getNumShips() > 0)
        {
                legacyNavies.push_back(newLegacyNavy);
        }
        if (newMtgNavy.getNumShips() > 0)
        {
                mtgNavies.push_back(newMtgNavy);
        }
}

HoI4::Navies::Navies(const std::vector<Vic2::Army>& srcArmies,
	 int backupNavalLocation,
	 const UnitMappings& unitMap,
	 const MtgUnitMappings& mtgUnitMap,
	 const ShipVariants& theShipVariants,
	 const std::map<int, int>& provinceToStateIDMap,
	 std::map<int, State> states,
	 const std::string& tag,
	 const ProvinceDefinitions& provinceDefinitions,
	 const mappers::ProvinceMapper& provinceMapper)
{
	for (auto army: srcArmies)
	{
		auto [navalLocation, base] = getLocationAndBase(army.getLocation(),
			 backupNavalLocation,
			 provinceMapper,
			 provinceDefinitions,
			 provinceToStateIDMap,
			 states);

		LegacyNavy newLegacyNavy(army.getName(), navalLocation, base);
		MtgNavy newMtgNavy(army.getName(), navalLocation, base);

		for (const auto& regiment: army.getUnits())
		{
			auto type = regiment.getType();
			if (unitMap.hasMatchingType(type))
			{
				for (const auto& unitInfo: unitMap.getMatchingUnitInfo(type))
				{
					if (unitInfo.getCategory() == "naval" && theShipVariants.hasLegacyVariant(unitInfo.getVersion()))
					{
						LegacyShip newLegacyShip(regiment.getName(), unitInfo.getType(), unitInfo.getEquipment(), tag);
						newLegacyNavy.addShip(newLegacyShip);
					}
				}
			}
			else
			{
				Log(LogLevel::Warning) << "Unknown legacy unit type: " << type;
			}
			if (mtgUnitMap.hasMatchingType(type))
			{
				for (const auto& unitInfo: mtgUnitMap.getMatchingUnitInfo(type))
				{
					if ((unitInfo.getCategory() == "naval") && theShipVariants.hasMtgVariant(unitInfo.getVersion()))
					{
						auto experience = static_cast<float>(regiment.getExperience() / 100);
						MtgShip newMtgShip(regiment.getName(),
							 unitInfo.getType(),
							 unitInfo.getEquipment(),
							 tag,
							 unitInfo.getVersion(),
							 experience);
						newMtgNavy.addShip(newMtgShip);
					}
				}
			}
			else
			{
				Log(LogLevel::Warning) << "Unknown mtg unit type: " << type;
			}
		}

		if (newLegacyNavy.getNumShips() > 0)
		{
			legacyNavies.push_back(newLegacyNavy);
		}
		if (newMtgNavy.getNumShips() > 0)
		{
			mtgNavies.push_back(newMtgNavy);
		}
	}
}


std::tuple<int, int> HoI4::Navies::getLocationAndBase(std::optional<int> vic2Location,
	 int backupNavalLocation,
	 const mappers::ProvinceMapper& provinceMapper,
	 const ProvinceDefinitions& provinceDefinitions,
	 const std::map<int, int>& provinceToStateIDMap,
	 std::map<int, State> states)
{
	if (vic2Location == std::nullopt)
	{
		return {backupNavalLocation, backupNavalLocation};
	}

	if (auto mapping = provinceMapper.getVic2ToHoI4ProvinceMapping(*vic2Location); mapping)
	{
		for (auto possibleProvince: *mapping)
		{
			if (provinceDefinitions.isSeaProvince(possibleProvince))
			{
				return {possibleProvince, backupNavalLocation};
			}
			if (auto stateID = provinceToStateIDMap.find(possibleProvince); stateID != provinceToStateIDMap.end())
			{
				if (auto state = states.find(stateID->second); state != states.end())
				{
					if (auto mainNavalLocation = state->second.getMainNavalLocation(); mainNavalLocation)
					{
						return {*mainNavalLocation, *mainNavalLocation};
					}
				}
			}
		}
	}

	return {backupNavalLocation, backupNavalLocation};
}
