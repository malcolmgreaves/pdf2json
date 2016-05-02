////////////////////////////////////////////////////////////////////////////////////////////////////
// pdf_to_json.cpp
// Copyright (c) 2016 Pdfix. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../include/pdf_to_json.h"
#include "../include/utils.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace boost::property_tree;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace pdf_to_json
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace pdf_to_json {
  //structures
  void process_pdf_rgb(PdfRGB& color, ptree& node);
  void process_pdf_text_state(PdfTextState& ts, ptree& node);

  //objects
  void process_pdf_font(PdfFontP font, ptree& node);
  void process_pdf_annot(PdfAnnotP annot, ptree& node);

  //element types
  void process_pde_element(PdeElementP element, ptree& node);
  void process_pde_text(PdeTextP text, ptree& node);
  void process_pde_image(PdeImageP image, ptree& node);
  void process_pde_path(PdePathP path, ptree& node);
  void process_pde_table(PdeTableP table, ptree& node);


  //methods
  void process_pdf_rgb(PdfRGB& color, ptree& node) {
    ptree stroke_node;
    stroke_node.put("red", color.r);
    stroke_node.put("green", color.g);
    stroke_node.put("blue", color.b);
    node.add_child("fill_color", stroke_node);
  }

  void process_pdf_text_state(PdfTextState& ts, ptree& node) {
    node.put("char_spacing", ts.char_spacing);
    node.put("flags", ts.flags);
    process_pdf_font(ts.font, node);
    node.put("font_size", ts.font_size);
    
    node.put("fill_alpha", ts.color_state.fill_opacity);
    process_pdf_rgb(ts.color_state.fill_color, node);
    node.put("stroke_alpha", ts.color_state.stroke_opacity);
    process_pdf_rgb(ts.color_state.stroke_color, node);
    
    node.put("word_spacing", ts.word_spacing);
  }

  void process_pdf_font(PdfFontP font, ptree& node) {
    std::wstring s1, s2, s3;
    s1.resize(font->GetFaceName(nullptr, 0));
    font->GetFaceName((wchar_t*)s1.c_str(), s1.size());
    node.put("face_name", w2utf8(s1.c_str()));
    s2.resize(font->GetFontName(nullptr, 0));
    font->GetFontName((wchar_t*)s2.c_str(), s2.size());
    node.put("font_name", w2utf8(s2.c_str()));
    s3.resize(font->GetSystemFontName(nullptr, 0));
    font->GetSystemFontName((wchar_t*)s3.c_str(), s3.size());
    node.put("system_font_name", w2utf8(s3.c_str()));
    node.put("system_font_bold", font->GetSystemFontBold());
    node.put("system_font_italic", font->GetSystemFontItalic());
    node.put("system_font_charset", font->GetSystemFontCharset());
  }

  void process_pdf_annot(PdfAnnotP annot, ptree& node) {
    PdfAnnotSubtype subtype = annot->GetSubtype();
    switch (subtype) {
    case kAnnotText:
      node.put("subtype", "Text"); break;
    case kAnnotLink:
      node.put("subtype", "Link"); break;
    case kAnnotFreeText:
      node.put("subtype", "FreeText"); break;
    case kAnnotLine:
      node.put("subtype", "Line"); break;
    case kAnnotSquare:
      node.put("subtype", "Square"); break;
    case kAnnotCircle:
      node.put("subtype", "Circle"); break;
    case kAnnotPolygon:
      node.put("subtype", "Polygon"); break;
    case kAnnotPolyLine:
      node.put("subtype", "Polyline"); break;
    case kAnnotHighlight:
      node.put("subtype", "Highlight"); break;
    case kAnnotUnderline:
      node.put("subtype", "Underline"); break;
    case kAnnotSquiggly:
      node.put("subtype", "Squiggly"); break;
    case kAnnotStrikeOut:
      node.put("subtype", "StrikeOut"); break;
    case kAnnotStamp:
      node.put("subtype", "Stamp"); break;
    case kAnnotCaret:
      node.put("subtype", "Caret"); break;
    case kAnnotInk:
      node.put("subtype", "Ink"); break;
    case kAnnotPopup:
      node.put("subtype", "Popup"); break;
    case kAnnotFileAttachment:
      node.put("subtype", "FileAttachment"); break;
    case kAnnotSound:
      node.put("subtype", "Sound"); break;
    case kAnnotMovie:
      node.put("subtype", "Movie"); break;
    case kAnnotWidget:
      node.put("subtype", "Widget"); break;
    case kAnnotScreen:
      node.put("subtype", "Screen"); break;
    case kAnnotPrinterMark:
      node.put("subtype", "PrinterMark"); break;
    case kAnnotTrapNet:
      node.put("subtype", "TrapNet"); break;
    case kAnnotWatermark:
      node.put("subtype", "WaterMark"); break;
    case kAnnot3D:
      node.put("subtype", "3D"); break;
    case kAnnotRedact:
      node.put("subtype", "Redact"); break;
    case kAnnotUnknown:
    default:
      node.put("subtype", "Unknown"); break;
    }

    PdfAnnotAppearance ap;
    annot->GetAppearance(&ap);
    ptree appearance;
    ptree fill_node;
    switch (ap.fill_type) {
    case kFillTypeNone: fill_node.put("fill_type", "none"); break;
    case kFillTypeSolid: fill_node.put("fill_type", "solid"); break;
    }
    process_pdf_rgb(ap.fill_color, fill_node);
    appearance.add_child("fill", fill_node);

    ptree border_node;
    switch (ap.border) {
    case kBorderSolid: border_node.put("border", "solid"); break;
    case kBorderDashed: border_node.put("border", "dashed"); break;
    case kBorderBeveled: border_node.put("border", "beveled"); break;
    case kBorderUnderline: border_node.put("border", "underline"); break;
    case kBorderInset: border_node.put("border", "inset"); break;
    }
    border_node.put("width", ap.border_width);
    process_pdf_rgb(ap.border_color, border_node);
    appearance.add_child("fill", border_node);
    appearance.put("opacity", ap.opacity);
    appearance.put("font_size", ap.font_size);

    node.add_child("appearance", appearance);
  }

  void process_pde_text(PdeTextP text, ptree& node) {
    PdfTextState ts;

    switch (text->GetType())
    {
    case kPdeText:
    {
      node.put("type", "text_paragraph");
      std::wstring s;
      s.resize(text->GetText(nullptr, 0));
      text->GetText((wchar_t*)s.c_str(), s.size());
      node.put("text", w2utf8(s.c_str()));
      text->GetTextState(&ts);
      auto num_lines = text->GetNumTextLines();
      for (auto i = 0; i < num_lines; i++) {
        ptree line_node;
        PdeTextLineP text_line = text->GetTextLine(i);
        process_pde_element((PdeElementP)text_line, line_node);
        node.add_child("element", line_node);
      }
    }
    break;
    case kPdeTextLine:
    {
      PdeTextLineP text_line = (PdeTextLine*)text;
      node.put("type", "text_line");
      std::wstring s;
      s.resize(text_line->GetText(nullptr, 0));
      text_line->GetText((wchar_t*)s.c_str(), s.size());
      node.put("text", w2utf8(s.c_str()));
      text_line->GetTextState(&ts);
      auto num_word = text_line->GetNumWords();
      for (auto i = 0; i < num_word; i++) {
        ptree word_node;
        PdeWordP text_word = text_line->GetWord(i);
        process_pde_element((PdeElementP)text_word, word_node);
        node.add_child("element", word_node);
      }
    }
    break;
    case kPdeWord:
    {
      PdeWordP word = (PdeWord*)text;
      node.put("type", "text_word");
      std::wstring s;
      s.resize(word->GetText(nullptr, 0));
      word->GetText((wchar_t*)s.c_str(), s.size());
      node.put("text", w2utf8(s.c_str()));
      word->GetTextState(&ts);
    }
    break;
    }
    process_pdf_text_state(ts, node);
  }

  void process_pde_image(PdeImageP image, ptree& node) {
    node.put("type", "image");
    //todo???
  }

  void process_pde_path(PdePathP path, ptree& node) {
    node.put("type", "path");
    switch (path->GetType())
    {
    case kPdePath:
      node.put("type", "path");
      break;
    case kPdeRect:
      node.put("type", "rect");
      break;
    case kPdeLine:
      node.put("type", "line");
      break;
    }
    PdfGraphicState gs;
    path->GetGraphicState(&gs);
    node.put("fill_alpha", gs.color_state.fill_opacity);
    process_pdf_rgb(gs.color_state.fill_color, node);
    node.put("stroke_alpha", gs.color_state.stroke_opacity);
    process_pdf_rgb(gs.color_state.stroke_color, node);
  }

  void process_pde_table(PdeTableP table, ptree& node) {
    node.put("type", "table");
    //todo rows, cells
  }

  void process_pde_element(PdeElementP element, ptree& node) {
    if (!element) return;
    node.put("id", element->GetId());

    PdfRect bbox;
    element->GetBBox(&bbox);
    node.put("left", bbox.left);
    node.put("bottom", bbox.bottom);
    node.put("right", bbox.right);
    node.put("top", bbox.top);

    switch (element->GetType())
    {
    case kPdeText:
    case kPdeTextLine:
    case kPdeWord:
      process_pde_text((PdeText*)element, node);
      break;
    case kPdeImage:
      process_pde_image((PdeImage*)element, node);
      break;
    case kPdePath:
    case kPdeLine:
    case kPdeRect:
      process_pde_path((PdePath*)element, node);
      break;
    case kPdeTable:
      process_pde_table((PdeTable*)element, node);
      break;
    default:
      break;
    }
    auto num_childs = element->GetNumChildren();
    for (auto i = 0; i < num_childs; i++) {
      ptree elem_node;
      PdeElementP child = element->GetChild(i);
      process_pde_element(child, elem_node);
      node.add_child("element", elem_node);
    }
  }

  void process_page(PdfPageP page, ptree& node) {
    if (!page) return;
    node.put("number", page->GetNumber());
    node.put("rotate", page->GetRotate());
    PdfPageMapParams params;
    PdePageMapP page_map = page->AcquirePageMap(&params, nullptr, nullptr);
    if (page_map == nullptr)
      return;
    int num_elements = page_map->GetNumElements();
    for (auto i = 0; i < num_elements; i++) {
      ptree elem_tree;
      PdeElementP element = page_map->GetElement(i);
      process_pde_element(element, elem_tree);
      node.add_child("element", elem_tree);
    }
    page->ReleasePageMap();
  }

  void process_file(PdfDocP doc, std::ostringstream& json) {
    ptree doc_node;
    doc_node.put("name", "document name");
    int num_pages = doc->GetNumPages();
    doc_node.put("num_pages", num_pages);
    for (auto i = 0; i < num_pages; i++) {
      ptree page_tree;
      PdfPageP page = doc->AcquirePage(i);
      if (!page)
        continue;
      process_page(page, page_tree);
      doc->ReleasePage(page);
      doc_node.add_child("page", page_tree);
    }
    ptree json_node;
    json_node.add_child("document", doc_node);
    write_json(json, json_node);
  }
}