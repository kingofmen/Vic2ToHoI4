#include "ImpassableProvinces.h"
#include "HOI4World/States/DefaultState.h"
#include "Log.h"



HoI4::ImpassableProvinces::ImpassableProvinces(const std::map<int, DefaultState>& states)
{
	Log(LogLevel::Info) << "\t\tFinding impassable provinces";
	for (const auto& state: states)
	{
		if (state.second.isImpassable())
		{
			for (auto province: state.second.getProvinces())
			{
				impassibleProvinces.insert(province);
			}
		}
	}
	komsOverrides = std::unordered_set<int>{
		 1958, 10838, 1115, 5004,
	};
}


bool HoI4::ImpassableProvinces::isProvinceImpassable(const int provinceNumber) const
{
  if (komsOverrides.contains(provinceNumber)) {
	 return true;
  }
  return impassibleProvinces.contains(provinceNumber);
}
