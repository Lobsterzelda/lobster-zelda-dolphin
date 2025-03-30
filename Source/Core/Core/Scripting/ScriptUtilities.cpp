#include "Core/Scripting/ScriptUtilities.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "Common/DynamicLibrary.h"
#include "Common/FileUtil.h"
#include "Common/MsgHandler.h"

#include "Core/Core.h"

#include "Core/Scripting/CoreScriptInterface/InternalScriptAPIs/ArgHolder_APIs.h"
#include "Core/Scripting/CoreScriptInterface/InternalScriptAPIs/ClassFunctionsResolver_APIs.h"
#include "Core/Scripting/CoreScriptInterface/InternalScriptAPIs/ClassMetadata_APIs.h"
#include "Core/Scripting/CoreScriptInterface/InternalScriptAPIs/FileUtility_APIs.h"
#include "Core/Scripting/CoreScriptInterface/InternalScriptAPIs/FunctionMetadata_APIs.h"
#include "Core/Scripting/CoreScriptInterface/InternalScriptAPIs/GCButton_APIs.h"
#include "Core/Scripting/CoreScriptInterface/InternalScriptAPIs/ModuleLists_APIs.h"
#include "Core/Scripting/CoreScriptInterface/InternalScriptAPIs/ScriptContext_APIs.h"
#include "Core/Scripting/CoreScriptInterface/InternalScriptAPIs/VectorOfArgHolders_APIs.h"

#include "Core/Scripting/EventCallbackRegistrationAPIs/OnInstructionHitCallbackAPI.h"
#include "Core/Scripting/EventCallbackRegistrationAPIs/OnMemoryAddressReadFromCallbackAPI.h"
#include "Core/Scripting/EventCallbackRegistrationAPIs/OnMemoryAddressWrittenToCallbackAPI.h"

#include "Core/Scripting/HelperClasses/ArgHolder_API_Implementations.h"
#include "Core/Scripting/HelperClasses/ClassFunctionsResolver_API_Implementations.h"
#include "Core/Scripting/HelperClasses/ClassMetadata.h"
#include "Core/Scripting/HelperClasses/FileUtility_API_Implementations.h"
#include "Core/Scripting/HelperClasses/FunctionMetadata.h"
#include "Core/Scripting/HelperClasses/GCButton_API_Implementations.h"
#include "Core/Scripting/HelperClasses/ModuleLists_API_Implementations.h"
#include "Core/Scripting/HelperClasses/ScriptQueueEvent.h"
#include "Core/Scripting/HelperClasses/VectorOfArgHolders_API_Implementations.h"

#include "Core/Scripting/InternalAPIModules/GraphicsAPI.h"

#include "Core/System.h"

namespace Scripting::ScriptUtilities
{

std::vector<ScriptContext*> list_of_all_scripts = std::vector<ScriptContext*>();
static std::vector<ScriptQueueEvent> script_events = std::vector<ScriptQueueEvent>();
static bool is_scripting_core_initialized = false;

std::mutex initialization_and_destruction_lock;
std::mutex global_code_and_frame_callback_running_lock;
std::mutex gc_controller_polled_callback_running_lock;
std::mutex instruction_hit_callback_running_lock;
std::mutex memory_address_read_from_callback_running_lock;
std::mutex memory_address_written_to_callback_running_lock;
std::mutex wii_input_polled_callback_running_lock;
std::mutex graphics_callback_running_lock;

static bool initialized_dolphin_api_structs = false;

static ArgHolder_APIs argHolder_apis = {};
static ClassFunctionsResolver_APIs classFunctionsResolver_apis = {};
static ClassMetadata_APIs classMetadata_apis = {};
static FileUtility_APIs fileUtility_apis = {};
static FunctionMetadata_APIs functionMetadata_apis = {};
static GCButton_APIs gcButton_apis = {};
static ModuleLists_APIs moduleLists_apis = {};
static VectorOfArgHolders_APIs vectorOfArgHolders_apis = {};
static Dolphin_Defined_ScriptContext_APIs dolphin_defined_scriptContext_apis = {};

static bool initialized_dlls = false;

static std::unordered_map<std::string, Common::DynamicLibrary*> file_extension_to_dll_map =
    std::unordered_map<std::string, Common::DynamicLibrary*>();
static std::vector<Common::DynamicLibrary*> all_dlls = std::vector<Common::DynamicLibrary*>();

static u64 size_of_function_pointer = sizeof(argHolder_apis.AddByteToBytesArgHolder);

// Validates that there's no NULL variables in the API struct passed in as an argument.
static bool ValidateApiStruct(void* start_of_struct, unsigned int struct_size,
                              const char* struct_name)
{
  u64* travel_ptr = ((u64*)start_of_struct);

  while (((u8*)travel_ptr) < (((u8*)start_of_struct) + struct_size))
  {
    if (static_cast<u64>(*travel_ptr) == 0)
    {
      std::cout << "Error: " << struct_name << " had a NULL member!" << std::endl;
#ifdef _WIN32
      std::quick_exit(68);
#else
      return false;
#endif
    }
    travel_ptr = (u64*)(((u8*)travel_ptr) + size_of_function_pointer);
  }

  std::cout << "All good in " << struct_name << "!" << std::endl;
  return true;
}

static void Initialize_ArgHolder_APIs()
{
  argHolder_apis.AddByteToBytesArgHolder = AddByteToBytesArgHolder_API_impl;
  argHolder_apis.CreateBytesArgHolder = CreateBytesArgHolder_API_impl;
  argHolder_apis.CreateBoolArgHolder = CreateBoolArgHolder_API_impl;
  argHolder_apis.CreateGameCubeControllerStateArgHolder =
      CreateGameCubeControllerStateArgHolder_API_impl;
  argHolder_apis.CreateDoubleArgHolder = CreateDoubleArgHolder_API_impl;
  argHolder_apis.CreateEmptyOptionalArgHolder = CreateEmptyOptionalArgHolder_API_impl;
  argHolder_apis.CreateFloatArgHolder = CreateFloatArgHolder_API_impl;
  argHolder_apis.CreateListOfPointsArgHolder = CreateListOfPointsArgHolder_API_impl;
  argHolder_apis.CreateRegistrationForButtonCallbackInputTypeArgHolder =
      CreateRegistrationForButtonCallbackInputTypeArgHolder_API_impl;
  argHolder_apis.CreateRegistrationInputTypeArgHolder =
      CreateRegistrationInputTypeArgHolder_API_impl;
  argHolder_apis.CreateRegistrationWithAutoDeregistrationInputTypeArgHolder =
      CreateRegistrationWithAutoDeregistrationInputTypeArgHolder_API_impl;
  argHolder_apis.CreateS8ArgHolder = CreateS8ArgHolder_API_impl;
  argHolder_apis.CreateS16ArgHolder = CreateS16ArgHolder_API_impl;
  argHolder_apis.CreateS32ArgHolder = CreateS32ArgHolder_API_impl;
  argHolder_apis.CreateS64ArgHolder = CreateS64ArgHolder_API_impl;
  argHolder_apis.CreateStringArgHolder = CreateStringArgHolder_API_impl;
  argHolder_apis.CreateU8ArgHolder = CreateU8ArgHolder_API_impl;
  argHolder_apis.CreateU16ArgHolder = CreateU16ArgHolder_API_impl;
  argHolder_apis.CreateU32ArgHolder = CreateU32ArgHolder_API_impl;
  argHolder_apis.CreateU64ArgHolder = CreateU64ArgHolder_API_impl;
  argHolder_apis.CreateUnregistrationInputTypeArgHolder =
      CreateUnregistrationInputTypeArgHolder_API_impl;
  argHolder_apis.CreateVoidPointerArgHolder = CreateVoidPointerArgHolder_API_impl;
  argHolder_apis.Delete_ArgHolder = Delete_ArgHolder_API_impl;
  argHolder_apis.GetArgType = GetArgType_API_impl;
  argHolder_apis.GetBoolFromArgHolder = GetBoolFromArgHolder_API_impl;
  argHolder_apis.GetGameCubeControllerStateArgHolderValue =
      GetGameCubeControllerStateArgHolderValue_API_impl;
  argHolder_apis.GetDoubleFromArgHolder = GetDoubleFromArgHolder_API_impl;
  argHolder_apis.GetErrorStringFromArgHolder = GetErrorStringFromArgHolder_API_impl;
  argHolder_apis.GetFloatFromArgHolder = GetFloatFromArgHolder_API_impl;
  argHolder_apis.GetIsEmpty = GetIsEmpty_API_impl;
  argHolder_apis.GetByteFromBytesArgHolderAtIndex = GetByteFromBytesArgHolderAtIndex_API_impl;
  argHolder_apis.GetListOfPointsXValueAtIndexForArgHolder =
      GetListOfPointsXValueAtIndexForArgHolder_API_impl;
  argHolder_apis.GetListOfPointsYValueAtIndexForArgHolder =
      GetListOfPointsYValueAtIndexForArgHolder_API_impl;
  argHolder_apis.GetS8FromArgHolder = GetS8FromArgHolder_API_impl;
  argHolder_apis.GetS16FromArgHolder = GetS16FromArgHolder_API_impl;
  argHolder_apis.GetS32FromArgHolder = GetS32FromArgHolder_API_impl;
  argHolder_apis.GetS64FromArgHolder = GetS64FromArgHolder_API_impl;
  argHolder_apis.GetSizeOfBytesArgHolder = GetSizeOfBytesArgHolder_API_impl;
  argHolder_apis.GetSizeOfListOfPointsArgHolder = GetSizeOfListOfPointsArgHolder_API_impl;
  argHolder_apis.GetStringFromArgHolder = GetStringFromArgHolder_API_impl;
  argHolder_apis.GetU8FromArgHolder = GetU8FromArgHolder_API_impl;
  argHolder_apis.GetU16FromArgHolder = GetU16FromArgHolder_API_impl;
  argHolder_apis.GetU32FromArgHolder = GetU32FromArgHolder_API_impl;
  argHolder_apis.GetU64FromArgHolder = GetU64FromArgHolder_API_impl;
  argHolder_apis.GetVoidPointerFromArgHolder = GetVoidPointerFromArgHolder_API_impl;
  argHolder_apis.ListOfPointsArgHolderPushBack = ListOfPointsArgHolderPushBack_API_impl;
  argHolder_apis.SetGameCubeControllerStateArgHolderValue =
      SetGameCubeControllerStateArgHolderValue_API_impl;

  ValidateApiStruct(&argHolder_apis, sizeof(argHolder_apis), "ArgHolder_APIs");
}

static void Initialize_ClassFunctionsResolver_APIs()
{
  classFunctionsResolver_apis.Send_ClassMetadataForVersion_To_DLL_Buffer =
      Send_ClassMetadataForVersion_To_DLL_Buffer_impl;
  classFunctionsResolver_apis.Send_FunctionMetadataForVersion_To_DLL_Buffer =
      Send_FunctionMetadataForVersion_To_DLL_Buffer_impl;

  ValidateApiStruct(&classFunctionsResolver_apis, sizeof(classFunctionsResolver_apis),
                    "ClassFunctionsResolver_APIs");
}

static void Initialize_ClassMetadata_APIs()
{
  classMetadata_apis.GetClassNameForClassMetadata = GetClassName_ClassMetadata_impl;
  classMetadata_apis.GetFunctionMetadataAtPosition =
      GetFunctionMetadataAtPosition_ClassMetadata_impl;
  classMetadata_apis.GetNumberOfFunctions = GetNumberOfFunctions_ClassMetadata_impl;

  ValidateApiStruct(&classMetadata_apis, sizeof(classMetadata_apis), "ClassMetadata_APIs");
}

static void Initialize_FileUtility_APIs()
{
  SetUserPath(File::GetUserPath(D_LOAD_IDX));
  SetSysPath(File::GetSysDirectory());

  fileUtility_apis.GetSystemDirectoryPath = GetSystemDirectoryPath_impl;
  fileUtility_apis.GetUserDirectoryPath = GetUserDirectoryPath_impl;

  ValidateApiStruct(&fileUtility_apis, sizeof(fileUtility_apis), "FileUtility_APIs");
}

static void Initialize_FunctionMetadata_APIs()
{
  functionMetadata_apis.GetExampleFunctionCall = GetExampleFunctionCall_impl;
  functionMetadata_apis.GetFunctionName = GetFunctionName_impl;
  functionMetadata_apis.GetFunctionPointer = GetFunctionPointer_impl;
  functionMetadata_apis.GetFunctionVersion = GetFunctionVersion_impl;
  functionMetadata_apis.GetNumberOfArguments = GetNumberOfArguments_impl;
  functionMetadata_apis.GetReturnType = GetReturnType_impl;
  functionMetadata_apis.GetTypeOfArgumentAtIndex = GetTypeOfArgumentAtIndex_impl;
  functionMetadata_apis.RunFunction = RunFunction_impl;

  ValidateApiStruct(&functionMetadata_apis, sizeof(functionMetadata_apis), "FunctionMetadata_APIs");
}

static void Initialize_GCButton_APIs()
{
  gcButton_apis.ConvertButtonEnumToString = ConvertButtonEnumToString_impl;
  gcButton_apis.IsAnalogButton = IsAnalogButton_impl;
  gcButton_apis.IsDigitalButton = IsDigitalButton_impl;
  gcButton_apis.IsValidButtonEnum = IsValidButtonEnum_impl;
  gcButton_apis.ParseGCButton = ParseGCButton_impl;

  ValidateApiStruct(&gcButton_apis, sizeof(gcButton_apis), "GCButton_APIs");
}

static void Initialize_ModuleLists_APIs()
{
  moduleLists_apis.GetElementAtListIndex = GetElementAtListIndex_impl;
  moduleLists_apis.GetImportModuleName = GetImportModuleName_impl;
  moduleLists_apis.GetListOfDefaultModules = GetListOfDefaultModules_impl;
  moduleLists_apis.GetListOfNonDefaultModules = GetListOfNonDefaultModules_impl;
  moduleLists_apis.GetSizeOfList = GetSizeOfList_impl;

  ValidateApiStruct(&moduleLists_apis, sizeof(moduleLists_apis), "ModuleLists_APIs");
}

static void Initialize_VectorOfArgHolders_APIs()
{
  vectorOfArgHolders_apis.CreateNewVectorOfArgHolders = CreateNewVectorOfArgHolders_impl;
  vectorOfArgHolders_apis.Delete_VectorOfArgHolders = Delete_VectorOfArgHolders_impl;
  vectorOfArgHolders_apis.GetArgumentForVectorOfArgHolders = GetArgumentForVectorOfArgHolders_impl;
  vectorOfArgHolders_apis.GetSizeOfVectorOfArgHolders = GetSizeOfVectorOfArgHolders_impl;
  vectorOfArgHolders_apis.PushBack_VectorOfArgHolders = PushBack_VectorOfArgHolders_impl;

  ValidateApiStruct(&vectorOfArgHolders_apis, sizeof(vectorOfArgHolders_apis),
                    "VectorOfArgHolders_APIs");
}

static void Initialize_DolphinDefined_ScriptContext_APIs()
{
  dolphin_defined_scriptContext_apis.GetCalledYieldingFunctionInLastFrameCallbackScriptResume =
      GetCalledYieldingFunctionInLastFrameCallbackScriptResume_impl;
  dolphin_defined_scriptContext_apis.GetCalledYieldingFunctionInLastGlobalScriptResume =
      GetCalledYieldingFunctionInLastGlobalScriptResume_impl;
  dolphin_defined_scriptContext_apis.GetDerivedScriptContextPtr = GetDerivedScriptContextPtr_impl;
  dolphin_defined_scriptContext_apis.SetDerivedScriptContextPtr = SetDerivedScriptContextPtr_impl;
  dolphin_defined_scriptContext_apis.GetDLLDefinedScriptContextAPIs =
      GetDLLDefinedScriptContextAPIs_impl;
  dolphin_defined_scriptContext_apis.SetDLLDefinedScriptContextAPIs =
      SetDLLDefinedScriptContextAPIs_impl;
  dolphin_defined_scriptContext_apis.GetIsFinishedWithGlobalCode = GetIsFinishedWithGlobalCode_impl;
  dolphin_defined_scriptContext_apis.GetIsScriptActive = GetIsScriptActive_impl;
  dolphin_defined_scriptContext_apis.GetPrintCallbackFunction = GetPrintCallback_impl;
  dolphin_defined_scriptContext_apis.GetScriptCallLocation = GetScriptCallLocation_impl;
  dolphin_defined_scriptContext_apis.GetScriptReturnCode = GetScriptReturnCode_impl;
  dolphin_defined_scriptContext_apis.SetScriptReturnCode = SetScriptReturnCode_impl;
  dolphin_defined_scriptContext_apis.GetErrorMessage = GetErrorMessage_impl;
  dolphin_defined_scriptContext_apis.SetErrorMessage = SetErrorMessage_impl;
  dolphin_defined_scriptContext_apis.GetScriptEndCallbackFunction = GetScriptEndCallback_impl;
  dolphin_defined_scriptContext_apis.GetScriptFilename = GetScriptFilename_impl;
  dolphin_defined_scriptContext_apis.GetScriptVersion = GetScriptVersion_impl;
  dolphin_defined_scriptContext_apis.ScriptContext_Destructor = ScriptContext_Destructor_impl;
  dolphin_defined_scriptContext_apis.ScriptContext_Initializer = ScriptContext_Initializer_impl;
  dolphin_defined_scriptContext_apis.SetCalledYieldingFunctionInLastFrameCallbackScriptResume =
      SetCalledYieldingFunctionInLastFrameCallbackScriptResume_impl;
  dolphin_defined_scriptContext_apis.SetCalledYieldingFunctionInLastGlobalScriptResume =
      SetCalledYieldingFunctionInLastGlobalScriptResume_impl;
  dolphin_defined_scriptContext_apis.SetIsFinishedWithGlobalCode = SetIsFinishedWithGlobalCode_impl;
  dolphin_defined_scriptContext_apis.SetIsScriptActive = SetIsScriptActive_impl;
  dolphin_defined_scriptContext_apis.AddScriptToQueueOfScriptsWaitingToStart =
      AddScriptToQueueOfScriptsWaitingToStart_impl;
  dolphin_defined_scriptContext_apis.GetUniqueScriptIdentifier = GetUniqueScriptIdentifier_impl;

  ValidateApiStruct(&dolphin_defined_scriptContext_apis, sizeof(dolphin_defined_scriptContext_apis),
                    "DolphinDefined_ScriptContext_APIs");
}

static void InitializeDolphinApiStructs()
{
  Initialize_ArgHolder_APIs();
  Initialize_ClassFunctionsResolver_APIs();
  Initialize_ClassMetadata_APIs();
  Initialize_FileUtility_APIs();
  Initialize_FunctionMetadata_APIs();
  Initialize_GCButton_APIs();
  Initialize_ModuleLists_APIs();
  Initialize_VectorOfArgHolders_APIs();
  Initialize_DolphinDefined_ScriptContext_APIs();
  initialized_dolphin_api_structs = true;
}

typedef void (*exported_dll_func_type)(void*);

static void queuePanicAlertEvent(const std::string& error_msg)
{
  Core::QueueHostJob([error_msg](Core::System& system) { PanicAlertFmt("{}", error_msg); }, true);
}

bool callSpecifiedDLLInitFunction(Common::DynamicLibrary* dynamic_lib, std::string file_name,
                                  const char* func_name, void* arg_val)
{
  void* func_addr_in_dll = dynamic_lib->GetSymbolAddress(func_name);
  if (func_addr_in_dll == nullptr)
  {
    queuePanicAlertEvent(
        std::string("Error: While searching for valid scripting plugins, attempted to use ") +
        file_name + " as a ScriptingPlugin. However, the " + func_name +
        " function was not defined in this file!");

    return false;
  }
  else
  {
    reinterpret_cast<exported_dll_func_type>(func_addr_in_dll)(arg_val);
    return true;
  }
}

void InitializeDLLs()
{
  std::string dll_suffix = "";
#if defined(_WIN32)
  dll_suffix = ".dll";
#elif defined(__APPLE__)
  dll_suffix = ".dylib";
#else
  dll_suffix = ".so";
#endif

  std::string base_plugins_directory =
      std::string(fileUtility_apis.GetSystemDirectoryPath()) + "ScriptingPlugins";

  for (auto const& dir_entry : std::filesystem::directory_iterator(base_plugins_directory))
  {
    if (dir_entry.is_directory())
    {
      std::string current_dir = std::string(dir_entry.path().generic_string());
      std::string extensions_file_name = "";
      if (current_dir[current_dir.length() - 1] == '/' ||
          current_dir[current_dir.length() - 1] == '\\')
        extensions_file_name = current_dir + "extensions.txt";
      else
        extensions_file_name = current_dir + "/extensions.txt";
      std::ifstream extension_file(extensions_file_name.c_str());
      if (!extension_file.is_open())
        continue;
      std::string current_line = "";
      std::string trimmed_extension = "";
      std::vector<std::string> trimmed_extensions_list = std::vector<std::string>();
      while (std::getline(extension_file, current_line))
      {
        trimmed_extension = "";
        for (int i = 0; i < current_line.length(); ++i)
        {
          if (!std::isblank(current_line[i]))
          {
            char temp[1] = {current_line[i]};
            trimmed_extension.append(temp, 1);
          }
        }
        if (trimmed_extension.length() == 0)
          continue;

        else if (file_extension_to_dll_map.contains(trimmed_extension) ||
                 std::count(trimmed_extensions_list.begin(), trimmed_extensions_list.end(),
                            trimmed_extension) > 0)
        {
          queuePanicAlertEvent("Error: Encountered duplicate file extension of " +
                               trimmed_extension + " while scanning " + extensions_file_name +
                               " file!");
          return;
        }
        else
          trimmed_extensions_list.push_back(trimmed_extension);
      }

      if (trimmed_extensions_list.empty())
      {
        queuePanicAlertEvent(std::string("Warning: file ") + extensions_file_name +
                             " did not contain any extensions!");
        continue;
      }

      // Now, we need to find a dll file in this folder
      for (auto const& inner_file_iterator : std::filesystem::directory_iterator(dir_entry))
      {
        if (!inner_file_iterator.is_directory())
        {
          std::string current_file_name = inner_file_iterator.path().generic_string();
          if (current_file_name.length() > dll_suffix.length() &&
              current_file_name.substr(current_file_name.length() - dll_suffix.length()) ==
                  dll_suffix)
          {
            Common::DynamicLibrary* new_lib = new Common::DynamicLibrary(current_file_name.c_str());
            if (!new_lib->IsOpen())
            {
              delete new_lib;
              continue;
            }

            // We use short-circuit evaluation here with &&, so if one symbol isn't found, we won't
            // try to find any of the symbols after it.
            if (callSpecifiedDLLInitFunction(new_lib, current_file_name, "Init_ArgHolder_APIs",
                                             &argHolder_apis) &&
                callSpecifiedDLLInitFunction(new_lib, current_file_name,
                                             "Init_ClassFunctionsResolver_APIs",
                                             &classFunctionsResolver_apis) &&
                callSpecifiedDLLInitFunction(new_lib, current_file_name, "Init_ClassMetadata_APIs",
                                             &classMetadata_apis) &&
                callSpecifiedDLLInitFunction(new_lib, current_file_name, "Init_FileUtility_APIs",
                                             &fileUtility_apis) &&
                callSpecifiedDLLInitFunction(new_lib, current_file_name,
                                             "Init_FunctionMetadata_APIs",
                                             &functionMetadata_apis) &&
                callSpecifiedDLLInitFunction(new_lib, current_file_name, "Init_GCButton_APIs",
                                             &gcButton_apis) &&
                callSpecifiedDLLInitFunction(new_lib, current_file_name, "Init_ModuleLists_APIs",
                                             &moduleLists_apis) &&
                callSpecifiedDLLInitFunction(new_lib, current_file_name, "Init_ScriptContext_APIs",
                                             &dolphin_defined_scriptContext_apis) &&
                callSpecifiedDLLInitFunction(new_lib, current_file_name,
                                             "Init_VectorOfArgHolders_APIs",
                                             &vectorOfArgHolders_apis))
            {
              for (auto& extension : trimmed_extensions_list)
                file_extension_to_dll_map[extension] = new_lib;
              all_dlls.push_back(new_lib);
              break;
            }
            else
            {
              delete new_lib;
              break;
            }
          }
        }
      }
    }
  }
  initialized_dlls = true;
}

bool IsScriptingCoreInitialized()
{
  return is_scripting_core_initialized;
}

typedef void* (*create_new_script_func_type)(
    int, const char*, Dolphin_Defined_ScriptContext_APIs::PRINT_CALLBACK_FUNCTION_TYPE,
    Dolphin_Defined_ScriptContext_APIs::SCRIPT_END_CALLBACK_FUNCTION_TYPE);

void InitializeScript(
    int unique_script_identifier, const std::string& script_filename,
    Dolphin_Defined_ScriptContext_APIs::PRINT_CALLBACK_FUNCTION_TYPE new_print_callback,
    Dolphin_Defined_ScriptContext_APIs::SCRIPT_END_CALLBACK_FUNCTION_TYPE new_script_end_callback)
{
  if (script_filename.length() < 1)
    return;

  // We only call this function from inside of ProccessScriptQueueEvents, which holds the
  // initialization_and_destruction lock when it makes this call.
  if (!initialized_dolphin_api_structs)
    InitializeDolphinApiStructs();

  if (!initialized_dlls)
  {
    InitializeDLLs();
  }

  std::string file_suffix = "";
  for (unsigned long long i = script_filename.length() - 1; i > 0; --i)
    if (script_filename[i] == '.')
    {
      file_suffix = script_filename.substr(i);
      break;
    }

  if (file_suffix.empty() || !file_extension_to_dll_map.contains(file_suffix))
    file_suffix =
        ".lua";  // Lua is the default language to try if we encounter an unknown file type.

  list_of_all_scripts.push_back(
      reinterpret_cast<ScriptContext*>(reinterpret_cast<create_new_script_func_type>(
          file_extension_to_dll_map[file_suffix]->GetSymbolAddress("CreateNewScript"))(
          unique_script_identifier, script_filename.c_str(), new_print_callback,
          new_script_end_callback)));

  if (list_of_all_scripts[list_of_all_scripts.size() - 1]->script_return_code !=
      Scripting::ScriptReturnCodesEnum::SuccessCode)
  {
    queuePanicAlertEvent(list_of_all_scripts[list_of_all_scripts.size() - 1]->last_script_error);
  }
}

void SetIsScriptActiveToFalse(void* base_script_context_ptr)
{
  dolphin_defined_scriptContext_apis.SetIsScriptActive(base_script_context_ptr, 0);
}

void StopScript(int unique_script_identifier)
{
  Core::CPUThreadGuard lock(Core::System::GetInstance());
  // initialization_and_destruction_lock.lock(); We already have this locked when the function is
  // called.
  global_code_and_frame_callback_running_lock.lock();
  gc_controller_polled_callback_running_lock.lock();
  instruction_hit_callback_running_lock.lock();
  memory_address_read_from_callback_running_lock.lock();
  memory_address_written_to_callback_running_lock.lock();
  wii_input_polled_callback_running_lock.lock();
  graphics_callback_running_lock.lock();

  ScriptContext* script_to_delete = nullptr;

  for (size_t i = 0; i < list_of_all_scripts.size(); ++i)
  {
    list_of_all_scripts[i]->script_specific_lock.lock();
    if (list_of_all_scripts[i]->unique_script_identifier == unique_script_identifier)
      script_to_delete = list_of_all_scripts[i];
    list_of_all_scripts[i]->script_specific_lock.unlock();
  }

  if (script_to_delete != nullptr)
  {
    list_of_all_scripts.erase(
        std::find(list_of_all_scripts.begin(), list_of_all_scripts.end(), script_to_delete));
    dolphin_defined_scriptContext_apis.ScriptContext_Destructor(script_to_delete);
  }
  graphics_callback_running_lock.unlock();
  wii_input_polled_callback_running_lock.unlock();
  memory_address_written_to_callback_running_lock.unlock();
  memory_address_read_from_callback_running_lock.unlock();
  instruction_hit_callback_running_lock.unlock();
  gc_controller_polled_callback_running_lock.unlock();
  global_code_and_frame_callback_running_lock.unlock();
  // initialization_and_destruction_lock.unlock();
}

void PushScriptCreateQueueEvent(
    const int new_unique_script_identifier, const char* new_script_filename,
    const Dolphin_Defined_ScriptContext_APIs::PRINT_CALLBACK_FUNCTION_TYPE& new_print_callback_func,
    const Dolphin_Defined_ScriptContext_APIs::SCRIPT_END_CALLBACK_FUNCTION_TYPE&
        new_script_end_callback_func)
{
  std::lock_guard<std::mutex> lock(initialization_and_destruction_lock);
  is_scripting_core_initialized = true;
  script_events.push_back(ScriptQueueEvent(ScriptQueueEventTypes::CreateScriptFromUI,
                                           new_unique_script_identifier, new_script_filename,
                                           new_print_callback_func, new_script_end_callback_func));
}

void PushScriptStopQueueEvent(const ScriptQueueEventTypes event_type, const int script_identifier)
{
  std::lock_guard<std::mutex> lock(initialization_and_destruction_lock);
  is_scripting_core_initialized = true;
  script_events.push_back(ScriptQueueEvent(event_type, script_identifier, "", nullptr, nullptr));
}

void ProcessScriptQueueEvents()
{
  // Core::CPUThreadGuard lock(Core::System::GetInstance());
  std::lock_guard<std::mutex> lock(initialization_and_destruction_lock);
  if (script_events.size() == 0)
    return;

  for (unsigned long long i = 0; i < script_events.size() - 1;
       ++i)  // deleting all duplicate events from the queue
  {
    for (unsigned long long j = i + 1; j < script_events.size(); ++j)
    {
      if (script_events[i] == script_events[j])
      {
        script_events.erase(script_events.begin() + j);
        --j;
      }
    }
  }

  for (unsigned long long i = 0; i < script_events.size() - 1;
       ++i)  // Now, removing all pairs of start and stop script events that have the same unique
             // script identifier
  {
    if (script_events[i].event_type == ScriptQueueEventTypes::CreateScriptFromUI)
    {
      int current_script_identifier = script_events[i].unique_script_identifier;
      for (unsigned long long j = i + 1; j < script_events.size(); ++j)
      {
        if (script_events[j].unique_script_identifier == current_script_identifier &&
            script_events[j].event_type == ScriptQueueEventTypes::StopScriptFromUI)
        {
          script_events.erase(script_events.begin() + j);
          script_events.erase(script_events.begin() + i);
          break;
        }
      }
    }
  }

  // Now, we begin processing the events in the queue in the order that they were added to the
  // queue.
  while (script_events.size() > 0)
  {
    ScriptQueueEvent current_event = script_events[0];
    if (current_event.event_type == ScriptQueueEventTypes::CreateScriptFromUI)
      InitializeScript(current_event.unique_script_identifier, current_event.script_filename,
                       current_event.print_callback_func, current_event.script_end_callback_func);
    else if (current_event.event_type == ScriptQueueEventTypes::StopScriptFromUI ||
             current_event.event_type == ScriptQueueEventTypes::StopScriptFromScriptEndCallback)
      StopScript(current_event.unique_script_identifier);
    else
    {
      queuePanicAlertEvent("Error: Encountered unknown ScriptQueueEventType in "
                           "ProcessScriptQueueEvents() function.");
      return;
    }
    script_events.erase(script_events.begin());
  }
}

bool StartScripts()
{
  std::lock_guard<std::mutex> lock(initialization_and_destruction_lock);
  std::lock_guard<std::mutex> second_lock(global_code_and_frame_callback_running_lock);
  bool return_value = false;
  if (list_of_all_scripts.size() == 0 || queue_of_scripts_waiting_to_start.Empty())
    return false;
  while (!queue_of_scripts_waiting_to_start.Empty())
  {
    ScriptContext* next_script = queue_of_scripts_waiting_to_start.Front();
    queue_of_scripts_waiting_to_start.Pop();

    if (next_script == nullptr)
      break;
    next_script->script_specific_lock.lock();
    if (next_script->is_script_active)
    {
      next_script->current_script_call_location =
          Scripting::ScriptCallLocationsEnum::FromScriptStartup;
      next_script->dll_specific_api_definitions.StartScript(next_script);
      return_value = next_script->called_yielding_function_in_last_global_script_resume;
      if (next_script->script_return_code != 0)
        queuePanicAlertEvent(next_script->last_script_error);
    }
    next_script->script_specific_lock.unlock();
    if (return_value)
      break;
  }
  return return_value;
}

bool RunGlobalCode()
{
  std::lock_guard<std::mutex> lock(global_code_and_frame_callback_running_lock);
  bool return_value = false;
  for (size_t i = 0; i < list_of_all_scripts.size(); ++i)
  {
    ScriptContext* current_script = list_of_all_scripts[i];
    current_script->script_specific_lock.lock();
    if (current_script->is_script_active && !current_script->finished_with_global_code)
    {
      current_script->current_script_call_location =
          Scripting::ScriptCallLocationsEnum::FromFrameStartGlobalScope;
      current_script->dll_specific_api_definitions.RunGlobalScopeCode(current_script);
      return_value = current_script->called_yielding_function_in_last_global_script_resume;
      if (current_script->script_return_code != 0)
        queuePanicAlertEvent(current_script->last_script_error);
    }
    current_script->script_specific_lock.unlock();
    if (return_value)
      break;
  }
  return return_value;
}

bool RunOnFrameStartCallbacks()
{
  std::lock_guard<std::mutex> lock(global_code_and_frame_callback_running_lock);
  bool return_value = false;
  for (size_t i = 0; i < list_of_all_scripts.size(); ++i)
  {
    ScriptContext* current_script = list_of_all_scripts[i];
    current_script->script_specific_lock.lock();
    if (current_script->is_script_active)
    {
      current_script->current_script_call_location =
          Scripting::ScriptCallLocationsEnum::FromFrameStartCallback;
      current_script->dll_specific_api_definitions.RunOnFrameStartCallbacks(current_script);
      return_value = current_script->called_yielding_function_in_last_frame_callback_script_resume;
      if (current_script->script_return_code != 0)
        queuePanicAlertEvent(current_script->last_script_error);
    }
    current_script->script_specific_lock.unlock();
    if (return_value)
      break;
  }
  return return_value;
}

void RunOnGCInputPolledCallbacks()
{
  std::lock_guard<std::mutex> lock(gc_controller_polled_callback_running_lock);
  for (size_t i = 0; i < list_of_all_scripts.size(); ++i)
  {
    ScriptContext* current_script = list_of_all_scripts[i];
    current_script->script_specific_lock.lock();
    if (current_script->is_script_active)
    {
      current_script->current_script_call_location =
          Scripting::ScriptCallLocationsEnum::FromGCControllerInputPolledCallback;
      current_script->dll_specific_api_definitions.RunOnGCControllerPolledCallbacks(current_script);
      if (current_script->script_return_code != 0)
        queuePanicAlertEvent(current_script->last_script_error);
    }
    current_script->script_specific_lock.unlock();
  }
}

void RunOnInstructionHitCallbacks(u32 instruction_address)
{
  std::lock_guard<std::mutex> lock(instruction_hit_callback_running_lock);

  OnInstructionHitCallbackAPI::instruction_address_for_current_callback = instruction_address;
  for (size_t i = 0; i < list_of_all_scripts.size(); ++i)
  {
    ScriptContext* current_script = list_of_all_scripts[i];
    current_script->script_specific_lock.lock();
    if (current_script->is_script_active)
    {
      current_script->current_script_call_location =
          Scripting::ScriptCallLocationsEnum::FromInstructionHitCallback;
      current_script->dll_specific_api_definitions.RunOnInstructionHitCallbacks(
          current_script, instruction_address);
      if (current_script->script_return_code != 0)
        queuePanicAlertEvent(current_script->last_script_error);
    }
    current_script->script_specific_lock.unlock();
  }
}

void RunOnMemoryAddressReadFromCallbacks(u32 memory_address)
{
  std::lock_guard<std::mutex> lock(memory_address_read_from_callback_running_lock);

  OnMemoryAddressReadFromCallbackAPI::memory_address_read_from_for_current_callback =
      memory_address;
  for (size_t i = 0; i < list_of_all_scripts.size(); ++i)
  {
    ScriptContext* current_script = list_of_all_scripts[i];
    current_script->script_specific_lock.lock();
    if (current_script->is_script_active)
    {
      current_script->current_script_call_location =
          Scripting::ScriptCallLocationsEnum::FromMemoryAddressReadFromCallback;
      current_script->dll_specific_api_definitions.RunOnMemoryAddressReadFromCallbacks(
          current_script, memory_address);
      if (current_script->script_return_code != 0)
        queuePanicAlertEvent(current_script->last_script_error);
    }
    current_script->script_specific_lock.unlock();
  }
}

void RunOnMemoryAddressWrittenToCallbacks(u32 memory_address, s64 new_value)
{
  std::lock_guard<std::mutex> lock(memory_address_written_to_callback_running_lock);

  OnMemoryAddressWrittenToCallbackAPI::memory_address_written_to_for_current_callback =
      memory_address;
  OnMemoryAddressWrittenToCallbackAPI::value_written_to_memory_address_for_current_callback =
      new_value;
  for (size_t i = 0; i < list_of_all_scripts.size(); ++i)
  {
    ScriptContext* current_script = list_of_all_scripts[i];
    current_script->script_specific_lock.lock();
    if (current_script->is_script_active)
    {
      current_script->current_script_call_location =
          Scripting::ScriptCallLocationsEnum::FromMemoryAddressWrittenToCallback;
      current_script->dll_specific_api_definitions.RunOnMemoryAddressWrittenToCallbacks(
          current_script, memory_address);
      if (current_script->script_return_code != 0)
        queuePanicAlertEvent(current_script->last_script_error);
    }
    current_script->script_specific_lock.unlock();
  }
}

void RunOnWiiInputPolledCallbacks()
{
  std::lock_guard<std::mutex> lock(wii_input_polled_callback_running_lock);

  for (size_t i = 0; i < list_of_all_scripts.size(); ++i)
  {
    ScriptContext* current_script = list_of_all_scripts[i];
    current_script->script_specific_lock.lock();
    if (current_script->is_script_active)
    {
      current_script->current_script_call_location =
          Scripting::ScriptCallLocationsEnum::FromWiiInputPolledCallback;
      current_script->dll_specific_api_definitions.RunOnWiiInputPolledCallbacks(current_script);
      if (current_script->script_return_code != 0)
        queuePanicAlertEvent(current_script->last_script_error);
    }
    current_script->script_specific_lock.unlock();
  }
}

void RunButtonCallbacksInQueues()
{
  std::lock_guard<std::mutex> lock(graphics_callback_running_lock);
  for (size_t i = 0; i < list_of_all_scripts.size(); ++i)
  {
    ScriptContext* current_script = list_of_all_scripts[i];
    current_script->script_specific_lock.lock();
    if (current_script->is_script_active)
    {
      current_script->current_script_call_location =
          Scripting::ScriptCallLocationsEnum::FromGraphicsCallback;
      current_script->dll_specific_api_definitions.RunButtonCallbacksInQueue(current_script);
      if (current_script->script_return_code != 0)
        queuePanicAlertEvent(current_script->last_script_error);
    }
    current_script->script_specific_lock.unlock();
  }
}

}  // namespace Scripting::ScriptUtilities
