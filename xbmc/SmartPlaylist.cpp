#include "SmartPlaylist.h"
#include "utils/log.h"
#include "StringUtils.h"

typedef struct
{
  char string[13];
  CSmartPlaylistRule::SEARCH_FIELD field;
} translateField;

static const translateField fields[] = { "genre", CSmartPlaylistRule::SONG_GENRE,
                                         "album", CSmartPlaylistRule::SONG_ALBUM,
                                         "artist", CSmartPlaylistRule::SONG_ARTIST,
                                         "title", CSmartPlaylistRule::SONG_TITLE,
                                         "year", CSmartPlaylistRule::SONG_YEAR,
                                         "time", CSmartPlaylistRule::SONG_TIME,
                                         "tracknumber", CSmartPlaylistRule::SONG_TRACKNUMBER,
                                         "filename", CSmartPlaylistRule::SONG_FILENAME,
                                         "playcount", CSmartPlaylistRule::SONG_PLAYCOUNT };

#define NUM_FIELDS sizeof(fields) / sizeof(translateField)

CSmartPlaylistRule::CSmartPlaylistRule()
{
  m_field = SONG_ARTIST;
  m_operator = OPERATOR_CONTAINS;
  m_parameter = "";
}

void CSmartPlaylistRule::TranslateStrings(const char *field, const char *oper, const char *parameter)
{
  m_field = TranslateField(field);
  m_operator = TranslateOperator(oper);
  m_parameter = parameter;
  if (m_field == SONG_TIME)
  { // translate time to seconds
    CStdStringArray secs;
    StringUtils::SplitString(m_parameter, ":", secs);
    int timeInSecs = 0;
    for (unsigned int i = 0; i < secs.size(); i++)
    {
      timeInSecs *= 60;
      timeInSecs += atoi(secs[i]);
    }
    m_parameter.Format("%i", timeInSecs);
  }
}

TiXmlElement CSmartPlaylistRule::GetAsElement()
{
  TiXmlElement rule("rule");
  TiXmlText parameter(m_parameter.c_str());
  rule.InsertEndChild(parameter);
  rule.SetAttribute("field", TranslateField(m_field).c_str());
  rule.SetAttribute("operator", TranslateOperator(m_operator).c_str());
  return rule;
}

CSmartPlaylistRule::SEARCH_FIELD CSmartPlaylistRule::TranslateField(const char *field)
{
  for (int i = 0; i < NUM_FIELDS; i++)
    if (strcmpi(field, fields[i].string) == 0) return fields[i].field;
  return SONG_ALBUM;
}

CStdString CSmartPlaylistRule::TranslateField(SEARCH_FIELD field)
{
  for (int i = 0; i < NUM_FIELDS; i++)
    if (field == fields[i].field) return fields[i].string;
  return "album";
}

CSmartPlaylistRule::SEARCH_OPERATOR CSmartPlaylistRule::TranslateOperator(const char *oper)
{
  if (strcmpi(oper, "contains") == 0) return OPERATOR_CONTAINS;
  else if (strcmpi(oper, "doesnotcontain") == 0) return OPERATOR_DOES_NOT_CONTAIN;
  else if (strcmpi(oper, "is") == 0) return OPERATOR_EQUALS;
  else if (strcmpi(oper, "isnot") == 0) return OPERATOR_DOES_NOT_EQUAL;
  else if (strcmpi(oper, "startswith") == 0) return OPERATOR_STARTS_WITH;
  else if (strcmpi(oper, "endswith") == 0) return OPERATOR_ENDS_WITH;
  else if (strcmpi(oper, "greaterthan") == 0) return OPERATOR_GREATER_THAN;
  else if (strcmpi(oper, "lessthan") == 0) return OPERATOR_LESS_THAN;
  return OPERATOR_CONTAINS;
}

CStdString CSmartPlaylistRule::TranslateOperator(SEARCH_OPERATOR oper)
{
  if (oper == OPERATOR_CONTAINS) return "contains";
  else if (oper == OPERATOR_DOES_NOT_CONTAIN) return "contains";
  else if (oper == OPERATOR_EQUALS) return "is";
  else if (oper == OPERATOR_DOES_NOT_EQUAL) return "isnot";
  else if (oper == OPERATOR_STARTS_WITH) return "startswith";
  else if (oper == OPERATOR_ENDS_WITH) return "endswith";
  else if (oper == OPERATOR_GREATER_THAN) return "greaterthan";
  else if (oper == OPERATOR_LESS_THAN) return "lessthan";
  return "contains";
}

CStdString CSmartPlaylistRule::GetWhereClause()
{
  // the comparison piece
  CStdString operatorString;
  switch (m_operator)
  {
  case OPERATOR_CONTAINS:
    operatorString = " LIKE '%%%s%%'"; break;
  case OPERATOR_DOES_NOT_CONTAIN:
    operatorString = " NOT LIKE '%%%s%%'"; break;
  case OPERATOR_EQUALS:
    operatorString = " LIKE '%s'"; break;
  case OPERATOR_DOES_NOT_EQUAL:
    operatorString = " NOT LIKE '%s'"; break;
  case OPERATOR_STARTS_WITH:
    operatorString = " LIKE '%s%%'"; break;
  case OPERATOR_ENDS_WITH:
    operatorString = " LIKE '%%%s'"; break;
  case OPERATOR_GREATER_THAN:
    operatorString = " > '%s'"; break;
  case OPERATOR_LESS_THAN:
    operatorString = " < '%s'"; break;
  }
  // TODO: This needs to be FormatSQL
  CStdString parameter;
  parameter.Format(operatorString.c_str(), m_parameter.c_str());
  // now the query parameter
  CStdString query;
  if (m_field == SONG_TITLE)
    query = "strTitle" + parameter;
  else if (m_field == SONG_GENRE)
    query = "(strGenre" + parameter + ") or (idsong IN (select idsong from genre,exgenresong where exgenresong.idgenre = genre.idgenre and genre.strGenre" + parameter + "))";
  else if (m_field == SONG_ALBUM)
    query = "strAlbum" + parameter;
  else if (m_field == SONG_YEAR)
    query = "iYear" + parameter;
  else if (m_field == SONG_ARTIST)
    query = "(strArtist" + parameter + ") or (idsong IN (select idsong from artist,exartistsong where exartistsong.idartist = artist.idartist and artist.strArtist" + parameter + "))";
  else if (m_field == SONG_TIME)
    query = "iDuration" + parameter;
  else if (m_field == SONG_FILENAME)
    query = "strFilename" + parameter;
  else if (m_field == SONG_PLAYCOUNT)
    query = "iTimesPlayed" + parameter;
  else if (m_field == SONG_TRACKNUMBER)
    query = "iTrack" + parameter;

  return query;
}

CSmartPlaylist::CSmartPlaylist()
{
  m_matchAllRules = true;
}

bool CSmartPlaylist::Load(const CStdString &path)
{
  TiXmlDocument doc;
  if (!doc.LoadFile(path))
  {
    CLog::Log(LOGERROR, "Error loading Smart playlist %s", path.c_str());
    return false;
  }

  TiXmlElement *root = doc.RootElement();
  if (!root || strcmpi(root->Value(),"smartplaylist") != 0)
  {
    CLog::Log(LOGERROR, "Error loading Smart playlist %s", path.c_str());
    return false;
  }
  // load the playlist name
  TiXmlHandle name = ((TiXmlHandle)root->FirstChild("name")).FirstChild();
  if (name.Node())
    m_playlistName = name.Node()->Value();
  TiXmlHandle match = ((TiXmlHandle)root->FirstChild("match")).FirstChild();
  if (match.Node())
    m_matchAllRules = strcmpi(match.Node()->Value(), "all") == 0;
  // now the rules
  TiXmlElement *rule = root->FirstChildElement("rule");
  while (rule)
  {
    // format is:
    // <rule field="Genre" operator="contains">parameter</rule>
    const char *field = rule->Attribute("field");
    const char *oper = rule->Attribute("operator");
    TiXmlNode *parameter = rule->FirstChild();
    if (field && oper && parameter)
    { // valid rule
      CSmartPlaylistRule rule;
      rule.TranslateStrings(field, oper, parameter->Value());
      m_playlistRules.push_back(rule);
    }
    rule = rule->NextSiblingElement("rule");
  }
  return true;
}

bool CSmartPlaylist::Save(const CStdString &path)
{
  TiXmlDocument doc;
  // TODO: Format strings in UTF8?
  TiXmlElement xmlRootElement("smartplaylist");
  TiXmlNode *pRoot = doc.InsertEndChild(xmlRootElement);
  if (!pRoot) return false;
  // add the <name> tag
  TiXmlText name(m_playlistName.c_str());
  TiXmlElement nodeName("name");
  nodeName.InsertEndChild(name);
  pRoot->InsertEndChild(nodeName);
  // add the <match> tag
  TiXmlText match(m_matchAllRules ? "all" : "one");
  TiXmlElement nodeMatch("match");
  nodeMatch.InsertEndChild(match);
  pRoot->InsertEndChild(nodeMatch);
  // add <rule> tags
  for (vector<CSmartPlaylistRule>::iterator it = m_playlistRules.begin(); it != m_playlistRules.end(); ++it)
  {
    pRoot->InsertEndChild((*it).GetAsElement());
  }
  return doc.SaveFile(path);
}

void CSmartPlaylist::SetName(const CStdString &name)
{
  m_playlistName = name;
}

void CSmartPlaylist::AddRule(const CSmartPlaylistRule &rule)
{
  m_playlistRules.push_back(rule);
}

CStdString CSmartPlaylist::GetWhereClause()
{
  CStdString rule;
  for (vector<CSmartPlaylistRule>::iterator it = m_playlistRules.begin(); it != m_playlistRules.end(); ++it)
  {
    if (it != m_playlistRules.begin())
      rule += m_matchAllRules ? " AND " : " OR ";
    else
      rule += "WHERE ";
    rule += "(";
    rule += (*it).GetWhereClause();
    rule += ")";
  }
  return rule;
}