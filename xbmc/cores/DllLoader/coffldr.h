#pragma once

/*
 *      Copyright (C) 2005-present Team Kodi
 *      This file is part of Kodi - https://kodi.tv
 *
 *  Kodi is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Kodi is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi. If not, see <https://www.gnu.org/licenses/>.
 *
 */

//#pragma message("including coffldr.h")
#include "coff.h"

#include <stdio.h>

class CoffLoader
{
public:
  CoffLoader();
  virtual ~CoffLoader();

  int ParseCoff(FILE *fp);
  int ParseHeaders(void* hModule);

  void *hModule;   //standard windows HINSTANCE handle hold the whole image
  //Pointers to somewhere in hModule, do not free these pointers
  COFF_FileHeader_t *CoffFileHeader;
  OptionHeader_t *OptionHeader;
  WindowsHeader_t *WindowsHeader;
  Image_Data_Directory_t *Directory;
  SectionHeader_t *SectionHeader;

protected:

  // Allocated structures... hModule now hold the master Memory handle
  SymbolTable_t *SymTable;
  char *StringTable;
  char **SectionData;

  unsigned long EntryAddress; //Initialize entry point

  // Unnecessary data
  //  This is data that is used only during linking and is not necessary
  //  while the program is running in general

  int NumberOfSymbols;
  int SizeOfStringTable;
  int NumOfDirectories;
  int NumOfSections;
  int FileHeaderOffset;

  // Members for printing the structures
  static void PrintFileHeader(COFF_FileHeader_t *FileHeader);
  static void PrintWindowsHeader(WindowsHeader_t *WinHdr);
  static void PrintOptionHeader(OptionHeader_t *OptHdr);
  static void PrintSection(SectionHeader_t *ScnHdr, char *data);
  void PrintStringTable(void);
  void PrintSymbolTable(void);

  // Members for Loading the Different structures
  int LoadCoffHModule(FILE * fp);
  int LoadSymTable(FILE *fp);
  int LoadStringTable(FILE *fp);
  int LoadSections(FILE *fp);

  // Members for access some of the Data

  int RVA2Section(unsigned long RVA);
  void* RVA2Data(unsigned long RVA);
  unsigned long Data2RVA(void* address);

  char *GetStringTblIndex(int index);
  char *GetStringTblOff(int Offset);
  char *GetSymbolName(SymbolTable_t *sym);
  char *GetSymbolName(int index);

  void PerformFixups(void);
};

