/*
	Read data from the engine control unit simtec 56.5 with the engine or similar

	Engines: X20XEV, X18XE
	Cylinder: is currently only for 4 cylinders (Knock Delay)

	- Opel Omega B
	- Opel Vectra B
	.....
	
	All cars with the Simtec 56.5 control unit should be able to read out the library

---------------------------------------------------------------------

	The source file from this library is here whit all function CAN:
	https://github.com/PowerBroker2/ELMduino

	Thx for Write all PowerBroker2

*/


#include "ELMDuino_Simtec_565.h"




/*
 bool ELM327_Simtec_565::begin(Stream &stream, const bool& debug, const uint16_t& timeout, const char& protocol, const uint16_t& payloadLen)

 Description:
 ------------
  * Constructor for the ELM327 Class; initializes ELM327

 Inputs:
 -------
  * Stream &stream      - Reference to Serial port connected to ELM327
  * int SelectProtocol  - Select protocol show below
  * bool debug          - Specify whether or not to print debug statements to "Serial"
  * uint16_t timeout    - Time in ms to wait for a query response
  * char protocol       - Protocol ID to specify the ELM327 to communicate with the ECU over
  * uint16_t payloadLen - Maximum number of bytes expected to be returned by the ELM327 after a query
 Return:
 -------
  * bool - Whether or not the ELM327 was properly initialized

 Notes:
 ------
  * Protocol - Description
  * 4        - ISO 14230-4 KWP (5 baud init)
  * 5        - ISO 14230-4 KWP (fast init)

  * --> *user adjustable
*/
bool ELM327_Simtec_565::begin(Stream &stream, int SelectProtocol, const bool &debug, const uint16_t &timeout, const uint16_t &payloadLen)
{
	elm_port    = &stream;
	PAYLOAD_LEN = payloadLen;
	debugMode   = debug;
	timeout_ms  = timeout;

	payload = (char *)malloc(PAYLOAD_LEN + 1); // allow for terminating '\0'

	// test if serial port is connected
	if (!elm_port)
		return false;

	// try to connect
	if (!initializeELM(SelectProtocol))
		return false;

	return true;
}




/*
 bool ELM327_Simtec_565::initializeELM()

 Description:
 ------------
  * Initializes ELM327

 Inputs:
 -------
  * SelectProtocol - show below Protocol

 Return:
 -------
  * bool - Whether or not the ELM327 was propperly initialized

 Notes:
 ------
  * Protocol - Description
  * 4        - ISO 14230-4 KWP (5 baud init)
  * 5        - ISO 14230-4 KWP (fast init)

  * --> *user adjustable
*/
bool ELM327_Simtec_565::initializeELM(int SelectProtocol)
{
	char command[10] = {'\0'};
	connected = false;

	sendCommand_Blocking(SET_ALL_TO_DEFAULTS);
	delay(100);

	sendCommand_Blocking(RESET_ALL);
	delay(100);

	sendCommand_Blocking(ECHO_OFF);
	delay(100);

	sendCommand_Blocking(HEADERS_ON);
	delay(100);

	sendCommand_Blocking(PRINTING_SPACES_OFF);
	delay(100);

	sendCommand_Blocking(LINEFEEDS_OFF);
	delay(100);

	sendCommand_Blocking(ALLOW_LONG_MESSAGES);
	delay(100);
	
	sendCommand_Blocking(MEMORY_OFF);
	delay(100);

	sendCommand_Blocking(ADAPTIVE_TIMING_AUTO_1);
	delay(100);

	// Set header
	sprintf(command, SET_HEADER, "8211F1");
	sendCommand_Blocking(command);
	delay(100);

	// Set data timeout
	sprintf(command, SET_TIMEOUT_TO_H_X_4MS, "64");
	sendCommand_Blocking(command);
	delay(100);

	// Set wakeup timeout
	sprintf(command, SET_WAKEUP_TO_H_X_20MS, "00");
	sendCommand_Blocking(command);
	delay(100);

	// Set protocol and save
	sprintf(command, SET_PROTOCOL_TO_H_SAVE, SelectProtocol);

	if (sendCommand_Blocking(command) == ELM_SUCCESS)
	{
		if (strstr(payload, "OK") != NULL)
		{
			connected = true;Vehicle Indification Number (VIN)
	// Bus Init: OK
	sendCommand_Blocking("81");
	delay(100);

	return connected;
}




/*
 void ELM327_Simtec_565::formatQueryArray(uint8_t service, uint16_t pid, uint8_t num_responses)

 Description:
 ------------
  * Creates a query stack to be sent to ELM327

 Inputs:
 -------
  * uint16_t service - Service number of the queried PID
  * uint32_t pid     - PID number of the queried PID
  * uint8_t num_responses - see function header for "queryPID()"

 Return:
 -------
  * void
*/
void ELM327_Simtec_565::formatQueryArray(uint8_t service, uint16_t pid, uint8_t num_responses)
{
	if (debugMode)
	{
		Serial.print(F("Service: "));
		Serial.println(service);
		Serial.print(F("PID: "));
		Serial.println(pid);
	}

	query[0] = ((service >> 4) & 0xF) + '0';
	query[1] = (service & 0xF) + '0';

	// determine PID length (standard queries have 16-bit PIDs,
	// but some custom queries have PIDs with 32-bit values)
	if (pid & 0xFF00)
	{
		if (debugMode)
			Serial.println(F("Long query detected"));

		longQuery = true;

		query[2] = ((pid >> 12) & 0xF) + '0';
		query[3] = ((pid >> 8) & 0xF) + '0';
		query[4] = ((pid >> 4) & 0xF) + '0';
		query[5] = (pid & 0xF) + '0';
		query[6] = num_responses + '0';
		query[7] = '\0';

		upper(query, 6);
	}
	else
	{
		if (debugMode)
			Serial.println(F("Normal length query detected"));

		longQuery = false;

		query[2] = ((pid >> 4) & 0xF) + '0';
		query[3] = (pid & 0xF) + '0';
		query[4] = num_responses + '0';
		query[5] = '\0';

		upper(query, 4);
	}

	if (debugMode)
	{
		Serial.print(F("Query string: "));
		Serial.println(query);
	}
}




/*
 void ELM327_Simtec_565::upper(char string[], uint8_t buflen)

 Description:
 ------------
  * Converts all elements of char array string[] to
  uppercase ascii

 Inputs:
 -------
  * uint8_t string[] - Char array
  * uint8_t buflen   - Length of char array

 Return:
 -------
  * void
*/
void ELM327_Simtec_565::upper(char string[], uint8_t buflen)
{
	for (uint8_t i = 0; i < buflen; i++)
	{
		if (string[i] > 'Z')
			string[i] -= 32;
		else if ((string[i] > '9') && (string[i] < 'A'))
			string[i] += 7;
	}
}




/*
 bool ELM327_Simtec_565::timeout()

 Description:
 ------------
  * Determines if a time-out has occurred

 Inputs:
 -------
  * void

 Return:
 -------
  * bool - whether or not a time-out has occurred
*/
bool ELM327_Simtec_565::timeout()
{
	currentTime = millis();
	if ((currentTime - previousTime) >= timeout_ms)
		return true;
	return false;
}




/*
 uint8_t ELM327_Simtec_565::ctoi(uint8_t value)

 Description:
 ------------
  * converts a decimal or hex char to an int

 Inputs:
 -------
  * uint8_t value - char to be converted

 Return:
 -------
  * uint8_t - int value of parameter "value"
*/
uint8_t ELM327_Simtec_565::ctoi(uint8_t value)
{
	if (value >= 'A')
		return value - 'A' + 10;
	else
		return value - '0';
}




/*
 int8_t ELM327_Simtec_565::nextIndex(char const *str,
                          char const *target,
                          uint8_t numOccur)

 Description:
 ------------
  * Finds and returns the first char index of
  numOccur'th instance of target in str

 Inputs:
 -------
  * char const *str    - string to search target within
  * char const *target - String to search for in str
  * uint8_t numOccur   - Which instance of target in str
  
 Return:
 -------
  * int8_t - First char index of numOccur'th
  instance of target in str. -1 if there is no
  numOccur'th instance of target in str
*/
int8_t ELM327_Simtec_565::nextIndex(char const *str,
												 char const *target,
												 uint8_t numOccur = 1)
{
	char const *p = str;
	char const *r = str;
	uint8_t count;

	for (count = 0;; ++count)
	{
		p = strstr(p, target);

		if (count == (numOccur - 1))
			break;

		if (!p)
			break;

		p++;
	}

	if (!p)
		return -1;

	return p - r;
}




/*
 void ELM327_Simtec_565::flushInputBuff()

 Description:
 ------------
  * Flushes input serial buffer

 Inputs:
 -------
  * void

 Return:
 -------
  * void
*/
void ELM327_Simtec_565::flushInputBuff()
{
	if (debugMode)
		Serial.println(F("Clearing input serial buffer"));

	while (elm_port->available())
		elm_port->read();
}




/*
  bool ELM327_Simtec_565::queryPID(const uint8_t& service, const uint16_t& pid, const uint8_t& num_responses)

  Description:
  ------------
  * create a PID query command string and send the command

  Inputs:
  -------
  * uint8_t service       - The diagnostic service ID. 01 is "Show current data"
  * uint16_t pid          - The Parameter ID (PID) from the service
  * uint8_t num_responses - Number of lines of data to receive - see ELM datasheet "Talking to the vehicle".
                            This can speed up retrieval of information if you know how many responses will be sent.
                            Basically the OBD scanner will not wait for more responses if it does not need to go through 
                            final timeout. Also prevents OBD scanners from sending mulitple of the same response.

  Return:
  -------
  * bool - Whether or not the query was submitted successfully
*/
bool ELM327_Simtec_565::queryPID(const uint8_t& service, const uint16_t& pid, const uint8_t& num_responses)
{
	formatQueryArray(service, pid, num_responses);
	sendCommand(query);

	return connected;
}




/*
 bool ELM327_Simtec_565::queryPID(char queryStr[])

 Description:
 ------------
  * Queries ELM327 for a specific type of vehicle telemetry data

 Inputs:
 -------
  * char queryStr[] - Query string (service and PID)

 Return:
 -------
  * bool - Whether or not the query was submitted successfully
*/
bool ELM327_Simtec_565::queryPID(char queryStr[])
{
	if (strlen(queryStr) <= 4)
		longQuery = false;
	else
		longQuery = true;

	sendCommand(queryStr);

	return connected;
}




/*
 void ELM327_Simtec_565::sendCommand(const char *cmd)

 Description:
 ------------
  * Sends a command/query for Non-Blocking PID queries

 Inputs:
 -------
  * const char *cmd - Command/query to send to ELM327

 Return:
 -------
  * void
*/
void ELM327_Simtec_565::sendCommand(const char *cmd)
{
	// clear payload buffer
	memset(payload, '\0', PAYLOAD_LEN + 1);

	// reset input serial buffer and number of received bytes
	recBytes = 0;
	flushInputBuff();
	connected = false;

	// Reset the receive state ready to start receiving a response message
	nb_rx_state = ELM_GETTING_MSG;

	if (debugMode)
	{
		Serial.print(F("Sending the following command/query: "));
		Serial.println(cmd);
	}

	elm_port->print(cmd);
	elm_port->print('\r');

	// prime the timeout timer
	previousTime = millis();
	currentTime = previousTime;
}




/*
 obd_rx_states ELM327_Simtec_565::sendCommand_Blocking(const char* cmd)

 Description:
 ------------
	* Sends a command/query and waits for a respoonse (blocking function)
    Sometimes it's desirable to use a blocking command, e.g when sending an AT command.
    This function removes the need for the caller to set up a loop waiting for the command to finish.
    Caller is free to parse the payload string if they need to use the response.

 Inputs:
 -------
  * const char *cmd - Command/query to send to ELM327

 Return:
 -------
  * int8_t - the ELM_XXX status of getting the OBD response
*/
int8_t ELM327_Simtec_565::sendCommand_Blocking(const char *cmd)
{
	sendCommand(cmd);
	while (get_response() == ELM_GETTING_MSG);
	return nb_rx_state;
}




/*
 obd_rx_states ELM327_Simtec_565::get_response(void)

 Description:
 ------------
  * Non Blocking (NB) receive OBD scanner response. Must be called repeatedly until
    the status progresses past ELM_GETTING_MSG.

 Inputs:
 -------
  * void

 Return:
 -------
  * int8_t - the ELM_XXX status of getting the OBD response
*/
int8_t ELM327_Simtec_565::get_response(void)
{
	// buffer the response of the ELM327 until either the
	// end marker is read or a timeout has occurred
	// last valid idx is PAYLOAD_LEN but want to keep one free for terminating '\0'
	// so limit counter to < PAYLOAD_LEN
	if (!elm_port->available())
	{
		nb_rx_state = ELM_GETTING_MSG;
		if (timeout())
			nb_rx_state = ELM_TIMEOUT;
	}
	else
	{
		char recChar = elm_port->read();

		if (debugMode)
		{
			Serial.print(F("\tReceived char: "));
			// display each received character, make non-printables printable
			if (recChar == '\f')
				Serial.println(F("\\f"));
			else if (recChar == '\n')
				Serial.println(F("\\n"));
			else if (recChar == '\r')
				Serial.println(F("\\r"));
			else if (recChar == '\t')
				Serial.println(F("\\t"));
			else if (recChar == '\v')
				Serial.println(F("\\v"));
			// convert spaces to underscore, easier to see in debug output
			else if (recChar == ' ')
				Serial.println("_");
			// display regular printable
			else
				Serial.println(recChar);
		}

		// this is the end of the OBD response
		if (recChar == '>')
		{
			if (debugMode)
				Serial.println(F("Delimiter found."));

			nb_rx_state = ELM_MSG_RXD;
		}
		else if (!isalnum(recChar) && (recChar != ':') && (recChar != '.'))
			// discard all characters except for alphanumeric and decimal places.
			// decimal places needed to extract floating point numbers, e.g. battery voltage
			nb_rx_state = ELM_GETTING_MSG; // Discard this character
		else
		{
			if (recBytes < PAYLOAD_LEN)
			{
				payload[recBytes] = recChar;
				recBytes++;
				nb_rx_state = ELM_GETTING_MSG;
			}
			else
				nb_rx_state = ELM_BUFFER_OVERFLOW;
		}
	}

	// Message is still being received (or is timing out), so exit early without doing all the other checks
	if (nb_rx_state == ELM_GETTING_MSG)
		return nb_rx_state;

	// End of response delimiter was found
	if (debugMode && nb_rx_state == ELM_MSG_RXD)
	{
		Serial.print(F("All chars received: "));
		Serial.println(payload);
	}

	if (nb_rx_state == ELM_TIMEOUT)
	{
		if (debugMode)
		{
			Serial.print(F("Timeout detected with overflow of "));
			Serial.print((currentTime - previousTime) - timeout_ms);
			Serial.println(F("ms"));
		}
		return nb_rx_state;
	}

	if (nb_rx_state == ELM_BUFFER_OVERFLOW)
	{
		if (debugMode)
		{
			Serial.print(F("OBD receive buffer overflow (> "));
			Serial.print(PAYLOAD_LEN);
			Serial.println(F(" bytes)"));
		}
		return nb_rx_state;
	}

	// Now we have successfully received OBD response, check if the payload indicates any OBD errors
	if (nextIndex(payload, "UNABLETOCONNECT") >= 0)
	{
		if (debugMode)
			Serial.println(F("ELM responded with errror \"UNABLE TO CONNECT\""));

		nb_rx_state = ELM_UNABLE_TO_CONNECT;
		return nb_rx_state;
	}

	connected = true;

	if (nextIndex(payload, "NODATA") >= 0)
	{
		if (debugMode)
			Serial.println(F("ELM responded with errror \"NO DATA\""));

		nb_rx_state = ELM_NO_DATA;
		return nb_rx_state;
	}

	if (nextIndex(payload, "STOPPED") >= 0)
	{
		if (debugMode)
			Serial.println(F("ELM responded with errror \"STOPPED\""));

		nb_rx_state = ELM_STOPPED;
		return nb_rx_state;
	}

	if (nextIndex(payload, "ERROR") >= 0)
	{
		if (debugMode)
			Serial.println(F("ELM responded with \"ERROR\""));

		nb_rx_state = ELM_GENERAL_ERROR;
		return nb_rx_state;
	}

	nb_rx_state = ELM_SUCCESS;
	return nb_rx_state;
}




/*
 uint64_t ELM327_Simtec_565::findResponse()

 Description:
 ------------
  * Parses the buffered ELM327's response and returns the queried data

 Inputs:
 -------
  * void

 Return:
 -------
  * uint64_t - Query response value
*/
uint64_t ELM327_Simtec_565::findResponse()
{
	uint8_t firstDatum = 0;
	char header[7] = {'\0'};

	if (longQuery)
	{
		header[0] = query[0] + 4;
		header[1] = query[1];
		header[2] = query[2];
		header[3] = query[3];
		header[4] = query[4];
		header[5] = query[5];
	}
	else
	{
		header[0] = query[0] + 4;
		header[1] = query[1];
		header[2] = query[2];
		header[3] = query[3];
	}

	if (debugMode)
	{
		Serial.print(F("Expected response header: "));
		Serial.println(header);
	}

	int8_t firstHeadIndex = nextIndex(payload, header);
	int8_t secondHeadIndex = nextIndex(payload, header, 2);

	if (firstHeadIndex >= 0)
	{
		if (longQuery)
			firstDatum = firstHeadIndex + 6;
		else
			firstDatum = firstHeadIndex + 4;

		// Some ELM327s (such as my own) respond with two
		// "responses" per query. "numPayChars" represents the
		// correct number of bytes returned by the ELM327
		// regardless of how many "responses" were returned
		if (secondHeadIndex >= 0)
		{
			if (debugMode)
				Serial.println(F("Double response detected"));

			numPayChars = secondHeadIndex - firstDatum;
		}
		else
		{
			if (debugMode)
				Serial.println(F("Single response detected"));

			numPayChars = recBytes - firstDatum;
		}

		response = 0;
		for (uint8_t i = 0; i < numPayChars; i++)
		{
			uint8_t payloadIndex = firstDatum + i;
			uint8_t bitsOffset = 4 * (numPayChars - i - 1);
			response = response | (ctoi(payload[payloadIndex]) << bitsOffset);
		}

		// It is useful to have the response bytes
		// broken-out because some PID algorithms (standard
		// and custom) require special operations for each
		// byte returned
		responseByte_0 = response & 0xFF;
		responseByte_1 = (response >> 8) & 0xFF;
		responseByte_2 = (response >> 16) & 0xFF;
		responseByte_3 = (response >> 24) & 0xFF;
		responseByte_4 = (response >> 32) & 0xFF;
		responseByte_5 = (response >> 40) & 0xFF;
		responseByte_6 = (response >> 48) & 0xFF;
		responseByte_7 = (response >> 56) & 0xFF;

		if (debugMode)
		{
			Serial.println(F("64-bit response: "));
			Serial.print(F("\tresponseByte_0: "));
			Serial.println(responseByte_0);
			Serial.print(F("\tresponseByte_1: "));
			Serial.println(responseByte_1);
			Serial.print(F("\tresponseByte_2: "));
			Serial.println(responseByte_2);
			Serial.print(F("\tresponseByte_3: "));
			Serial.println(responseByte_3);
			Serial.print(F("\tresponseByte_4: "));
			Serial.println(responseByte_4);
			Serial.print(F("\tresponseByte_5: "));
			Serial.println(responseByte_5);
			Serial.print(F("\tresponseByte_6: "));
			Serial.println(responseByte_6);
			Serial.print(F("\tresponseByte_7: "));
			Serial.println(responseByte_7);
		}

		return response;
	}

	if (debugMode)
		Serial.println(F("Response not detected"));

	return 0;
}




/*
 void ELM327_Simtec_565::printError()

 Description:
 ------------
  * Prints appropriate error description if an error has occurred

 Inputs:
 -------
  * void

 Return:
 -------
  * void
*/
void ELM327_Simtec_565::printError()
{
	Serial.print(F("Received: "));
	Serial.println(payload);

	if (nb_rx_state == ELM_SUCCESS)
		Serial.println(F("ELM_SUCCESS"));
	else if (nb_rx_state == ELM_NO_RESPONSE)
		Serial.println(F("ERROR: ELM_NO_RESPONSE"));
	else if (nb_rx_state == ELM_BUFFER_OVERFLOW)
		Serial.println(F("ERROR: ELM_BUFFER_OVERFLOW"));
	else if (nb_rx_state == ELM_UNABLE_TO_CONNECT)
		Serial.println(F("ERROR: ELM_UNABLE_TO_CONNECT"));
	else if (nb_rx_state == ELM_NO_DATA)
		Serial.println(F("ERROR: ELM_NO_DATA"));
	else if (nb_rx_state == ELM_STOPPED)
		Serial.println(F("ERROR: ELM_STOPPED"));
	else if (nb_rx_state == ELM_TIMEOUT)
		Serial.println(F("ERROR: ELM_TIMEOUT"));
	else if (nb_rx_state == ELM_BUFFER_OVERFLOW)
		Serial.println(F("ERROR: BUFFER OVERFLOW"));
	else if (nb_rx_state == ELM_GENERAL_ERROR)
		Serial.println(F("ERROR: ELM_GENERAL_ERROR"));
	else
		Serial.println(F("No error detected"));

	delay(100);
}




/*
 int ELM327_Simtec_565::ConvertHextoMathvalue(char HexValue1, char HexValue2)

 Description:
 ------------
  * Convert HEX value in Decimal value
  Example: HEX: 79 -> DEC: 121

 Inputs:
 -------
  * char HexValue1
  * char HexValue2

 Return:
 -------
  * int - The Converted Decimal value
*/
int ELM327_Simtec_565::ConvertHextoMathvalue(char HexValue1, char HexValue2)
{
	if (HexValue1 != '\0' && HexValue2 != '\0')
	{
		char Buffer [2] = { HexValue1, HexValue2 };

		// Convert Hex to Decimal
		int BufferComb = strtoul(Buffer, NULL, 16);

		if (debugMode)
		{
			Serial.print("values: ");
			Serial.print(HexValue1);
			Serial.print(HexValue2);
			Serial.print(" Finish: ");
			Serial.println(BufferComb);
		}
		return BufferComb;
	}
	return 0;
}




/*
 float ELM327_Simtec_565::getECU_VCC()

 Description:
 ------------
	* Read the ECU VCC (Volt)

 Return:
 -------
  * float
*/
float ELM327_Simtec_565::getECU_VCC()
{
	bool isOkay = get2101Data();
	if (isOkay)
	{
		int Select1 = 18; // HEX Select Value 1
		int Select2 = 19; // HEX Select Value 2
		float Math1 = 25.5; // Calculator Param 1
		int Math2 = 255; // Calculator Param 2
		int BackData = ConvertHextoMathvalue(ClearArray[Select1], ClearArray[Select2]); // Convert to Decimal
		float MathValue = (BackData * Math1) / Math2; // Calculation

		if (debugMode)
		{
			Serial.print("ECU VCC: ");
			Serial.println(MathValue);
		}

		return MathValue;
	}
	else
	{
		if (debugMode)
			Serial.println("ECU VCC not found");

		return -1;
	}
}




/*
 int ELM327_Simtec_565::getMOTOR_RPM()

 Description:
 ------------
	* Read the Motor RPM

 Return:
 -------
  * int
*/
int ELM327_Simtec_565::getMOTOR_RPM()
{
	bool isOkay = get2101Data();
	if (isOkay)
	{
		int Select1 = 36; // HEX Select Value 1
		int Select2 = 37; // HEX Select Value 2
		int Math1 = 8160; // Calculator Param 1
		int Math2 = 255; // Calculator Param 2
		int BackData = ConvertHextoMathvalue(ClearArray[Select1], ClearArray[Select2]); // Convert to Decimal
		int MathValue = (BackData * Math1) / Math2; // Calculation

		if (debugMode)
		{
			Serial.print("MOTOR RPM: ");
			Serial.println(MathValue);
		}

		return MathValue;
	}
	else
	{
		if (debugMode)
			Serial.println("MOTOR RPM not found");

		return -1;
	}
}




/*
 int ELM327_Simtec_565::getCAR_SPEED()

 Description:
 ------------
	* Read the Car Speed

 Return:
 -------
  * int
*/
int ELM327_Simtec_565::getCAR_SPEED()
{
	bool isOkay = get2101Data();
	if (isOkay)
	{
		int Select1 = 70; // HEX Select Value 1
		int Select2 = 71; // HEX Select Value 2
		int BackData = ConvertHextoMathvalue(ClearArray[Select1], ClearArray[Select2]); // Convert to Decimal

		if (debugMode)
		{
			Serial.print("CAR SPEED: ");
			Serial.println(BackData);
		}

		return BackData;
	}
	else
	{
		if (debugMode)
			Serial.println("CAR SPEED not found");

		return -1;
	}
}




/*
 float ELM327_Simtec_565::getINJECTOR_PULSE()

 Description:
 ------------
	* Read the Injector Pulse

 Return:
 -------
  * float
*/
float ELM327_Simtec_565::getINJECTOR_PULSE()
{
	bool isOkay = get2101Data();
	if (isOkay)
	{
		int Select1 = 38; // HEX Select Value 1
		int Select2 = 39; // HEX Select Value 2
		float Math1 = 261.11; // Calculator Param 1
		int Math2 = 255; // Calculator Param 2
		int BackData = ConvertHextoMathvalue(ClearArray[Select1], ClearArray[Select2]); // Convert to Decimal
		float MathValue = (BackData * Math1) / Math2; // Calculation

		Select1 = 46; // HEX Select Value 1
		Select2 = 47; // HEX Select Value 2
		Math1 = 1.02; // Calculator Param 1
		Math2 = 255; // Calculator Param 2
		BackData = ConvertHextoMathvalue(ClearArray[Select1], ClearArray[Select2]); // Convert to Decimal
		float MathValue1 = (BackData * Math1) / Math2; // Calculation

		if (debugMode)
		{
			Serial.print("INJECTOR PULSE: ");
			Serial.println(MathValue + MathValue1);
		}

		return MathValue + MathValue1;
	}
	else
	{
		if (debugMode)
			Serial.println("INJECTOR PULSE not found");

		return -1;
	}
}




/*
 int ELM327_Simtec_565::getREQUIRED_MOTOR_RPM()

 Description:
 ------------
	* Read the Required Motor RPM

 Return:
 -------
  * int
*/
int ELM327_Simtec_565::getREQUIRED_MOTOR_RPM()
{
	bool isOkay = get2101Data();
	if (isOkay)
	{
		int Select1 = 52; // HEX Select Value 1
		int Select2 = 53; // HEX Select Value 2
		int Math1 = 4080; // Calculator Param 1
		int Math2 = 255; // Calculator Param 2
		int BackData = ConvertHextoMathvalue(ClearArray[Select1], ClearArray[Select2]); // Convert to Decimal
		int MathValue = (BackData * Math1) / Math2; // Calculation

		if (debugMode)
		{
			Serial.print("REQUIRED MOTOR RPM: ");
			Serial.println(MathValue);
		}

		return MathValue;
	}
	else
	{
		if (debugMode)
			Serial.println("REQUIRED MOTOR RPM not found");

		return -1;
	}
}




/*
 float ELM327_Simtec_565::getLAMBDA_VCC()

 Description:
 ------------
	* Read the Lambdasonde VCC

 Return:
 -------
  * float
*/
float ELM327_Simtec_565::getLAMBDA_VCC()
{
	bool isOkay = get2101Data();
	if (isOkay)
	{
		int Select1 = 54; // HEX Select Value 1
		int Select2 = 55; // HEX Select Value 2
		float Math1 = 4980; // Calculator Param 1
		int Math2 = 255; // Calculator Param 2
		int BackData = ConvertHextoMathvalue(ClearArray[Select1], ClearArray[Select2]); // Convert to Decimal
		float MathValue = (BackData * Math1) / Math2; // Calculation

		if (debugMode)
		{
			Serial.print("LAMBDA VCC: ");
			Serial.println(MathValue);
		}

		return MathValue;
	}
	else
	{
		if (debugMode)
			Serial.println("LAMBDA VCC not found");

		return -1;
	}
}




/*
 float ELM327_Simtec_565::CYL_IGN_ANGLE()

 Description:
 ------------
	* Read the Cylinder Knock Delay

 Inputs:
 -------
  * int Cylinder - Number of Cylinder

 Return:
 -------
  * float
*/
float ELM327_Simtec_565::getCYL_IGN_ANGLE(int Cylinder)
{
	bool isOkay = get2101Data();
	if (isOkay && Cylinder >= 1 && Cylinder <= CylinderNum)
	{
		int Select1, Select2;

		switch(Cylinder)
		{
			case 1:
			{
				Select1 = 26; // HEX Select Value 1
				Select2 = 27; // HEX Select Value 2
				break;
			}

			case 2:
			{
				Select1 = 28; // HEX Select Value 1
				Select2 = 29; // HEX Select Value 2
				break;
			}

			case 3:
			{
				Select1 = 30; // HEX Select Value 1
				Select2 = 31; // HEX Select Value 2
				break;
			}

			case 4:
			{
				Select1 = 32; // HEX Select Value 1
				Select2 = 33; // HEX Select Value 2
				break;
			}
		}

		float Math1 = 95; // Calculator Param 1
		int Math2 = 255; // Calculator Param 2
		int BackData = ConvertHextoMathvalue(ClearArray[Select1], ClearArray[Select2]); // Convert to Decimal
		float MathValue = (BackData * Math1) / Math2; // Calculation

		if (debugMode)
		{
			Serial.print("CYLINDER: ");
			Serial.print(Cylinder);
			Serial.print(" KNOCK DELAY: ");
			Serial.println(MathValue);
		}

		return MathValue;
	}
	else
	{
		if (debugMode)
			Serial.print("CYLINDER: ");
			Serial.print(Cylinder);
			Serial.println(" KNOCK DELAY not found");

		return -1;
	}
}




/*
 float ELM327_Simtec_565::getTROTTLE_POS()

 Description:
 ------------
	* Read the Throttle Valve Position

 Return:
 -------
  * float
*/
float ELM327_Simtec_565::getTROTTLE_POS()
{
	bool isOkay = get2101Data();
	if (isOkay)
	{
		int Select1 = 72; // HEX Select Value 1
		int Select2 = 73; // HEX Select Value 2
		float Math1 = 99; // Calculator Param 1
		int Math2 = 255; // Calculator Param 2
		int BackData = ConvertHextoMathvalue(ClearArray[Select1], ClearArray[Select2]); // Convert to Decimal
		float MathValue = (BackData * Math1) / Math2; // Calculation

		if (debugMode)
		{
			Serial.print("TROTTLE POS: ");
			Serial.println(MathValue);
		}

		return MathValue;
	}
	else
	{
		if (debugMode)
			Serial.println("TROTTLE POS not found");

		return -1;
	}
}




/*
 int ELM327_Simtec_565::getIDLE_REGULATOR()

 Description:
 ------------
	* Read the Idle Regulator
 Return:
 -------
  * int
*/
int ELM327_Simtec_565::getIDLE_REGULATOR()
{
	bool isOkay = get2101Data();
	if (isOkay)
	{
		int Select1 = 56; // HEX Select Value 1
		int Select2 = 57; // HEX Select Value 2
		int BackData = ConvertHextoMathvalue(ClearArray[Select1], ClearArray[Select2]); // Convert to Decimal

		if (debugMode)
		{
			Serial.print("IDLE REGULATOR: ");
			Serial.println(BackData);
		}

		return BackData;
	}
	else
	{
		if (debugMode)
			Serial.println("IDLE REGULATOR not found");

		return -1;
	}
}




/*
 float ELM327_Simtec_565::getCOOLANT_TEMP_VCC()

 Description:
 ------------
	* Read the Engine Coolant Temp VCC

 Return:
 -------
  * float
*/
float ELM327_Simtec_565::getCOOLANT_TEMP_VCC()
{
	bool isOkay = get2101Data();
	if (isOkay)
	{
		int Select1 = 22; // HEX Select Value 1
		int Select2 = 23; // HEX Select Value 2
		float Math1 = 4.97; // Calculator Param 1
		int Math2 = 255; // Calculator Param 2
		int BackData = ConvertHextoMathvalue(ClearArray[Select1], ClearArray[Select2]); // Convert to Decimal
		float MathValue = (BackData * Math1) / Math2; // Calculation

		if (debugMode)
		{
			Serial.print("COOLANT TEMP VCC: ");
			Serial.println(MathValue);
		}

		return MathValue;
	}
	else
	{
		if (debugMode)
			Serial.println("COOLANT TEMP VCC not found");

		return -1;
	}
}




/*
 float ELM327_Simtec_565::getSUCTION_TEMP_VCC()

 Description:
 ------------
	* Read the Engine Suction Temp VCC

 Return:
 -------
  * float
*/
float ELM327_Simtec_565::getSUCTION_TEMP_VCC()
{
	bool isOkay = get2101Data();
	if (isOkay)
	{
		int Select1 = 20; // HEX Select Value 1
		int Select2 = 21; // HEX Select Value 2
		float Math1 = 4.97; // Calculator Param 1
		int Math2 = 255; // Calculator Param 2
		int BackData = ConvertHextoMathvalue(ClearArray[Select1], ClearArray[Select2]); // Convert to Decimal
		float MathValue = (BackData * Math1) / Math2; // Calculation

		if (debugMode)
		{
			Serial.print("SUCTION TEMP VCC: ");
			Serial.println(MathValue);
		}

		return MathValue;
	}
	else
	{
		if (debugMode)
			Serial.println("SUCTION TEMP VCC not found");

		return -1;
	}
}




/*
 bool ELM327_Simtec_565::get2101Data()

 Description:
 ------------
	* Read from PID 2101 the data and Clear

 Return:
 -------
  * bool - is Data here or not
*/
bool ELM327_Simtec_565::get2101Data()
{
	if (debugMode)
		Serial.println("Getting 2101 Data...");

	// This is for old cars
	sendCommand("2101");

	while (get_response() == ELM_GETTING_MSG);

	if (nb_rx_state == ELM_SUCCESS)
	{
		// Clear Data
		memset(ClearArray, 0, sizeof(ClearArray));

		if (strstr(payload, "80F1112B6101"))
		{
			/*
				Clear Payload
				Priority, Receiver and Transmitter (3 Byte Header)

				
				80F1112B610106034028056062000079FFFF4600000000FF00B946818EE6E27C4B008000000144041800060000810008
				------------06034028056062000079FFFF4600000000FF00B946818EE6E27C4B008000000144041000060000810000
							
			*/
			for(int i = 0; i < strlen(payload) - 11; i++)
				ClearArray[i] = payload[i + 12];

			if (debugMode)
			{
				Serial.print("2101 Data: ");
				Serial.println(ClearArray);
			}

			return true;
		}
		else
		{
			if (debugMode)
			{
				Serial.println("No 2101 Data found");
				printError();
			}

			return false;
		}
	}
	else
	{
		if (debugMode)
		{
			Serial.println("No 2101 Data response");
			printError();
		}
	}
	return false;
}




/*
 bool ELM327_Simtec_565::get_vin()

 Description:
 ------------
	* Read Vehicle Identification Number (VIN). This is a blocking function.

 Return:
 -------
  * bool - the ELM_XXX status of getting the VIN
*/
bool ELM327_Simtec_565::get_vin()
{
	char temp[3] = {0};
	char *idx;
	uint8_t vin_counter = 0;
	uint8_t ascii_val;

	if (debugMode)
		Serial.println("Getting VIN...");

	// This is for old cars
	sendCommand("1A90");

	while (get_response() == ELM_GETTING_MSG);

	if (nb_rx_state == ELM_SUCCESS)
	{
		// Clear Data
		memset(vin, 0, sizeof(vin));

		// **** Decoding ****
		if (strstr(payload, "80F111135A90"))
		{
			idx = strstr(payload, "80F111135A90") + 6;  // Pointer to first ASCII code digit of first VIN digit
			// Loop over each pair of ASCII code digits. 17 VIN digits + 2 skipped line numbers = 19 loops
			for (int i = 6; i < (22 * 2); i += 2) {
				temp[0] = *(idx + i);      // Get first digit of ASCII code
				temp[1] = *(idx + i + 1);  // Get second digit of ASCII code
				// No need to add string termination, temp[3] always == 0
				if (strstr(temp, ":")) continue; // Skip the second "1:" and third "2:" line numbers
				ascii_val = strtol(temp, 0, 16);                // Convert ASCII code to integer
				sprintf(vin + vin_counter++, "%c", ascii_val);  // Convert ASCII code integer back to character
			}
			
			if (debugMode)
			{
				Serial.print("VIN: ");
				Serial.println(vin);
			}

			return true;
		}
		else
		{
			String noVin = "No Vin";
			noVin.toCharArray(vin, noVin.length() + 1);

			if (debugMode)
			{
				Serial.println("No VIN found");
				printError();
			}

			return false;
		}
	}
	else
	{
		String noVin = "No Vin";
		noVin.toCharArray(vin, noVin.length() + 1);

		if (debugMode)
		{
			Serial.println("No VIN response");
			printError();
		}
	}
	return false;
}
