#include "LuaMemoryApi.h"
#include <optional>
#include "Core/PowerPC/mmu.h"
#include "Core/HW/Memmap.h"
#include "LuaByteWrapper.h"
#include "Core/System.h"

namespace Lua
{
namespace LuaMemoryApi
{

MEMORY* GetInstance()
{
  if (instance == NULL)
    instance = new MEMORY();
  return instance;
}

void InitLuaMemoryApi(lua_State* luaState)
{
  MEMORY** instancePtrPtr = (MEMORY**)lua_newuserdata(luaState, sizeof(MEMORY*));
  *instancePtrPtr = GetInstance();
  luaL_newmetatable(luaState, "LuaMemoryFunctionsMetaTable");
  lua_pushvalue(luaState, -1);
  lua_setfield(luaState, -2, "__index");

  luaL_Reg* luaMemoryFunctionsList = new luaL_Reg[]{

      {"readFrom", do_general_read},
      {"read_unsigned_bytes", do_read_unsigned_bytes},
      {"read_signed_bytes", do_read_signed_bytes},
      {"read_fixed_length_string", do_read_fixed_length_string},
      {"read_null_terminated_string", do_read_null_terminated_string},

      {"writeTo", do_general_write},
      {"write_bytes", do_write_bytes},
      {"write_string", do_write_string},

      {nullptr, nullptr}};

  luaL_setfuncs(luaState, luaMemoryFunctionsList, 0);
  lua_setmetatable(luaState, -2);
  lua_setglobal(luaState, "memory");
}

// The following functions are helper functions which are only used in this file:
u8 read_u8_from_domain_function(lua_State* luaState, u32 address)
{
  std::optional<PowerPC::ReadResult<u8>> readResult = PowerPC::HostTryReadU8(address);
  if (!readResult.has_value())
  {
    luaL_error(luaState, "Error: Attempt to read_u8 from memory failed!");
    return 1;
  }
  return readResult.value().value;
}

u16 read_u16_from_domain_function(lua_State* luaState, u32 address)
{
  std::optional<PowerPC::ReadResult<u16>> readResult = PowerPC::HostTryReadU16(address);
  if (!readResult.has_value())
  {
    luaL_error(luaState, "Error: Attempt to read_u16 from memory failed!");
    return 1;
  }
  return readResult.value().value;
}

u32 read_u32_from_domain_function(lua_State* luaState, u32 address)
{
  std::optional<PowerPC::ReadResult<u32>> readResult = PowerPC::HostTryReadU32(address);
  if (!readResult.has_value())
  {
    luaL_error(luaState, "Error: Attempt to read_u32 from memory failed!");
    return 1;
  }
  return readResult.value().value;
}

u64 read_u64_from_domain_function(lua_State* luaState, u32 address)
{
  std::optional<PowerPC::ReadResult<u64>> readResult = PowerPC::HostTryReadU64(address);
  if (!readResult.has_value())
  {
    luaL_error(luaState, "Error: Attempt to read_u64 from memory failed!");
    return 1;
  }
  return readResult.value().value;
}

std::string get_string_from_domain_function(lua_State* luaState, u32 address, u32 length)
{
  std::optional<PowerPC::ReadResult<std::string>> readResult =
      PowerPC::HostTryReadString(address, length);
  if (!readResult.has_value())
  {
    luaL_error(luaState, "Error: Attempt to read string from memory failed!");
    return "";
  }
  return readResult.value().value;
}

u8* get_pointer_to_domain_function(lua_State* luaState, u32 address)
{
  return Core::System::GetInstance().GetMemory().GetPointer(address);
}

void write_u8_to_domain_function(lua_State* luaState, u32 address, u8 val)
{
  std::optional<PowerPC::WriteResult> writeResult = PowerPC::HostTryWriteU8(val, address);
  if (!writeResult.has_value())
    luaL_error(luaState, "Error: Attempt to write_u8 to memory failed!");
}

void write_u16_to_domain_function(lua_State* luaState, u32 address, u16 val)
{
  std::optional<PowerPC::WriteResult> writeResult = PowerPC::HostTryWriteU16(val, address);
  if (!writeResult.has_value())
    luaL_error(luaState, "Error: Attempt to write_u16 to memory failed!");
}

void write_u32_to_domain_function(lua_State* luaState, u32 address, u32 val)
{
  std::optional<PowerPC::WriteResult> writeResult = PowerPC::HostTryWriteU32(val, address);
  if (!writeResult.has_value())
    luaL_error(luaState, "Error: Attempt to write_u32 to memory failed!");
}

void write_u64_to_domain_function(lua_State* luaState, u32 address, u64 val)
{
  std::optional<PowerPC::WriteResult> writeResult = PowerPC::HostTryWriteU64(val, address);
  if (!writeResult.has_value())
    luaL_error(luaState, "Error: Attempt to write_u64 to memory failed!");
}

void copy_string_to_domain_function(lua_State* luaState, u32 address, const char* stringToWrite, size_t stringLength)
{
  Core::System::GetInstance().GetMemory().CopyToEmu(address, stringToWrite, stringLength + 1);
}

u8 read_u8(lua_State* luaState, u32 address)
{
  return read_u8_from_domain_function(luaState, address);
}

s8 read_s8(lua_State* luaState, u32 address)
{
  u8 rawVal = read_u8_from_domain_function(luaState, address);
  s8 signedByte = 0;
  memcpy(&signedByte, &rawVal, sizeof(s8));
  return signedByte;
}

u16 read_u16(lua_State* luaState, u32 address)
{
  return read_u16_from_domain_function(luaState, address);
}

s16 read_s16(lua_State* luaState, u32 address)
{
  u16 rawVal = read_u16_from_domain_function(luaState, address);
  s16 signedShort = 0;
  memcpy(&signedShort, &rawVal, sizeof(s16));
  return signedShort;
}

u32 read_u32(lua_State* luaState, u32 address)
{
  return read_u32_from_domain_function(luaState, address);
}

s32 read_s32(lua_State* luaState, u32 address)
{
  u32 rawVal = read_u32_from_domain_function(luaState, address);
  s32 signedInt = 0;
  memcpy(&signedInt, &rawVal, sizeof(s32));
  return signedInt;
}

u64 read_u64(lua_State* luaState, u32 address)
{
  return read_u64_from_domain_function(luaState, address);
}

s64 read_s64(lua_State* luaState, u32 address)
{
  u64 rawVal = read_u64_from_domain_function(luaState, address);
  s64 signedLongLong = 0;
  memcpy(&signedLongLong, &rawVal, sizeof(s64));
  return signedLongLong;
}

float read_float(lua_State* luaState, u32 address)
{
  u32 rawVal = read_u32_from_domain_function(luaState, address);
  float myFloat = 0.0;
  memcpy(&myFloat, &rawVal, sizeof(float));
  return myFloat;
}

double read_double(lua_State* luaState, u32 address)
{
  u64 rawVal = read_u64_from_domain_function(luaState, address);
  double myDouble = 0.0;
  memcpy(&myDouble, &rawVal, sizeof(double));
  return myDouble;
}

int do_read_into_byte_wrapper(lua_State* luaState, u32 address, u8 byteWrapperSize)
{
  std::optional<PowerPC::ReadResult<u64>> readResultSize64;
  std::optional<PowerPC::ReadResult<u32>> readResultSize32;
  std::optional<PowerPC::ReadResult<u16>> readResultSize16;
  std::optional<PowerPC::ReadResult<u8>> readResultSize8; 
  ByteWrapper* byteWrapperReturnResult = NULL;
  switch (byteWrapperSize)
  {
  case 8:
    readResultSize64 = PowerPC::HostTryReadU64(address);
    if (readResultSize64.has_value())
    {
      u64 u64Result = readResultSize64.value().value;
      byteWrapperReturnResult = ByteWrapper::CreateByteWrapperFromU64(u64Result);
    }
    break;

  case 4:
    readResultSize32 = PowerPC::HostTryReadU32(address);
    if (readResultSize32.has_value())
    {
      u32 u32Result = readResultSize32.value().value;
      byteWrapperReturnResult = ByteWrapper::CreateByteWrapperFromU32(u32Result);
    }
    break;

  case 2:
    readResultSize16 = PowerPC::HostTryReadU16(address);
    if (readResultSize16.has_value())
    {
      u16 u16Result = readResultSize16.value().value;
      byteWrapperReturnResult = ByteWrapper::CreateByteWrapperFromU16(u16Result);
    }
    break;

  case 1:
    readResultSize8 = PowerPC::HostTryReadU8(address);
    if (readResultSize8.has_value())
    {
      u8 u8Result = readResultSize8.value().value;
      byteWrapperReturnResult = ByteWrapper::CreateByteWrapperFromU8(u8Result);
    }
  default:
    luaL_error(luaState, "Error: Invalid ByteWrapper size passed into memory:readFrom()");
  }

  if (byteWrapperReturnResult == NULL)
  {
    luaL_error(luaState, "Error: Failed to read bytes into ByteWrapper in memory:readFrom()");
    return 0;
  }

  *reinterpret_cast<ByteWrapper**>(lua_newuserdata(luaState, sizeof(ByteWrapper*))) = byteWrapperReturnResult;
  luaL_setmetatable(luaState, LuaByteWrapper::LUA_BYTE_WRAPPER);
  return 1;
}

void write_u8(lua_State* luaState, u32 address, u8 value)
{
  write_u8_to_domain_function(luaState, address, value);
}

void write_s8(lua_State* luaState, u32 address, s8 value)
{
  u8 unsignedByte = 0;
  memcpy(&unsignedByte, &value, sizeof(s8));
  write_u8_to_domain_function(luaState, address, unsignedByte);
}

void write_u16(lua_State* luaState, u32 address, u16 value)
{
  write_u16_to_domain_function(luaState, address, value);
}

void write_s16(lua_State* luaState, u32 address, s16 value)
{
  u16 unsignedShort = 0;
  memcpy(&unsignedShort, &value, sizeof(s16));
  write_u16_to_domain_function(luaState, address, unsignedShort);
}

void write_u32(lua_State* luaState, u32 address, u32 value)
{
  write_u32_to_domain_function(luaState, address, value);
}

void write_s32(lua_State* luaState, u32 address, s32 value)
{
  u32 unsignedInt = 0;
  memcpy(&unsignedInt, &value, sizeof(s32));
  write_u32_to_domain_function(luaState, address, unsignedInt);
}

void write_u64(lua_State* luaState, u32 address, u64 value)
{
  write_u64_to_domain_function(luaState, address, value);
}

void write_s64(lua_State* luaState, u32 address, s64 value)
{
  u64 unsignedLongLong = 0;
  memcpy(&unsignedLongLong, &value, sizeof(s64));
  write_u64_to_domain_function(luaState, address, unsignedLongLong);
}

void write_float(lua_State* luaState, u32 address, float value)
{
  u32 unsignedInt = 0UL;
  memcpy(&unsignedInt, &value, sizeof(float));
  write_u32_to_domain_function(luaState, address, unsignedInt);
}

void write_double(lua_State* luaState, u32 address, double value)
{
  u64 unsignedLongLong = 0ULL;
  memcpy(&unsignedLongLong, &value, sizeof(double));
  write_u64_to_domain_function(luaState, address, unsignedLongLong);
}

int do_write_from_byte_wrapper(lua_State* luaState, u32 address, ByteWrapper* byteWrapperPointer, u8 numBytesRequired)
{
  if (byteWrapperPointer->totalBytesAllocated < numBytesRequired)
  {
    luaL_error(luaState, "Error: ByteWrapper was not big enough to write the specified number of bytes to memory in Memory:writeTo()");
    return 0;
  }
  u8 u8Val = 0;
  u16 u16Val = 0;
  u32 u32Val = 0;
  u64 u64Val = 0ULL;

  switch (numBytesRequired)
  {
  case 1:
    u8Val = byteWrapperPointer->getValueAsU8();
    write_u8(luaState, address, u8Val);
    return 0;

  case 2:
    u16Val = byteWrapperPointer->getValueAsU16();
    write_u16(luaState, address, u16Val);
    return 0;

  case 4:
    u32Val = byteWrapperPointer->getValueAsU32();
    write_u32(luaState, address, u32Val);
    return 0;

  case 8:
    u64Val = byteWrapperPointer->getValueAsU64();
    write_u64(luaState, address, u64Val);
    return 0;

  default:
    luaL_error(luaState, "Error: invalid size specified for number of bytes to write for ByteWrapper in Memory:writeTo() method. Valid sizes are 1, 2, 4 and 8");
    return 0;
  }
}

void luaColonOperatorTypeCheck(lua_State* luaState, const char* functionName, const char* exampleCall)
{
  if (lua_type(luaState, 1) != LUA_TUSERDATA)
  {
    luaL_error(luaState, (std::string("Error: User attempted to call ") + functionName + " function using the dot operator. Please use the colon operator instead like this: '" + exampleCall + "'").c_str());
  }
}
// End of helper functions block.

int do_general_read(lua_State* luaState) // format is: 1st argument after object is type string, 2nd argument after object is address. 3rd argument is number of bytes to include IF first argument was ByteWrapper. Otherwise, this argument is ignored.
{
  u8 byteWrapperSize = 255;
  u8 u8Val = 0;
  u16 u16Val = 0;
  u32 u32Val = 0LL;
  u64 u64Val = 0ULL;
  s8 s8Val = 0;
  s16 s16Val = 0;
  s32 s32Val = 0;
  s64 s64Val = 0ULL;
  float floatVal = 0.0;
  double doubleVal = 0.0;

  luaColonOperatorTypeCheck(luaState, "readFrom", "memory:readFrom(\"u32\", 0X80000043");

  const char* typeString = luaL_checkstring(luaState, 2);
  u32 address = luaL_checkinteger(luaState, 3);

  switch (ByteWrapper::parseType(typeString))
  {
    case ByteWrapper::ByteType::UNSIGNED_8:
      u8Val = read_u8(luaState, address);
      lua_pushinteger(luaState, static_cast<lua_Integer>(u8Val));
      return 1;

    case ByteWrapper::ByteType::UNSIGNED_16:
      u16Val = read_u16(luaState, address);
      lua_pushinteger(luaState, static_cast<lua_Integer>(u16Val));
      return 1;

    case ByteWrapper::ByteType::UNSIGNED_32:
      u32Val = read_u32(luaState, address);
      lua_pushinteger(luaState, static_cast<lua_Integer>(u32Val));
      return 1;

    case ByteWrapper::ByteType::UNSIGNED_64:
      u64Val = read_u64(luaState, address);
      lua_pushnumber(luaState, static_cast<lua_Number>(u64Val));
      return 1;

    case ByteWrapper::ByteType::SIGNED_8:
      s8Val = read_s8(luaState, address);
      lua_pushinteger(luaState, static_cast<lua_Integer>(s8Val));
      return 1;

    case ByteWrapper::ByteType::SIGNED_16:
      s16Val = read_s16(luaState, address);
      lua_pushinteger(luaState, static_cast<lua_Integer>(s16Val));
      return 1;

    case ByteWrapper::ByteType::SIGNED_32:
      s32Val = read_s32(luaState, address);
      lua_pushinteger(luaState, static_cast<lua_Integer>(s32Val));
      return 1;

    case ByteWrapper::ByteType::SIGNED_64:
      s64Val = read_s64(luaState, address);
      lua_pushnumber(luaState, static_cast<lua_Number>(s64Val));
      return 1;

    case ByteWrapper::ByteType::FLOAT:
      floatVal = read_float(luaState, address);
      lua_pushnumber(luaState, static_cast<lua_Number>(floatVal));
      return 1;

    case ByteWrapper::ByteType::DOUBLE:
      doubleVal = read_double(luaState, address);
      lua_pushnumber(luaState, static_cast<lua_Number>(doubleVal));
      return 1;

    case ByteWrapper::ByteType::WRAPPER:
      byteWrapperSize = luaL_checkinteger(luaState, 4);
      if (byteWrapperSize != 8 && byteWrapperSize != 4 && byteWrapperSize != 2 && byteWrapperSize != 1)
      {
        luaL_error(luaState, "Error: size of ByteWrapper passed into memory:readFrom() function must be either 1, 2, 4 or 8.");
        return 0;
      }
      return do_read_into_byte_wrapper(luaState, address, byteWrapperSize);

    default:
      luaL_error(luaState, "Error: Undefined type encountered in Memory:readFrom() function. Valid types are "
                 "u8, u16, u32, u64, s8, s16, s32, s64, float, double, unsigned_8, unsigned_16, "
                 "unsigned_32, unsigned_64, signed_8, signed_16, signed_32, signed_64, unsigned "
                 "byte, signed byte, unsigned int, signed int, unsigned long long, signed long "
                 "long, ByteWrapper and wrapper");
      return 0;
  }
}

int do_read_unsigned_bytes(lua_State* luaState)
{
  luaColonOperatorTypeCheck(luaState, "read_unsigned_bytes", "memory:read_unsigned_bytes(0X80000456, 44)");
  u32 address = luaL_checkinteger(luaState, 2);
  u32 numBytes = luaL_checkinteger(luaState, 3);
  u8* pointerToBaseAddress = get_pointer_to_domain_function(luaState, address);
  lua_createtable(luaState, numBytes, 0);
  for (size_t i = 0; i < numBytes; ++i)
  {
    u8 currByte = pointerToBaseAddress[i];
    lua_pushinteger(luaState, static_cast<lua_Integer>(currByte));
    lua_rawseti(luaState, -2, address + i);
  }
  return 1;
}

int do_read_signed_bytes(lua_State* luaState)
{
  luaColonOperatorTypeCheck(luaState, "read_signed_bytes", "memory:read_signed_bytes(0X800000065, 44)");
  u32 address = luaL_checkinteger(luaState, 2);
  u32 numBytes = luaL_checkinteger(luaState, 3);
  u8* pointerToBaseAddress = get_pointer_to_domain_function(luaState, address);
  lua_createtable(luaState, numBytes, 0);
  for (size_t i = 0; i < numBytes; ++i)
  {
    s8 currByteSigned = 0;
    memcpy(&currByteSigned, pointerToBaseAddress + i, sizeof(s8));
    lua_pushinteger(luaState, static_cast<lua_Integer>(currByteSigned));
    lua_rawseti(luaState, -2, address + i);
  }
  return 1;
}

int do_read_fixed_length_string(lua_State* luaState)
{
  luaColonOperatorTypeCheck(luaState, "read_fixed_length_string", "memory:read_fixed_length_string(0X8000043, 52)");
  u32 address = luaL_checkinteger(luaState, 2);
  u32 stringLength = luaL_checkinteger(luaState, 3);
  std::string returnResult = get_string_from_domain_function(luaState, address, stringLength);
  lua_pushfstring(luaState, returnResult.c_str());
  return 1;
}

int do_read_null_terminated_string(lua_State* luaState)
{
  luaColonOperatorTypeCheck(luaState, "read_null_termianted_string", "memory:read_null_terminated_string(0X80000043)");
  u32 address = luaL_checkinteger(luaState, 2);
  u8* memPointerStart = get_pointer_to_domain_function(luaState, address);
  u8* memPointerCurrent = memPointerStart;

  while (*memPointerCurrent != '\0')
    memPointerCurrent++;

  std::string returnResult = get_string_from_domain_function(
      luaState, address, (u32)(memPointerCurrent - memPointerStart));
  lua_pushfstring(luaState, returnResult.c_str());
  return 1;
}

int do_general_write(lua_State* luaState)
{
  u8 numBytesToWrite = 0;
  u8 u8Val = 0;
  s8 s8Val = 0;
  u16 u16Val = 0;
  s16 s16Val = 0;
  u32 u32Val = 0;
  s32 s32Val = 0;
  float floatVal = 0.0;
  u64 u64Val = 0ULL;
  s64 s64Val;
  double doubleVal = 0;
  ByteWrapper* byteWrapperPointer = NULL;

  luaColonOperatorTypeCheck(luaState, "writeTo", "memory:writeTo(\"u32\", 0X8000045, 34000)");
  const char* typeString = luaL_checkstring(luaState, 2);
  u32 address = luaL_checkinteger(luaState, 3);

  switch (ByteWrapper::parseType(typeString))
  {
  case ByteWrapper::ByteType::UNSIGNED_8:
    u8Val = luaL_checkinteger(luaState, 4);
    write_u8(luaState, address, u8Val);
    return 0;

  case ByteWrapper::ByteType::SIGNED_8:
    s8Val = luaL_checkinteger(luaState, 4);
    write_s8(luaState, address, s8Val);
    return 0;

  case ByteWrapper::ByteType::UNSIGNED_16:
    u16Val = luaL_checkinteger(luaState, 4);
    write_u16(luaState, address, u16Val);
    return 0;

  case ByteWrapper::ByteType::SIGNED_16:
    s16Val = luaL_checkinteger(luaState, 4);
    write_s16(luaState, address, s16Val);
    return 0;

  case ByteWrapper::ByteType::UNSIGNED_32:
    u32Val = luaL_checkinteger(luaState, 4);
    write_u32(luaState, address, u32Val);
    return 0;

  case ByteWrapper::ByteType::SIGNED_32:
    s32Val = luaL_checkinteger(luaState, 4);
    write_s32(luaState, address, s32Val);
    return 0;

  case ByteWrapper::ByteType::FLOAT:
    floatVal = luaL_checknumber(luaState, 4);
    write_float(luaState, address, floatVal);
    return 0;

  case ByteWrapper::ByteType::UNSIGNED_64:
    u64Val = luaL_checknumber(luaState, 4);
    write_u64(luaState, address, u64Val);
    return 0;

  case ByteWrapper::ByteWrapper::SIGNED_64:
    s64Val = luaL_checknumber(luaState, 4);
    write_s64(luaState, address, s64Val);
    return 0;

  case ByteWrapper::ByteType::DOUBLE:
    doubleVal = luaL_checknumber(luaState, 4);
    write_double(luaState, address, doubleVal);
    return 0;

  case ByteWrapper::ByteType::WRAPPER:
    byteWrapperPointer = (*reinterpret_cast<ByteWrapper**>(luaL_checkudata(luaState, 4, LuaByteWrapper::LUA_BYTE_WRAPPER)));
    // If the 5th argument is present, then we use this integer as the number of bytes to write from the ByteWrapper.
    // Otherwise, we write out the full number of bytes stored in the ByteWrapper.
    if (lua_gettop(luaState) >= 5)
      numBytesToWrite = luaL_checkinteger(luaState, 5);
    else
      numBytesToWrite = byteWrapperPointer->getByteSize();
    return do_write_from_byte_wrapper(luaState, address, byteWrapperPointer, numBytesToWrite);

  default:
    luaL_error(luaState, "Error: undefined type passed into Memory:writeTo() method. Valid types "
                         "include u8, u16, u32, u64, s8, s16, s32, s64, float, double and ByteWrapper");
    return 0;
  }
}

int do_write_bytes(lua_State* luaState)
{
  luaColonOperatorTypeCheck(luaState, "write_bytes", "memory:write_bytes(arrayName)");
  lua_pushnil(luaState); /* first key */
  while (lua_next(luaState, 2) != 0)
  {
    /* uses 'key' (at index -2) and 'value' (at index -1) */
    u32 address = luaL_checkinteger(luaState, -2);
    s32 value = luaL_checkinteger(luaState, -1);

    if (value < -128 || value > 255)
    {
      luaL_error(luaState, (std::string("Error: Invalid number passed into memory:write_bytes() function. In order to be representable as a byte, the value must be between -128 and 255. However, the number which was supposed to be written to address " + std::to_string(address) + " was " + std::to_string(value)).c_str()));
      return 0;
    }
    if (value < 0)
    {
      s8 s8Val = value;
      write_s8(luaState, address, s8Val);
    }
    else
    {
      u8 u8Val = value;
      write_u8(luaState, address, u8Val);
    }
    /* removes 'value'; keeps 'key' for next iteration */
    lua_pop(luaState, 1);
  }
  return 0;
}

int do_write_string(lua_State* luaState)
{
  luaColonOperatorTypeCheck(luaState, "write_string", "memory:write_fixed_length_string(0X80000043, \"exampleString\")");
  u32 address = luaL_checkinteger(luaState, 2);
  const char* stringToWrite = luaL_checkstring(luaState, 3);
  size_t stringSize = strlen(stringToWrite);
  copy_string_to_domain_function(luaState, address, stringToWrite, stringSize);
  return 0;
}

}  // namespace LuaMemoryApi
}  // namespace Lua