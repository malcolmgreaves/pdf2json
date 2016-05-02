#include "../include/utils.h"

std::string w2utf8(const wchar_t unicode) {
  std::string out;
  if ((unsigned int)unicode < 0x80) {
    out.push_back((char)unicode);
  }
  else {
    if ((unsigned int)unicode >= 0x80000000) {
      return out;
    }
    int nbytes = 0;
    if ((unsigned int)unicode < 0x800) {
      nbytes = 2;
    }
    else if ((unsigned int)unicode < 0x10000) {
      nbytes = 3;
    }
    else if ((unsigned int)unicode < 0x200000) {
      nbytes = 4;
    }
    else if ((unsigned int)unicode < 0x4000000) {
      nbytes = 5;
    }
    else {
      nbytes = 6;
    }
    static uint8_t prefix[] = { 0xc0, 0xe0, 0xf0, 0xf8, 0xfc };
    int order = 1 << ((nbytes - 1) * 6);
    int code = unicode;
    out.push_back(prefix[nbytes - 2] | (code / order));
    for (int i = 0; i < nbytes - 1; i++) {
      code = code % order;
      order >>= 6;
      out.push_back(0x80 | (code / order));
    }
  }
  return out;
}

std::string w2utf8(const wchar_t* wstr, int len) {
  if (!wstr)
    return "";
  if (len < 0) {
    len = wcslen(wstr);
  }
  std::string out;
  while (len-- > 0) {
    out.append(w2utf8(*wstr++));
  }
  return out;
}

std::wstring utf82w(const char * s) {
  typedef unsigned char byte;
  struct Level {
    byte Head, Data, Null;
    Level(byte h, byte d) {
      Head = h; // the head shifted to the right
      Data = d; // number of data bits
      Null = h << d; // encoded byte with zero data bits
    }
    bool encoded(byte b) { return b >> Data == Head; }
  }; // struct Level
  Level lev[] = {
    Level(2, 6),
    Level(6, 5),
    Level(14, 4),
    Level(30, 3),
    Level(62, 2),
    Level(126, 1)
  };

  wchar_t wc = 0;
  const char * p = s;
  std::wstring result;
  while (*p != 0) {
    byte b = *p++;
    if (b >> 7 == 0) { // deal with ASCII
      wc = b;
      result.push_back(wc);
      continue;
    } // ASCII
    bool found = false;
    for (int i = 1; i < (sizeof(lev) / sizeof(lev[0])); ++i) {
      if (lev[i].encoded(b)) {
        wc = b ^ lev[i].Null; // remove the head
        wc <<= lev[0].Data * i;
        for (int j = i; j > 0; --j) { // trailing bytes
          if (*p == 0) return result; // unexpected
          b = *p++;
          if (!lev[0].encoded(b)) // encoding corrupted
            return result;
          wchar_t tmp = b ^ lev[0].Null;
          wc |= tmp << lev[0].Data*(j - 1);
        } // trailing bytes
        result.push_back(wc);
        found = true;
        break;
      } // lev[i]
    }   // for lev
    if (!found) return result; // encoding incorrect
  }   // while
  return result;
}   // wstring Convert
