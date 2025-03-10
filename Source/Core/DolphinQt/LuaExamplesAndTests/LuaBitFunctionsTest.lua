require ("bit")

testNum = 1
resultsTable = {}
resultsTable["PASS"] = 0
resultsTable["FAIL"] = 0

function doTest2Args(actualValue, expectedValue, firstValue, secondValue, functionName, operatorValue)
	io.write("Test " .. tostring(testNum) .. ":\n")
	testNum = testNum + 1
	actualExpression = "bit:" .. functionName .. "(" .. tostring(firstValue) .. ", " .. tostring(secondValue) .. ")"
	baseExpression =  tostring(firstValue) .. " " .. operatorValue .. " " .. tostring(secondValue)
	io.write("\tTesting if " .. actualExpression .. " == " .. baseExpression .. ":\n")
	if (actualValue == expectedValue) then
		io.write("\t" .. tostring(actualValue) .. " == " .. tostring(expectedValue) .. "\nPASS!\n\n")
		resultsTable["PASS"] = resultsTable["PASS"] + 1
	else
		io.write("\t" .. tostring(actualValue) .. " != " .. tostring(expectedValue) .. "\nFAILURE! Actual value did not equal expected value!\n\n")
		resultsTable["FAIL"] = resultsTable["FAIL"] + 1
	end
end

function doTest1Arg(actualValue, expectedValue, inputValue, functionName, operatorValue) 
	io.write("Test " .. tostring(testNum) .. ":\n")
	testNum = testNum + 1
	actualExpression = "bit:" .. functionName .. "(" .. tostring(inputValue) .. ")"
	baseExpression = operatorValue .. tostring(inputValue)
	io.write("\tTesting if " .. actualExpression .. " == " .. baseExpression .. ":\n")
	if (actualValue == expectedValue) then
		io.write("\t" .. tostring(actualValue) .. " == " .. tostring(expectedValue) .. "\nPASS!\n\n")
		resultsTable["PASS"] = resultsTable["PASS"] + 1
	else
		io.write("\t" .. tostring(actualValue) .. " != " .. tostring(expectedValue) .. "\nFAILURE! Actual value did not equal expected value!\n\n")
		resultsTable["FAIL"] = resultsTable["FAIL"] + 1
	end
end

function LuaBitTests()
	firstMainInteger = 113
	secondMainInteger = 546

	firstBitShiftArg = 13
	secondBitShiftArg = 2
	
	firstBool = true
	secondBool = false

	doTest2Args(bit:bitwise_and(firstMainInteger, secondMainInteger), firstMainInteger & secondMainInteger, firstMainInteger, secondMainInteger, "bitwise_and", "&")
	doTest2Args(bit:bitwise_or(firstMainInteger, secondMainInteger), firstMainInteger | secondMainInteger, firstMainInteger, secondMainInteger, "bitwise_or", "|")
	doTest2Args(bit:bitwise_xor(firstMainInteger, secondMainInteger), firstMainInteger ~ secondMainInteger, firstMainInteger, secondMainInteger, "bitwise_xor", "^")
	doTest2Args(bit:logical_and(firstBool, secondBool), firstBool and secondBool, firstBool, secondBool, "logical_and", "&&")
	doTest2Args(bit:logical_or(firstBool, secondBool), firstBool or secondBool, firstBool, secondBool, "logical_or", "||")
	doTest2Args(bit:logical_xor(firstBool, secondBool), ((firstBool or secondBool) and not (firstBool and secondBool)), firstBool, secondBool, "logical_xor", "XOR")
	doTest2Args(bit:bit_shift_left(firstBitShiftArg, secondBitShiftArg), firstBitShiftArg << secondBitShiftArg, firstBitShiftArg, secondBitShiftArg, "bit_shift_left", "<<")
	doTest2Args(bit:bit_shift_right(firstBitShiftArg, secondBitShiftArg), firstBitShiftArg >> secondBitShiftArg, firstBitShiftArg, secondBitShiftArg, "bit_shift_right", ">>")	
	
	doTest1Arg(bit:bitwise_not(firstMainInteger), ~firstMainInteger, firstMainInteger, "bitwise_not", "~")
	doTest1Arg(bit:logical_not(firstBool), not firstBool, firstBool, "logical_not", "!")

	io.write("Total Tests: " .. tostring(testNum - 1) .. "\n")
	io.write("\tTests Passed: " .. tostring(resultsTable["PASS"]) .. "\n")
	io.write("\tTests Failed: " .. tostring(resultsTable["FAIL"]) .. "\n")
	print("Total Tests: " .. tostring(testNum - 1) .. "\n")
	print("\tTests Passed: " .. tostring(resultsTable["PASS"]) .. "\n")
	print("\tTests Failed: " .. tostring(resultsTable["FAIL"]) .. "\n")
end




file = io.open("LuaExamplesAndTests/TestResults/LuaBitTestsResults.txt", "w")
io.output(file)
LuaBitTests()
io.flush()
io.close()
