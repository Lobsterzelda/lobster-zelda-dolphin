#include "Core/Scripting/HelperClasses/ScriptContext.h"

const char* most_recent_script_version = "1.0.0";

ThreadSafeQueue<ScriptContext*> queue_of_scripts_waiting_to_start =  ThreadSafeQueue<ScriptContext*>();

ScriptContext* castToScriptContextPtr(void* input)
{
  return reinterpret_cast<ScriptContext*>(input);
}

void* ScriptContext_Initializer_impl(int unique_identifier, const char* script_file_name,
                                     void (*print_callback_function)(void*, const char*),
                                     void (*script_end_callback)(void*, int),
                                     void* new_dll_api_definitions)
{
  ScriptContext* ret_val = new ScriptContext();
  ret_val->unique_script_identifier = unique_identifier;
  ret_val->script_filename = script_file_name;
  ret_val->print_callback_function = print_callback_function;
  ret_val->script_end_callback_function = script_end_callback;
  ret_val->current_script_call_location = ScriptCallLocations::FromScriptStartup;
  ret_val->is_script_active = 1;
  ret_val->finished_with_global_code = 0;
  ret_val->called_yielding_function_in_last_global_script_resume = 0;
  ret_val->called_yielding_function_in_last_frame_callback_script_resume = 0;
  ret_val->instructionBreakpointsHolder = InstructionBreakpointsHolder();
  ret_val->memoryAddressBreakpointsHolder = MemoryAddressBreakpointsHolder();
  DLL_Defined_ScriptContext_APIs temp_dll_apis =
      *((DLL_Defined_ScriptContext_APIs*)(new_dll_api_definitions));
  ret_val->dll_specific_api_definitions = temp_dll_apis;

  return *((void**)&ret_val);
}
void ScriptContext_Destructor_impl(void* script_context)
{
  ScriptContext* casted_struct_ptr = castToScriptContextPtr(script_context);
  casted_struct_ptr->is_script_active = 0;
  casted_struct_ptr->dll_specific_api_definitions.DLLSpecificDestructor(script_context);
  delete casted_struct_ptr;

}

void ScriptContext_ShutdownScript_impl(void* script_context)
{
  ScriptContext* casted_struct_ptr = castToScriptContextPtr(script_context);
  casted_struct_ptr->is_script_active = 0;
  casted_struct_ptr->script_end_callback_function(script_context, casted_struct_ptr->unique_script_identifier);
}

PRINT_CALLBACK_TYPE ScriptContext_GetPrintCallback_impl(void* script_context)
{
  return castToScriptContextPtr(script_context)->print_callback_function;
}

SCRIPT_END_CALLBACK_TYPE ScriptContext_GetScriptEndCallback_impl(void* script_context)
{
  return castToScriptContextPtr(script_context)->script_end_callback_function;
}

int ScriptContext_GetUniqueScriptIdentifier_impl(void* script_context)
{
  return castToScriptContextPtr(script_context)->unique_script_identifier;
}

const char* ScriptContext_GetScriptFilename_impl(void* script_context)
{
  return castToScriptContextPtr(script_context)->script_filename;
}

int ScriptContext_GetScriptCallLocation_impl(void* script_context)
{
  return (int) castToScriptContextPtr(script_context)->current_script_call_location;
}

int ScriptContext_GetIsScriptActive_impl(void* script_context)
{
  return castToScriptContextPtr(script_context)->is_script_active;
}

void ScriptContext_SetIsScriptActive_impl(void* script_context, int new_val)
{
  int value_to_set = new_val != 0 ? 1 : 0;
  castToScriptContextPtr(script_context)->is_script_active = value_to_set;
}

int ScriptContext_GetIsFinishedWithGlobalCode_impl(void* script_context)
{
  return castToScriptContextPtr(script_context)->finished_with_global_code;
}

void ScriptContext_SetIsFinishedWithGlobalCode_impl(void* script_context, int new_val)
{
  int value_to_set = new_val != 0 ? 1 : 0;
  castToScriptContextPtr(script_context)->finished_with_global_code = value_to_set;
}

int ScriptContext_GetCalledYieldingFunctionInLastGlobalScriptResume_impl(void* script_context)
{
  return castToScriptContextPtr(script_context)->called_yielding_function_in_last_global_script_resume;
}

void ScriptContext_SetCalledYieldingFunctionInLastGlobalScriptResume_impl(void* script_context, int new_val)
{
  int value_to_set = new_val != 0 ? 1 : 0;
  castToScriptContextPtr(script_context)->called_yielding_function_in_last_global_script_resume = value_to_set;
}

int ScriptContext_GetCalledYieldingFunctionInLastFrameCallbackScriptResume_impl(void* script_context)
{
  return castToScriptContextPtr(script_context)->called_yielding_function_in_last_frame_callback_script_resume;
}

void ScriptContext_SetCalledYieldingFunctionInLastFrameCallbackScriptResume_impl(void* script_context, int new_val)
{
  int value_to_set = new_val != 0 ? 1 : 0;
  castToScriptContextPtr(script_context)->called_yielding_function_in_last_frame_callback_script_resume = value_to_set;
}

void* ScriptContext_GetInstructionBreakpointsHolder_impl(void* script_context)
{
  return reinterpret_cast<void*>(&(castToScriptContextPtr(script_context)->instructionBreakpointsHolder));
}

void* ScriptContext_GetMemoryAddressBreakpointsHolder_impl(void* script_context)
{
  return reinterpret_cast<void*>(&(castToScriptContextPtr(script_context)->memoryAddressBreakpointsHolder));
}

void* ScriptContext_GetDllDefinedScriptContextApis_impl(void* script_context)
{
  return reinterpret_cast<void*>(&(castToScriptContextPtr(script_context)->dll_specific_api_definitions));
}

void* ScriptContext_GetDerivedScriptContextPtr_impl(void* script_context)
{
  return castToScriptContextPtr(script_context)->derived_script_context_class_ptr;
}

const char* ScriptContext_GetScriptVersion_impl()
{
  return most_recent_script_version;
}

void ScriptContext_SetDLLScriptContextPtr(void* script_context, void* dll_script_context)
{
  castToScriptContextPtr(script_context)->derived_script_context_class_ptr = dll_script_context;
}

void ScriptContext_AddScriptToQueueOfScriptsWaitingToStart_impl(void* script_context)
{
  queue_of_scripts_waiting_to_start.push(reinterpret_cast<ScriptContext*>(script_context));
}