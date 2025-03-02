#include "Core/Scripting/EventCallbackRegistrationAPIs/OnInstructionHitCallbackAPI.h"

#include "Core/Scripting/HelperClasses/InstructionBreakpointsHolder.h"
#include "Core/Scripting/HelperClasses/VersionResolver.h"

namespace Scripting::OnInstructionHitCallbackAPI
{

const char* class_name = "OnInstructionHit";
u32 instruction_address_for_current_callback = 0;
bool in_instruction_hit_breakpoint = false;

static std::array all_on_instruction_hit_callback_functions_metadata_list = {
    FunctionMetadata("register", "1.0", "register(instructionAddress, value)", Register,
                     Scripting::ArgTypeEnum::RegistrationReturnType,
                     {Scripting::ArgTypeEnum::U32, Scripting::ArgTypeEnum::RegistrationInputType}),
    FunctionMetadata("registerWithAutoDeregistration", "1.0",
                     "registerWithAutoDeregistration(instructionAddress, value)",
                     RegisterWithAutoDeregistration, Scripting::ArgTypeEnum::VoidType,
                     {Scripting::ArgTypeEnum::U32,
                      Scripting::ArgTypeEnum::RegistrationWithAutoDeregistrationInputType}),
    FunctionMetadata(
        "unregister", "1.0", "unregister(instructionAddress, value)", Unregister,
        Scripting::ArgTypeEnum::VoidType,
        {Scripting::ArgTypeEnum::U32, Scripting::ArgTypeEnum::UnregistrationInputType}),
    FunctionMetadata("isInInstructionHitCallback", "1.0", "isInInstructionHitCallback()",
                     IsInInstructionHitCallback, Scripting::ArgTypeEnum::Boolean, {}),
    FunctionMetadata("getAddressOfInstructionForCurrentCallback", "1.0",
                     "getAddressOfInstructionForCurrentCallback()",
                     GetAddressOfInstructionForCurrentCallback, Scripting::ArgTypeEnum::U32, {})};

ClassMetadata GetClassMetadataForVersion(const std::string& api_version)
{
  std::unordered_map<std::string, std::string> deprecated_functions_map;
  return {class_name,
          GetLatestFunctionsForVersion(all_on_instruction_hit_callback_functions_metadata_list,
                                       api_version, deprecated_functions_map)};
}

ClassMetadata GetAllClassMetadata()
{
  return {class_name, GetAllFunctions(all_on_instruction_hit_callback_functions_metadata_list)};
}

FunctionMetadata GetFunctionMetadataForVersion(const std::string& api_version,
                                               const std::string& function_name)
{
  std::unordered_map<std::string, std::string> deprecated_functions_map;
  return GetFunctionForVersion(all_on_instruction_hit_callback_functions_metadata_list, api_version,
                               function_name, deprecated_functions_map);
}

ArgHolder* Register(ScriptContext* current_script, std::vector<ArgHolder*>* args_list)
{
  u32 address_of_breakpoint = (*args_list)[0]->u32_val;
  void* callback = (*args_list)[1]->void_pointer_val;

  current_script->instruction_breakpoints_holder.AddBreakpoint(address_of_breakpoint);
  return CreateRegistrationReturnTypeArgHolder(
      current_script->dll_specific_api_definitions.RegisterOnInstructionHitCallback(
          current_script, address_of_breakpoint, callback));
}

ArgHolder* RegisterWithAutoDeregistration(ScriptContext* current_script,
                                          std::vector<ArgHolder*>* args_list)
{
  u32 address_of_breakpoint = (*args_list)[0]->u32_val;
  void* callback = (*args_list)[1]->void_pointer_val;

  current_script->instruction_breakpoints_holder.AddBreakpoint(address_of_breakpoint);
  current_script->dll_specific_api_definitions
      .RegisterOnInstructionHitWithAutoDeregistrationCallback(current_script, address_of_breakpoint,
                                                              callback);
  return CreateVoidTypeArgHolder();
}

ArgHolder* Unregister(ScriptContext* current_script, std::vector<ArgHolder*>* args_list)
{
  u32 address_of_breakpoint = (*args_list)[0]->u32_val;
  void* callback = (*args_list)[1]->void_pointer_val;

  if (!current_script->instruction_breakpoints_holder.ContainsBreakpoint(address_of_breakpoint))
  {
    return CreateErrorStringArgHolder(
        "Error: Address passed into OnInstructionHit:unregister() did not correspond to any "
        "breakpoint that was currently enabled!");
  }

  current_script->instruction_breakpoints_holder.RemoveBreakpoint(address_of_breakpoint);

  bool return_value =
      current_script->dll_specific_api_definitions.UnregisterOnInstructionHitCallback(
          current_script, callback);

  if (!return_value)
  {
    return CreateErrorStringArgHolder(
        "Argument passed into OnInstructionHit:unregister() was not a reference returned from "
        "calling OnInstructionHit:register(), or unregister was called twice on the same "
        "reference");
  }

  else
  {
    return CreateVoidTypeArgHolder();
  }
}

ArgHolder* IsInInstructionHitCallback(ScriptContext* current_script,
                                      std::vector<ArgHolder*>* args_list)
{
  return CreateBoolArgHolder(current_script->current_script_call_location ==
                             Scripting::ScriptCallLocationsEnum::FromInstructionHitCallback);
}

ArgHolder* GetAddressOfInstructionForCurrentCallback(ScriptContext* current_script,
                                                     std::vector<ArgHolder*>* args_list)
{
  if (current_script->current_script_call_location !=
      Scripting::ScriptCallLocationsEnum::FromInstructionHitCallback)
  {
    return CreateErrorStringArgHolder(
        "User attempted to call OnInstructionHit:getAddressOfInstructionForCurrentCallback() "
        "outside of an OnInstructionHit callback function!");
  }

  return CreateU32ArgHolder(instruction_address_for_current_callback);
}

}  // namespace Scripting::OnInstructionHitCallbackAPI
