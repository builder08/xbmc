#ifndef BUTTON_TRANSLATOR_H
#define BUTTON_TRANSLATOR_H

#pragma once

#ifdef HAS_EVENT_SERVER
#include "utils/EventClient.h"
#endif

class CKey;
struct CAction;
class TiXmlNode;

struct CButtonAction
{
  WORD wID;
  CStdString strID; // needed for "XBMC.ActivateWindow()" type actions
};
// class to map from buttons to actions
class CButtonTranslator
{
#ifdef HAS_EVENT_SERVER
  friend class EVENTCLIENT::CEventButtonState;
#endif
public:
  CButtonTranslator();
  virtual ~CButtonTranslator();

  bool Load();
  void Clear();
  void GetAction(WORD wWindow, const CKey &key, CAction &action);
  WORD TranslateWindowString(const char *szWindow);
  bool TranslateActionString(const char *szAction, WORD &wAction);

private:
  typedef std::multimap<WORD, CButtonAction> buttonMap; // our button map to fill in
  std::map<WORD, buttonMap> translatorMap;       // mapping of windows to button maps
  WORD GetActionCode(WORD wWindow, const CKey &key, CStdString &strAction);
  WORD TranslateGamepadString(const char *szButton);
  WORD TranslateRemoteString(const char *szButton);
  WORD TranslateUniversalRemoteString(const char *szButton);
  WORD TranslateKeyboardString(const char *szButton);
  void MapWindowActions(TiXmlNode *pWindow, WORD wWindowID);
  void MapAction(WORD wButtonCode, const char *szAction, buttonMap &map);
};

extern CButtonTranslator g_buttonTranslator;

#endif
