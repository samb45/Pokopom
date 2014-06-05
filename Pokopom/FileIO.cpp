/* Copyright (c) 2012 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include "General.h"
#include "FileIO.h"

#include <fstream>
#include <stdlib.h>

#ifdef _WIN32
extern HINSTANCE hInstance;
#endif

extern char settingsDirectory[1024];
extern u8 multitap;

extern bool bPriority;
extern void SetPriority();

#define PATH_LENGTH 1024
#define LINE_LENGTH 512

namespace FileIO
{
	void GetFilename(char *filename)
	{

		if(settingsDirectory[0])
			sprintf(filename, "%spadPokopom.ini", settingsDirectory);

#ifdef _WIN32
		else
		{
			s32 length = GetModuleFileNameA(hInstance, filename, 1024);

			if(length)
			{
				filename[length -  3] = L'i';
				filename[length -  2] = L'n';
				filename[length -  1] = L'i';
			}
			else
				sprintf(filename, "padPokopom.ini");
		}
#elif defined __linux__
		else
			sprintf(filename, "padPokopom.ini");
#endif

	}

	bool SaveEntry(const char *sec, s32 sectionNumber, const char *key, s32 value, FILE *iniFile)
	{
		char newsec[LINE_LENGTH+1] = {0};
		char line[LINE_LENGTH+1] = {0};

		if(sectionNumber >= 0)
			sprintf(newsec, "[%s%d]", sec, sectionNumber);
		else
			sprintf(newsec, "[%s]", sec);

		static char section[LINE_LENGTH+1] = {0};

		if(strcmp(newsec, section) != 0)
		{
			memcpy(section, newsec, LINE_LENGTH);
			sprintf(line, "%s\n", newsec);
			fputs(line, iniFile);
		}

		sprintf(line, "%s=%d\n", key, value);
		fputs(line, iniFile);

		return true;
	}

	s32 ReadEntry(const char *sec, s32 sectionNumber, const char *key, FILE *iniFile)
	{
		char section[LINE_LENGTH+1] = {0};
		char line[LINE_LENGTH+1] = {0};

		s32 value = -1, length;
		bool sectionFound = false;

		fseek(iniFile, 0, SEEK_SET);

		if(sectionNumber >= 0)
			sprintf(section, "[%s%d]", sec, sectionNumber);
		else
			sprintf(section, "[%s]", sec);

		while(fgets(line, LINE_LENGTH, iniFile) != NULL)
		{
			if(memcmp(line, section, strlen(section)) == 0)
			{
				sectionFound = true;
			}
			else if(sectionFound)
			{
				length = strlen(key);

				if(memcmp(line, key, length) == 0)
				{
					if(line[length] == '=')
					{
						value = atoi(&line[strlen(key)+1]);
						return value;
					}
				}
			}
		}

		return value;
	}

	void INI_SaveSettings()
	{
		char filename[PATH_LENGTH] = {0};
		FILE* iniFile = NULL;
		bool ready = false;

		GetFilename(filename);

		iniFile = fopen(filename, "w");
		ready = iniFile == NULL? false : true;

		if(ready)
		{	
			SaveEntry("General", -1, "ProcPriority", bPriority? 1 : 0, iniFile);
			SaveEntry("General", -1, "KeepAwake", bKeepAwake? 1 : 0, iniFile);
			SaveEntry("General", -1, "INIversion", INIversion, iniFile);
			SaveEntry("General", -1, "Multitap", multitap, iniFile);
			SaveEntry("General", -1, "SwapPorts", SwapPortsEnabled, iniFile); 

			for(s32 port = 0; port < 4; port++)
			{
				s32 AxisInverted =	((settings[port].axisInverted[GP_AXIS_LX]?1:0) << 12) | ((settings[port].axisInverted[GP_AXIS_LY]?1:0) << 8) |
												((settings[port].axisInverted[GP_AXIS_RX]?1:0) << 4) | (settings[port].axisInverted[GP_AXIS_RY]?1:0);

				s32 AxisRemap =	(settings[port].axisRemap[GP_AXIS_LX] << 12) | (settings[port].axisRemap[GP_AXIS_LY] << 8) |
											(settings[port].axisRemap[GP_AXIS_RX] << 4) | (settings[port].axisRemap[GP_AXIS_RY]);

				SaveEntry("Controller", port, "AxisInverted", AxisInverted, iniFile);
				SaveEntry("Controller", port, "AxisRemap", AxisRemap, iniFile);

				SaveEntry("Controller", port, "TriggerDeadzone", settings[port].triggerDeadzone, iniFile);
				SaveEntry("Controller", port, "Pressure", settings[port].pressureRate, iniFile);
				SaveEntry("Controller", port, "Rumble", (s32)(settings[port].rumble * 100.1), iniFile);
				SaveEntry("Controller", port, "SticksLocked", settings[port].sticksLocked ? 1 : 0, iniFile);

				SaveEntry("Controller", port, "XInputPort", settings[port].xinputPort, iniFile);
				SaveEntry("Controller", port, "Disabled", settings[port].disabled ? 1 : 0, iniFile);
				SaveEntry("Controller", port, "DefautMode", settings[port].defaultAnalog ? 1 : 0, iniFile);
				SaveEntry("Controller", port, "GreenAnalog", settings[port].greenAnalog ? 1 : 0, iniFile);
				SaveEntry("Controller", port, "GuitarController", settings[port].isGuitar ? 1 : 0, iniFile);
				
				SaveEntry("Controller", port, "SwapDCBumpers", settings[port].SwapDCBumpers ? 1 : 0, iniFile);
				SaveEntry("Controller", port, "SwapSticks", settings[port].SwapSticksEnabled ? 1 : 0, iniFile);
				SaveEntry("Controller", port, "SwapXO", settings[port].SwapXO ? 1 : 0, iniFile);

				// Left Stick Settings

				SaveEntry("Controller", port, "LS_4wayDAC", settings[port].stickL.b4wayDAC ? 1 : 0, iniFile);
				SaveEntry("Controller", port, "LS_EnableDAC", settings[port].stickL.DACenabled ? 1 : 0, iniFile);
				SaveEntry("Controller", port, "LS_DACthreshold", (s32)(settings[port].stickL.DACthreshold), iniFile);

				SaveEntry("Controller", port, "LS_ExtentionThreshold", (s32)(settings[port].stickL.extThreshold), iniFile);

				SaveEntry("Controller", port, "LS_Linearity", (s32)(settings[port].stickL.linearity * 10.1)+40, iniFile);
				SaveEntry("Controller", port, "LS_AntiDeadzone", (s32)(settings[port].stickL.antiDeadzone * 100.1), iniFile);
				SaveEntry("Controller", port, "LS_Deadzone", (s32)(settings[port].stickL.deadzone * 100.1), iniFile);

				// Right Stick Settings

				SaveEntry("Controller", port, "RS_4wayDAC", settings[port].stickR.b4wayDAC ? 1 : 0, iniFile);
				SaveEntry("Controller", port, "RS_EnableDAC", settings[port].stickR.DACenabled ? 1 : 0, iniFile);
				SaveEntry("Controller", port, "RS_DACthreshold", (s32)(settings[port].stickR.DACthreshold), iniFile);

				SaveEntry("Controller", port, "RS_ExtentionThreshold", (s32)(settings[port].stickR.extThreshold), iniFile);
				
				SaveEntry("Controller", port, "RS_Linearity", (s32)(settings[port].stickR.linearity * 10.1)+40, iniFile);
				SaveEntry("Controller", port, "RS_AntiDeadzone", (s32)(settings[port].stickR.antiDeadzone * 100.1), iniFile);
				SaveEntry("Controller", port, "RS_Deadzone", (s32)(settings[port].stickR.deadzone * 100.1), iniFile);

			}

			fclose(iniFile);
		}
	}

	void INI_LoadSettings()
	{
		settings[0].xinputPort = 0;
		settings[1].xinputPort = 1;
		settings[2].xinputPort = 2;
		settings[3].xinputPort = 3;

		char filename[PATH_LENGTH] = {0};
		FILE* iniFile = NULL;
		bool ready = false;

		GetFilename(filename);

		iniFile = fopen(filename, "r");
		ready = iniFile == NULL? false : true;

		if(ready)
		{
			if(ReadEntry("General", -1, "INIversion", iniFile) != INIversion)
			{
				fclose(iniFile);
				return;
			}

			bPriority = ReadEntry("General", -1, "ProcPriority", iniFile) == 1;
				SetPriority();
			
			SwapPortsEnabled = ReadEntry("General", -1, "SwapPorts", iniFile) == 1;
			bKeepAwake = ReadEntry("General", -1, "KeepAwake", iniFile) == 1;
			multitap = ReadEntry("General", -1, "Multitap", iniFile) & 0xFF;
			multitap = multitap > 2 ? 0 : multitap;

			for(s32 port = 0; port < 4; port++)
			{
				s32 result;

				result = ReadEntry("Controller", port, "AxisInverted", iniFile);
				if(result != -1)
				{
					settings[port].axisInverted[GP_AXIS_RY] = (result & 0xF) ? true : false;
					settings[port].axisInverted[GP_AXIS_RX] = ((result >> 4) & 0xF ) ? true : false;
					settings[port].axisInverted[GP_AXIS_LY] =  ((result >> 8) & 0xF ) ? true : false;
					settings[port].axisInverted[GP_AXIS_LX] = ((result >> 12) & 0xF ) ? true : false;
				}

				result = ReadEntry("Controller", port, "AxisRemap", iniFile);
				if(result != -1)
				{
					settings[port].axisRemap[GP_AXIS_RY] = result & 0xF;
					settings[port].axisRemap[GP_AXIS_RX] = (result >> 4) & 0xF;
					settings[port].axisRemap[GP_AXIS_LY] =  (result >> 8) & 0xF;
					settings[port].axisRemap[GP_AXIS_LX] = (result >> 12) & 0xF;
				}

				result = ReadEntry("Controller", port, "TriggerDeadzone",  iniFile);
				if(result != -1) settings[port].triggerDeadzone = result & 0xFF;

				result = ReadEntry("Controller", port, "Pressure",  iniFile);
				if(result != -1) settings[port].pressureRate = result & 0xFF;

				result = ReadEntry("Controller", port, "Rumble", iniFile);
				if(result != -1) settings[port].rumble = result / 100.0f;

				result = ReadEntry("Controller", port, "XInputPort", iniFile);
				if(result != -1) settings[port].xinputPort = result & 0xF;

				result = ReadEntry("Controller", port, "Disabled", iniFile);
				if(result != -1) settings[port].disabled = result == 1;

				result = ReadEntry("Controller", port, "SticksLocked", iniFile);
				if(result != -1) settings[port].sticksLocked = result == 1;

				result = ReadEntry("Controller", port, "DefautMode", iniFile);
				if(result != -1) settings[port].defaultAnalog = result == 1;

				result = ReadEntry("Controller", port, "GreenAnalog", iniFile);
				if(result != -1) settings[port].greenAnalog = result == 1;

				result = ReadEntry("Controller", port, "GuitarController", iniFile);
				if(result != -1) settings[port].isGuitar = result == 1;

				result = ReadEntry("Controller", port, "SwapDCBumpers", iniFile);
				if(result != -1) settings[port].SwapDCBumpers = result == 1;

				result = ReadEntry("Controller", port, "SwapSticks", iniFile);
				if(result != -1) settings[port].SwapSticksEnabled = result == 1;

				result = ReadEntry("Controller", port, "SwapXO", iniFile);
				if(result != -1) settings[port].SwapXO = result == 1;

				// Left Stick Settings

				result = ReadEntry("Controller", port, "LS_4wayDAC", iniFile);
				if(result != -1) settings[port].stickL.b4wayDAC = result == 1;

				result = ReadEntry("Controller", port, "LS_EnableDAC", iniFile);
				if(result != -1) settings[port].stickL.DACenabled = result == 1;

				result = ReadEntry("Controller", port, "LS_DACthreshold", iniFile);
				if(result != -1) settings[port].stickL.DACthreshold = result;

				result = ReadEntry("Controller", port, "LS_ExtentionThreshold",  iniFile);
				if(result != -1)
				{
					settings[port].stickL.extThreshold = (f64)result;
					settings[port].stickL.extMult = 46339.535798279205464084934426179 / (f64)result;
				}

				result = ReadEntry("Controller", port, "LS_Linearity",  iniFile);
				if(result != -1) settings[port].stickL.linearity = (result-40) / 10.0;

				result = ReadEntry("Controller", port, "LS_AntiDeadzone",  iniFile);
				if(result != -1) settings[port].stickL.antiDeadzone = result / 100.0f;

				result = ReadEntry("Controller", port, "LS_Deadzone",  iniFile);
				if(result != -1) settings[port].stickL.deadzone = result / 100.0f;

				// Right Stick Settings

				result = ReadEntry("Controller", port, "RS_4wayDAC", iniFile);
				if(result != -1) settings[port].stickR.b4wayDAC = result == 1;

				result = ReadEntry("Controller", port, "RS_EnableDAC", iniFile);
				if(result != -1) settings[port].stickR.DACenabled = result == 1;

				result = ReadEntry("Controller", port, "RS_DACthreshold", iniFile);
				if(result != -1) settings[port].stickR.DACthreshold = result;

				result = ReadEntry("Controller", port, "RS_ExtentionThreshold",  iniFile);
				if(result != -1)
				{
					settings[port].stickR.extThreshold = (f64)result;
					settings[port].stickR.extMult = 46339.535798279205464084934426179 / (f64)result;
				}

				result = ReadEntry("Controller", port, "RS_Linearity",  iniFile);
				if(result != -1) settings[port].stickR.linearity = (result-40) / 10.0;

				result = ReadEntry("Controller", port, "RS_AntiDeadzone",  iniFile);
				if(result != -1) settings[port].stickR.antiDeadzone = result / 100.0f;

				result = ReadEntry("Controller", port, "RS_Deadzone",  iniFile);
				if(result != -1) settings[port].stickR.deadzone = result / 100.0f;

			}

			fclose(iniFile);
		}
	}

	bool FASTCALL LoadMempak(u8 *data, u8 port)
	{
#ifdef  _WIN32
		char filename[256];

		sprintf(filename, "MemPaks\\Pokopom%d.mempak", port+1);

		std::fstream file;
		file.open(filename, std::ios::binary | std::ios::in);

		if(!file.is_open()) return false;

		file.seekg(EOF, std::ios_base::end);
		s64 size = file.tellg();

		if(size != 32767) { file.close(); return false; }

		file.seekg(0);
		file.read((char*)data, 0x8000);
		file.close();

		return true;
#else
		return false;
#endif
	}

	void FASTCALL SaveMempak(u8* data, u8 port)
	{
#ifdef _WIN32
		CreateDirectoryA("MemPaks", NULL);

		char filename[1024] = {0};
		sprintf(filename, "MemPaks\\Pokopom%d.mempak", port+1);

		std::fstream file;
		file.open(filename, std::ios::binary | std::ios::out);

		if(!file.is_open()) return;

		file.write((char*)data, 0x8000);
		file.close();
#endif
	}



	bool FASTCALL LoadEEPROM(u8 *data)
	{
#ifdef _WIN32
		FILE *file = NULL;
		file = fopen("Pokopom.eeprom", "r");

		if(file != NULL)
		{
			fread(data, 1, 0x80, file);
			fclose(file);
			return true;
		}
		else
			return false;
#else
		return false;
#endif
	}

	void FASTCALL SaveEEPROM(u8 *data)
	{
#ifdef _WIN32
		FILE *file = NULL;
		file = fopen("Pokopom.eeprom", "w");

		if(file != NULL)
		{
			fwrite(data, 1, 0x80, file);
			fclose(file);
		}
#endif
	}

} // End namespace FileIO

