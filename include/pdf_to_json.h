////////////////////////////////////////////////////////////////////////////////////////////////////
// pdf_to_json.h
// Copyright (c) 2016 Pdfix. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef pdf_to_json_h
#define pdf_to_json_h

#include "../lib/pdfix/Pdfix.h"
#include <sstream>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace pdf_to_json
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace pdf_to_json {
  void process_file(PdfDocP doc, std::ostringstream& json);
}

#endif