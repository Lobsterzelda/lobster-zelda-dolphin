#include "Core/Scripting/EventCallbackRegistrationAPIs/OnGCControllerPolledCallbackAPI.h"

#include "Core/Scripting/HelperClasses/VersionResolver.h"

namespace Scripting::OnGCControllerPolledCallbackAPI
{

const char* class_name = "OnGCControllerPolled";

std::array<bool, 4> overwrite_controller_at_specified_port{};
std::array<Movie::ControllerState, 4> new_controller_inputs{};
std::array<Movie::ControllerState, 4> controller_inputs_on_last_frame{};
int current_controller_number_polled = -1;

static std::array all_on_gc_controller_polled_callback_functions_metadata_list = {
    FunctionMetadata("register", "1.0", "register(value)", Register,
                     Scripting::ArgTypeEnum::RegistrationReturnType,
                     {Scripting::ArgTypeEnum::RegistrationInputType}),
    FunctionMetadata("registerWithAutoDeregistration", "1.0",
                     "registerWithAutoDeregisteration(value)", RegisterWithAutoDeregistration,
                     Scripting::ArgTypeEnum::VoidType,
                     {Scripting::ArgTypeEnum::RegistrationWithAutoDeregistrationInputType}),
    FunctionMetadata("unregister", "1.0", "unregister(value)", Unregister,
                     Scripting::ArgTypeEnum::VoidType,
                     {Scripting::ArgTypeEnum::UnregistrationInputType}),

    FunctionMetadata("isInGCControllerPolledCallback", "1.0", "isInGCControllerPolledCallback()",
                     IsInGCControllerPolledCallback, Scripting::ArgTypeEnum::Boolean, {}),
    FunctionMetadata("getCurrentPortNumberOfPoll", "1.0", "getCurrentPortNumberOfPoll()",
                     GetCurrentPortNumberOfPoll, Scripting::ArgTypeEnum::S64, {}),
    FunctionMetadata("setInputsForPoll", "1.0", "setInputsForPoll(controllerValuesTable)",
                     SetInputsForPoll, Scripting::ArgTypeEnum::VoidType,
                     {Scripting::ArgTypeEnum::GameCubeControllerStateObject}),
    FunctionMetadata("getInputsForPoll", "1.0", "getInputsForPoll()", GetInputsForPoll,
                     Scripting::ArgTypeEnum::GameCubeControllerStateObject, {})};

ClassMetadata GetClassMetadataForVersion(const std::string& api_version)
{
  std::unordered_map<std::string, std::string> deprecated_functions_map;
  return {class_name,
          GetLatestFunctionsForVersion(all_on_gc_controller_polled_callback_functions_metadata_list,
                                       api_version, deprecated_functions_map)};
}

ClassMetadata GetAllClassMetadata()
{
  return {class_name,
          GetAllFunctions(all_on_gc_controller_polled_callback_functions_metadata_list)};
}

FunctionMetadata GetFunctionMetadataForVersion(const std::string& api_version,
                                               const std::string& function_name)
{
  std::unordered_map<std::string, std::string> deprecated_functions_map;
  return GetFunctionForVersion(all_on_gc_controller_polled_callback_functions_metadata_list,
                               api_version, function_name, deprecated_functions_map);
}

ArgHolder* Register(ScriptContext* current_script, std::vector<ArgHolder*>* args_list)
{
  return CreateRegistrationReturnTypeArgHolder(
      current_script->dll_specific_api_definitions.RegisterOnGCControllerPolledCallback(
          current_script, (*args_list)[0]->void_pointer_val));
}

ArgHolder* RegisterWithAutoDeregistration(ScriptContext* current_script,
                                          std::vector<ArgHolder*>* args_list)
{
  current_script->dll_specific_api_definitions
      .RegisterOnGCControllerPolledWithAutoDeregistrationCallback(
          current_script, (*args_list)[0]->void_pointer_val);
  return CreateVoidTypeArgHolder();
}

ArgHolder* Unregister(ScriptContext* current_script, std::vector<ArgHolder*>* args_list)
{
  bool return_value =
      current_script->dll_specific_api_definitions.UnregisterOnGCControllerPolledCallback(
          current_script, (*args_list)[0]->void_pointer_val);

  if (!return_value)
  {
    return CreateErrorStringArgHolder(
        "Argument passed into OnGCControllerPolled:unregister() was not a reference returned from "
        "calling OnGCControllerPolled:register(), or unregister was called twice on the same "
        "reference");
  }

  else
  {
    return CreateVoidTypeArgHolder();
  }
}

ArgHolder* IsInGCControllerPolledCallback(ScriptContext* current_script,
                                          std::vector<ArgHolder*>* args_list)
{
  return CreateBoolArgHolder(
      current_script->current_script_call_location ==
      Scripting::ScriptCallLocationsEnum::FromGCControllerInputPolledCallback);
}

ArgHolder* GetCurrentPortNumberOfPoll(ScriptContext* current_script,
                                      std::vector<ArgHolder*>* args_list)
{
  if (current_script->current_script_call_location !=
      Scripting::ScriptCallLocationsEnum::FromGCControllerInputPolledCallback)
  {
    return CreateErrorStringArgHolder(
        "User attempted to call OnGCControllerPolled:getCurrentPortNumberOfPoll() outside of an "
        "OnGCControllerPolled callback function!");
  }

  return CreateS64ArgHolder(current_controller_number_polled + 1);
}

// NOTE: In SI.cpp, UpdateDevices() is called to update each device, which moves exactly 8 bytes
// forward for each controller. Also, it moves in order from controllers 1 to 4.
ArgHolder* SetInputsForPoll(ScriptContext* current_script, std::vector<ArgHolder*>* args_list)
{
  if (current_script->current_script_call_location !=
      Scripting::ScriptCallLocationsEnum::FromGCControllerInputPolledCallback)
  {
    return CreateErrorStringArgHolder(
        "User attempted to call OnGCControllerPolled:setInputsForPoll() outside of an "
        "OnGCControllerPolled callback function!");
  }

  overwrite_controller_at_specified_port[current_controller_number_polled] = true;
  new_controller_inputs[current_controller_number_polled] =
      (*args_list)[0]->game_cube_controller_state_val;
  return CreateVoidTypeArgHolder();
}

ArgHolder* GetInputsForPoll(ScriptContext* current_script, std::vector<ArgHolder*>* args_list)
{
  if (current_script->current_script_call_location !=
      Scripting::ScriptCallLocationsEnum::FromGCControllerInputPolledCallback)
  {
    return CreateErrorStringArgHolder(
        "User attempted to call OnGCControllerPolled:getInputsForPoll() outside of an "
        "OnGCControllerPolled callback function!");
  }

  return CreateGameCubeControllerStateArgHolder(
      new_controller_inputs[current_controller_number_polled]);
}

}  // namespace Scripting::OnGCControllerPolledCallbackAPI
