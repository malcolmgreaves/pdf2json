////////////////////////////////////////////////////////////////////////////////////////////////////
// pdf_export_json.cpp
// Copyright (c) 2016 Pdfix. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <fstream>
#include <stdio.h>
#include <tchar.h>

#include "./lib/pdfix/Pdfix.h"
#include "./include/utils.h"
#include "./include/pdf_to_json.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Invalid number of arguments ");
    return 1;
  }

  std::string input, output;
  if (argc >= 2)
    input = argv[1];
  if (argc >= 3)
    output = argv[2];
  else {
    auto pos = input.find_last_of(".");
    output = std::string(input.begin(), input.begin() + pos) + ".json";
  }

  //collect files
  std::wstring _input = utf82w(input.c_str());
  remove(output.c_str());

  //create valid dll name
  std::string lib_name = "pdfix";
#ifdef _WIN64
  lib_name += "64";
#endif
  lib_name += ".dll";

  // initialize pdfix library
  Pdfix_init(lib_name.c_str());

  Pdfix* pdfix = GetPdfix();

  try {
    if (!pdfix->Authorize(L"e_mail", L"license_key"))
      return 0;
    PdfDocP doc = pdfix->OpenDoc(_input.c_str(), L"");
    if (doc) {
      std::ostringstream json;
      pdf_to_json::process_file(doc, json);
      doc->Close();

      std::ofstream out(output);
      out << json.str();
      out.close();
    }
  }
  catch (...) {
  }

  pdfix->Destroy();

  return 0;
}

