#pragma once

#include <array>
#include <string>
#include <vector>

#include "Core/Movie.h"
#include "Core/Scripting/HelperClasses/ClassMetadata.h"
#include "Core/Scripting/HelperClasses/ScriptContext.h"

namespace Scripting::GameCubeControllerApi
{

extern const char* class_name;

extern std::array<Movie::ControllerState, 4> controller_inputs_on_last_frame;

ClassMetadata GetClassMetadataForVersion(const std::string& api_version);
ClassMetadata GetAllClassMetadata();
FunctionMetadata GetFunctionMetadataForVersion(const std::string& api_version,
                                               const std::string& function_name);

ArgHolder* GetInputsForPreviousFrame(ScriptContext* current_script,
                                     std::vector<ArgHolder*>* args_list);
ArgHolder* IsGcControllerInPort(ScriptContext* current_script, std::vector<ArgHolder*>* args_list);
ArgHolder* IsUsingPort(ScriptContext* current_script, std::vector<ArgHolder*>* args_list);

}  // namespace Scripting::GameCubeControllerApi
