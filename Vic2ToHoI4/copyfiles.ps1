copy "C:\Users\Rolf\base\third_party\Vic2ToHoI4\Resources\msvcp140_codecvt_ids.dll" "..\Release\Vic2ToHoI4\"
copy "C:\Users\Rolf\base\third_party\Vic2ToHoI4\Resources\vcruntime140_1.dll" "..\Release\Vic2ToHoI4\"
copy "C:\Users\Rolf\base\third_party\Vic2ToHoI4\Fronter\Fronter\Resources\VC_redist.x64.exe" "..\Release\Vic2ToHoI4\"
copy "C:\Users\Rolf\base\third_party\Vic2ToHoI4\Fronter\Fronter\Resources\*ico" "..\Release\"
copy "C:\Users\Rolf\base\third_party\Vic2ToHoI4\Fronter\Fronter\Resources\*yml" "..\Release\Configuration\"

# **Create blankMod**
del -Force -Recurse "..\Release\Vic2ToHoI4\blankMod"
copy -Force -Recurse "Data_Files\blankMod" "..\Release\Vic2ToHoI4\blankmod"
exit