#include "Core/Scripting/EventCallbackRegistrationAPIs/OnMemoryAddressReadFromCallbackAPI.h"

#include "Core/Scripting/HelperClasses/MemoryAddressBreakpointsHolder.h"
#include "Core/Scripting/HelperClasses/VersionResolver.h"

namespace Scripting::OnMemoryAddressReadFromCallbackAPI
{

const char* class_name = "OnMemoryAddressReadFrom";

u32 memory_address_read_from_for_current_callback = 0;
u32 read_size = 0;
bool in_memory_address_read_from_breakpoint = false;

static std::array all_on_memory_address_read_from_callback_functions_metadata_list = {
    FunctionMetadata("register", "1.0", "register(memoryStartAddress, memoryEndAddress, value)",
                     Register, Scripting::ArgTypeEnum::RegistrationReturnType,
                     {Scripting::ArgTypeEnum::U32, Scripting::ArgTypeEnum::U32,
                      Scripting::ArgTypeEnum::RegistrationInputType}),
    FunctionMetadata("registerWithAutoDeregistration", "1.0",
                     "registerWithAutoDeregistration(memoryStartAddress, memoryEndAddress, value)",
                     RegisterWithAutoDeregistration, Scripting::ArgTypeEnum::VoidType,
                     {Scripting::ArgTypeEnum::U32, Scripting::ArgTypeEnum::U32,
                      Scripting::ArgTypeEnum::RegistrationWithAutoDeregistrationInputType}),
    FunctionMetadata(
        "unregister", "1.0", "unregister(memoryStartAddress, value)", Unregister,
        Scripting::ArgTypeEnum::VoidType,
        {Scripting::ArgTypeEnum::U32, Scripting::ArgTypeEnum::UnregistrationInputType}),
    FunctionMetadata("isInMemoryAddressReadFromCallback", "1.0",
                     "isInMemoryAddressReadFromCallback()", IsInMemoryAddressReadFromCallback,
                     Scripting::ArgTypeEnum::Boolean, {}),
    FunctionMetadata("getMemoryAddressReadFromForCurrentCallback", "1.0",
                     "getMemoryAddressReadFromForCurrentCallback()",
                     GetMemoryAddressReadFromForCurrentCallback, Scripting::ArgTypeEnum::U32, {}),
    FunctionMetadata("getReadSize", "1.0", "getReadSize()", GetReadSize,
                     Scripting::ArgTypeEnum::U32, {})};

ClassMetadata GetClassMetadataForVersion(const std::string& api_version)
{
  std::unordered_map<std::string, std::string> deprecated_functions_map;
  return {class_name, GetLatestFunctionsForVersion(
                          all_on_memory_address_read_from_callback_functions_metadata_list,
                          api_version, deprecated_functions_map)};
}

ClassMetadata GetAllClassMetadata()
{
  return {class_name,
          GetAllFunctions(all_on_memory_address_read_from_callback_functions_metadata_list)};
}

FunctionMetadata GetFunctionMetadataForVersion(const std::string& api_version,
                                               const std::string& function_name)
{
  std::unordered_map<std::string, std::string> deprecated_functions_map;
  return GetFunctionForVersion(all_on_memory_address_read_from_callback_functions_metadata_list,
                               api_version, function_name, deprecated_functions_map);
}

ArgHolder* Register(ScriptContext* current_script, std::vector<ArgHolder*>* args_list)
{
  u32 memory_breakpoint_start_address = (*args_list)[0]->u32_val;
  u32 memory_breakpoint_end_address = (*args_list)[1]->u32_val;
  void* callback = (*args_list)[2]->void_pointer_val;

  if (memory_breakpoint_start_address == 0)
  {
    return CreateErrorStringArgHolder("Error: Memory address read breakpoint cannot be 0!");
  }

  else if (memory_breakpoint_start_address > memory_breakpoint_end_address)
  {
    return CreateErrorStringArgHolder(
        "Error: Memory address read breakpoint has an ending address less "
        "than its starting address!");
  }

  current_script->memory_address_breakpoints_holder.AddReadBreakpoint(
      memory_breakpoint_start_address, memory_breakpoint_end_address);

  return CreateRegistrationReturnTypeArgHolder(
      current_script->dll_specific_api_definitions.RegisterOnMemoryAddressReadFromCallback(
          current_script, memory_breakpoint_start_address, memory_breakpoint_end_address,
          callback));
}

ArgHolder* RegisterWithAutoDeregistration(ScriptContext* current_script,
                                          std::vector<ArgHolder*>* args_list)
{
  u32 memory_breakpoint_start_address = (*args_list)[0]->u32_val;
  u32 memory_breakpoint_end_address = (*args_list)[1]->u32_val;
  void* callback = (*args_list)[2]->void_pointer_val;

  if (memory_breakpoint_start_address == 0)
  {
    return CreateErrorStringArgHolder("Error: Memory address read breakpoint cannot be 0!");
  }

  else if (memory_breakpoint_start_address > memory_breakpoint_end_address)
  {
    return CreateErrorStringArgHolder("Error: Memory address read breakpoint has an ending address "
                                      "less than its starting address!");
  }

  current_script->memory_address_breakpoints_holder.AddReadBreakpoint(
      memory_breakpoint_start_address, memory_breakpoint_end_address);

  current_script->dll_specific_api_definitions
      .RegisterOnMemoryAddressReadFromWithAutoDeregistrationCallback(
          current_script, memory_breakpoint_start_address, memory_breakpoint_end_address, callback);

  return CreateVoidTypeArgHolder();
}

ArgHolder* Unregister(ScriptContext* current_script, std::vector<ArgHolder*>* args_list)
{
  u32 memory_breakpoint_start_address = (*args_list)[0]->u32_val;
  void* callback = (*args_list)[1]->void_pointer_val;

  if (!current_script->memory_address_breakpoints_holder.ContainsReadBreakpoint(
          memory_breakpoint_start_address))
  {
    return CreateErrorStringArgHolder(
        "Error: Address passed into OnMemoryAddressReadFrom:unregister() did not represent the "
        "start address of a read "
        "breakpoint that was currently enabled!");
  }

  current_script->memory_address_breakpoints_holder.RemoveReadBreakpoint(
      memory_breakpoint_start_address);

  bool return_value =
      current_script->dll_specific_api_definitions.UnregisterOnMemoryAddressReadFromCallback(
          current_script, callback);

  if (!return_value)
  {
    return CreateErrorStringArgHolder(
        "2nd Argument passed into OnMemoryAddressReadFrom:unregister() was not a reference "
        "returned from calling OnMemoryAddressReadFrom:register(), or unregister was called twice "
        "on the same reference");
  }

  else
  {
    return CreateVoidTypeArgHolder();
  }
}

ArgHolder* IsInMemoryAddressReadFromCallback(ScriptContext* current_script,
                                             std::vector<ArgHolder*>* args_list)
{
  return CreateBoolArgHolder(current_script->current_script_call_location ==
                             Scripting::ScriptCallLocationsEnum::FromMemoryAddressReadFromCallback);
}
ArgHolder* GetMemoryAddressReadFromForCurrentCallback(ScriptContext* current_script,
                                                      std::vector<ArgHolder*>* args_list)
{
  if (current_script->current_script_call_location !=
      Scripting::ScriptCallLocationsEnum::FromMemoryAddressReadFromCallback)
  {
    return CreateErrorStringArgHolder(
        "User attempted to call "
        "OnMemoryAddressReadFrom:getMemoryAddressReadFromForCurrentCallback() outside of an "
        "OnMemoryAddressReadFrom callback function!");
  }

  return CreateU32ArgHolder(memory_address_read_from_for_current_callback);
}

ArgHolder* GetReadSize(ScriptContext* current_script, std::vector<ArgHolder*>* args_list)
{
  if (current_script->current_script_call_location !=
      Scripting::ScriptCallLocationsEnum::FromMemoryAddressReadFromCallback)
  {
    return CreateErrorStringArgHolder("User attempted to call "
                                      "OnMemoryAddressReadFrom:getReadSize() outside of an "
                                      "OnMemoryAddressReadFrom callback function!");
  }

  return CreateU32ArgHolder(read_size);
}

}  // namespace Scripting::OnMemoryAddressReadFromCallbackAPI
