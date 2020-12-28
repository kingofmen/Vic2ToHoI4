#include "HoI4State.h"
#include "HOI4World/Map/CoastalProvinces.h"
#include "Log.h"
#include "StateCategories.h"
#include "V2World/Provinces/Province.h"
#include "V2World/States/State.h"
#include <random>
#include <unordered_map>
#include <unordered_set>


constexpr int POP_CONVERSION_FACTOR = 4;
constexpr int POPULATION_PER_STATE_SLOT = 120000;
constexpr int FIRST_INFRASTRUCTURE_REWARD_LEVEL = 4;
constexpr int SECOND_INFRASTRUCTURE_REWARD_LEVEL = 6;
constexpr int THIRD_INFRASTRUCTURE_REWARD_LEVEL = 10;



HoI4::State::State(const Vic2::State& sourceState, int _ID, const std::string& _ownerTag): ID(_ID), ownerTag(_ownerTag)
{
	population = sourceState.getPopulation();
	employedWorkers = sourceState.getEmployedWorkers();

	if (ownerTag.empty())
	{
		infrastructure = 0;
	}
	addInfrastructureFromRails(sourceState.getAverageRailLevel());
        for (const auto& extra : sourceState.getExtraResources()) {
                addResource(extra.first, extra.second);
                Log(LogLevel::Info)
                    << "Adding " << extra.second << " " << extra.first
                    << " to state " << ID << " owned by " << ownerTag << " ("
                    << sourceState.getOwner() << ")";
        }

        // This should really be done while parsing the Victoria save,
        // just generating a "resource in province X" map.
        struct resource {
          double weight;
          std::string good;
        };
        static std::unordered_map<std::string, resource> resourceMap{
            {"iron", {1.0, "steel"}},
            {"grain", {0.1, "steel"}},
            {"wool", {0.1, "steel"}},
            {"fish", {0.1, "steel"}},
            {"coal", {1.0, "aluminium"}},
            {"fruit", {0.1, "aluminium"}},
            {"tea", {0.2, "aluminium"}},
            {"coffee", {0.2, "aluminium"}},
            {"sulphur", {1.0, "chromium"}},
            {"cotton", {0.1, "chromium"}},
            {"timber", {1.0, "tungsten"}},
            {"dye", {0.2, "tungsten"}},
            {"precious_metal", {0.2, "tungsten"}},
            {"tobacco", {0.2, "tungsten"}},
            {"cattle", {0.1, "tungsten"}},
            {"oil", {1.0, "oil"}},
            {"silk", {0.2, "oil"}},
            {"rubber", {1.0, "rubber"}},
            {"opium", {0.2, "rubber"}},
            {"tropical_wood", {0.2, "rubber"}},
        };

        static std::unordered_set<std::string> seen;
        for (const auto& prov : sourceState.getProvinces())
        {
                const auto& rgo = prov->getRgoType();
                if (resourceMap.find(rgo) == resourceMap.end())
                {
                        if (seen.find(rgo) == seen.end())
                        {
                                Log(LogLevel::Warning)
                                    << "Unknown V2 resource " << rgo
                                    << " will not map to HoI4 "
                                       "resources.";
                                seen.insert(rgo);
                        }
                        continue;
                }
                const auto& hoi = resourceMap.at(prov->getRgoType());
                double amount = prov->getRgoEmployees();
                if (amount < 1000)
                {
                        amount = 1000;
                }
                amount *= hoi.weight;
                weightMap[hoi.good] += amount;
        }
}


void HoI4::State::convertNavalBases(const std::map<int, int>& sourceNavalBases,
	 const CoastalProvinces& theCoastalProvinces,
	 const mappers::ProvinceMapper& theProvinceMapper)
{
	for (const auto& sourceNavalBase: sourceNavalBases)
	{
		int navalBaseLevel = determineNavalBaseLevel(sourceNavalBase.second);
		if (navalBaseLevel == 0)
		{
			continue;
		}

		auto navalBaseLocation =
			 determineNavalBaseLocation(sourceNavalBase.first, theCoastalProvinces, theProvinceMapper);
		if (navalBaseLocation)
		{
			addNavalBase(navalBaseLevel, *navalBaseLocation);
		}
	}
}


int HoI4::State::determineNavalBaseLevel(int sourceLevel)
{
	int navalBaseLevel = sourceLevel * 2;
	if (navalBaseLevel > 10)
	{
		navalBaseLevel = 10;
	}

	return navalBaseLevel;
}


std::optional<int> HoI4::State::determineNavalBaseLocation(int sourceProvince,
	 const CoastalProvinces& theCoastalProvinces,
	 const mappers::ProvinceMapper& theProvinceMapper)
{
	if (auto mapping = theProvinceMapper.getVic2ToHoI4ProvinceMapping(sourceProvince))
	{
		for (auto HoI4ProvNum: *mapping)
		{
			if (theCoastalProvinces.isProvinceCoastal(HoI4ProvNum))
			{
				return HoI4ProvNum;
			}
		}
	}

	return {};
}


void HoI4::State::addNavalBase(int level, int location)
{
	if ((level > 0) && provinces.contains(location))
	{
		navalBases[location] = level;
	}
}


void HoI4::State::addCores(const std::set<std::string>& newCores)
{
	for (auto newCore: newCores)
	{
		cores.insert(newCore);
	}
}


void HoI4::State::convertControlledProvinces(const std::vector<std::pair<int, std::string>>& foreignControlledProvinces,
	 const mappers::ProvinceMapper& theProvinceMapper,
	 const CountryMapper& countryMapper)
{
	for (const auto& foreignControlledProvince: foreignControlledProvinces)
	{
		auto possibleController = countryMapper.getHoI4Tag(foreignControlledProvince.second);
		if ((!possibleController) || (*possibleController == "REB"))
		{
			continue;
		}

		auto provinceMapping = theProvinceMapper.getVic2ToHoI4ProvinceMapping(foreignControlledProvince.first);
		if (provinceMapping)
		{
			for (auto destinationProvince: *provinceMapping)
			{
				if (!provinces.contains(destinationProvince))
				{
					continue;
				}

				if (!controlledProvinces.contains(*possibleController))
				{
					std::set<int> destinationProvinces;
					destinationProvinces.insert(destinationProvince);
					controlledProvinces.insert(std::make_pair(*possibleController, destinationProvinces));
				}
				else
				{
					controlledProvinces.find(*possibleController)->second.insert(destinationProvince);
				}
			}
		}
	}
}


void HoI4::State::setControlledProvince(int provinceNum, const std::string& country)
{
	if (country == ownerTag)
	{
		return;
	}

	auto [existing, inserted] = controlledProvinces.insert(std::make_pair(country, std::set<int>{provinceNum}));
	if (!inserted)
	{
		existing->second.insert(provinceNum);
	}
}


bool HoI4::State::assignVPFromVic2Province(int Vic2ProvinceNumber, const mappers::ProvinceMapper& theProvinceMapper)
{
	if (auto mapping = theProvinceMapper.getVic2ToHoI4ProvinceMapping(Vic2ProvinceNumber))
	{
		for (auto province: *mapping)
		{
			if (isProvinceInState(province))
			{
				assignVP(province);
				return true;
			}
		}
	}

	return false;
}


void HoI4::State::assignVP(int location)
{
	victoryPointPosition = location;

	victoryPointValue = 1;
	if (cores.contains(ownerTag))
	{
		victoryPointValue += 2;
	}
}


std::optional<int> HoI4::State::getMainNavalLocation() const
{
	std::optional<int> mainLocation;
	int mainSize = 0;
	for (const auto& navalBase: navalBases)
	{
		if (navalBase.second > mainSize)
		{
			mainLocation = navalBase.first;
			mainSize = navalBase.second;
		}
	}

	return mainLocation;
}


int HoI4::State::getManpower() const
{
	if (manpower == 0)
	{
		return 1;
	}

	return manpower;
}



void HoI4::State::tryToCreateVP(const Vic2::State& sourceState,
	 const mappers::ProvinceMapper& theProvinceMapper,
	 const Configuration& theConfiguration)
{
	bool VPCreated = false;

	auto vic2CapitalProvince = sourceState.getCapitalProvince();
	if (vic2CapitalProvince)
	{
		VPCreated = assignVPFromVic2Province(*vic2CapitalProvince, theProvinceMapper);
	}

	if (!VPCreated)
	{
		if (theConfiguration.getDebug() && !sourceState.isPartialState() && !impassable && !hadImpassablePart)
		{
			Log(LogLevel::Warning) << "Could not initially create VP for state " << ID << ", but state is not split.";
		}
		auto vic2UpperClassProvince = sourceState.getUpperClassLocation();
		if (vic2UpperClassProvince)
		{
			VPCreated = assignVPFromVic2Province(*vic2UpperClassProvince, theProvinceMapper);
		}
	}

	if (!VPCreated)
	{
		for (const auto& province: sourceState.getProvincesOrderedByPopulation())
		{
			VPCreated = assignVPFromVic2Province(province, theProvinceMapper);
			if (VPCreated)
			{
				break;
			}
		}
	}

	if (!VPCreated)
	{
		Log(LogLevel::Warning) << "Could not create VP for state " << ID;
	}

	addDebugVPs(sourceState, theProvinceMapper);
}


void HoI4::State::addDebugVPs(const Vic2::State& sourceState, const mappers::ProvinceMapper& theProvinceMapper)
{
	for (auto sourceProvinceNum: sourceState.getProvinceNumbers())
	{
		auto mapping = theProvinceMapper.getVic2ToHoI4ProvinceMapping(sourceProvinceNum);
		if (mapping && (isProvinceInState((*mapping)[0])))
		{
			debugVictoryPoints.insert((*mapping)[0]);
		}
		for (auto province: *mapping)
		{
			secondaryDebugVictoryPoints.insert(province);
		}
	}
}


void HoI4::State::addManpower(const std::set<std::shared_ptr<Vic2::Province>>& sourceProvinces,
	 const mappers::ProvinceMapper& theProvinceMapper,
	 const Configuration& theConfiguration)
{
	for (auto sourceProvince: sourceProvinces)
	{
		int numProvincesInState = 0;
		int numProvincesInMapping = 0;
		if (auto mapping = theProvinceMapper.getVic2ToHoI4ProvinceMapping(sourceProvince->getNumber()))
		{
			numProvincesInMapping = static_cast<int>(mapping->size());
			for (auto HoI4Province: *mapping)
			{
				if (isProvinceInState(HoI4Province))
				{
					numProvincesInState++;
				}
			}
		}

		if (numProvincesInMapping && numProvincesInState)
		{
			manpower +=
				 static_cast<int>(sourceProvince->getTotalPopulation() * POP_CONVERSION_FACTOR *
										theConfiguration.getManpowerFactor() * numProvincesInState / numProvincesInMapping);
		}
	}
}


void HoI4::State::convertIndustry(double workerFactoryRatio,
	 const HoI4::StateCategories& theStateCategories,
	 const CoastalProvinces& theCoastalProvinces)
{
	int factories = determineFactoryNumbers(workerFactoryRatio);

	determineCategory(factories, theStateCategories);
	addInfrastructureFromFactories(factories);
	setIndustry(factories, theCoastalProvinces);
	addVictoryPointValue(factories / 2);
}


int HoI4::State::determineFactoryNumbers(double workerFactoryRatio) const
{
	double rawFactories = employedWorkers * workerFactoryRatio;
	rawFactories = round(rawFactories);
	return constrainFactoryNumbers(rawFactories);
}


int HoI4::State::constrainFactoryNumbers(double rawFactories) const
{
	int factories = static_cast<int>(rawFactories);

	int upperLimit = 12;
	if (capitalState)
	{
		upperLimit = 11;
	}

	if (factories < 0)
	{
		factories = 0;
	}
	else if (factories > upperLimit)
	{
		factories = upperLimit;
	}

	return factories;
}


void HoI4::State::determineCategory(int factories, const HoI4::StateCategories& theStateCategories)
{
	if (capitalState)
	{
		factories++;
	}

	int stateSlots = population / POPULATION_PER_STATE_SLOT;
	if (factories >= stateSlots)
	{
		stateSlots = factories + 2;
	}

	if (!impassable)
	{
		category = theStateCategories.getBestCategory(stateSlots);
	}
}


void HoI4::State::addInfrastructureFromRails(float averageRailLevels)
{
	infrastructure += static_cast<int>(averageRailLevels / 2);
}


void HoI4::State::addInfrastructureFromFactories(int factories)
{
	if (factories > FIRST_INFRASTRUCTURE_REWARD_LEVEL)
	{
		infrastructure++;
	}
	if (factories > SECOND_INFRASTRUCTURE_REWARD_LEVEL)
	{
		infrastructure++;
	}
	if (factories > THIRD_INFRASTRUCTURE_REWARD_LEVEL)
	{
		infrastructure++;
	}
}


static std::mt19937 randomnessEngine;
static std::uniform_int_distribution<> numberDistributor(0, 99);
void HoI4::State::setIndustry(int factories, const CoastalProvinces& theCoastalProvinces)
{
	if (ownerHasNoCore())
	{
		factories /= 2;
	}

        struct facs {
                double mil;
                double civ;
                double dok;
        };
        static std::unordered_map<std::string, facs> accumulator;
        if (accumulator.find(ownerTag) == accumulator.end()) {
          accumulator[ownerTag] = {0, 0, 0};
        }
        auto& curr = accumulator[ownerTag];
	if (amICoastal(theCoastalProvinces))
	{
		//		20% chance of dockyard
		//		57% chance of civilian factory
		//		23% chance of military factory
                curr.mil += 0.23 * factories;
                curr.civ += 0.57 * factories;
                curr.dok += 0.20 * factories;
	}
	else
	{
		//		 0% chance of dockyard
		//		71% chance of civilian factory
		//		29% chance of military factory
                curr.mil += 0.29 * factories;
                curr.civ += 0.71 * factories;
	}

        while (curr.mil >= 1.0) {
                milFactories++;
                curr.mil -= 1.0;
        }
        while (curr.civ >= 1.0) {
                civFactories++;
                curr.civ -= 1.0;
        }
        while (curr.dok >= 1.0) {
                dockyards++;
                curr.dok -= 1.0;
        }
}


bool HoI4::State::amICoastal(const CoastalProvinces& theCoastalProvinces) const
{
	auto coastalProvinces = theCoastalProvinces.getCoastalProvinces();
	for (auto province: provinces)
	{
		if (coastalProvinces.contains(province))
		{
			return true;
		}
	}

	return false;
}


bool HoI4::State::ownerHasNoCore() const
{
	return !cores.contains(ownerTag);
}


bool HoI4::State::isProvinceInState(int provinceNum) const
{
	return provinces.contains(provinceNum);
}
