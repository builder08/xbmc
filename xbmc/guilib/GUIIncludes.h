/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "interfaces/info/InfoBool.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

// forward definitions
namespace tinyxml2
{
class XMLDocument;
class XMLElement;
} // namespace tinyxml2

namespace INFO
{
  class CSkinVariableString;
}

class CGUIIncludes
{
public:
  CGUIIncludes();
  ~CGUIIncludes();

  /*!
   \brief Clear all include components (defaults, constants, variables, expressions and includes)
  */
  void Clear();

  /*!
   \brief Load all include components(defaults, constants, variables, expressions and includes)
   from the main entrypoint \code{file}. Flattens nested expressions and expressions in variable
   conditions after loading all other included files.

   \param file the file to load
  */
  void Load(const std::string &file);

  /*!
   \brief Resolve all include components (defaults, constants, variables, expressions and includes)
   for the given \code{node}. Place the conditions specified for <include> elements in \code{includeConditions}.

   \param node the node from where we start to resolve the include components
   \param includeConditions a map that holds the conditions for resolved includes
   */
  void Resolve(tinyxml2::XMLElement* node, std::map<INFO::InfoPtr, bool>* includeConditions = NULL);

  /*!
   \brief Create a skin variable for the given \code{name} within the given \code{context}.

   \param name the name of the skin variable
   \param context the context where the variable is created in
   \return skin variable
   */
  const INFO::CSkinVariableString* CreateSkinVariable(const std::string& name, int context);

private:
  enum ResolveParamsResult
  {
    NO_PARAMS_FOUND,
    PARAMS_RESOLVED,
    SINGLE_UNDEFINED_PARAM_RESOLVED
  };

  /*!
   \brief Load all include components (defaults, constants, variables, expressions and includes)
   from the given \code{file}.

   \param file the file to load
   \return true if the file was loaded otherwise false
  */
  bool Load_Internal(const std::string &file);

  bool HasLoaded(const std::string &file) const;

  void LoadDefaults(const tinyxml2::XMLElement* node);
  void LoadIncludes(const tinyxml2::XMLElement* node);
  void LoadVariables(const tinyxml2::XMLElement* node);
  void LoadConstants(const tinyxml2::XMLElement* node);
  void LoadExpressions(const tinyxml2::XMLElement* node);

  /*!
   \brief Resolve all expressions containing other expressions to a single evaluatable expression.
  */
  void FlattenExpressions();

  /*!
   \brief Expand any expressions nested in this expression.

   \param expression the expression to flatten
   \param resolved list of already evaluated expression names, to avoid expanding circular references
  */
  void FlattenExpression(std::string &expression, const std::vector<std::string> &resolved);

  /*!
   \brief Resolve all variable conditions containing expressions to a single evaluatable condition.
  */
  void FlattenSkinVariableConditions();

  void SetDefaults(tinyxml2::XMLElement* node);
  void ResolveIncludes(tinyxml2::XMLElement* node,
                       std::map<INFO::InfoPtr, bool>* xmlIncludeConditions = NULL);
  void ResolveConstants(tinyxml2::XMLElement* node);
  void ResolveExpressions(tinyxml2::XMLElement* node);

  typedef std::map<std::string, std::string> Params;
  static void InsertNested(tinyxml2::XMLElement* controls,
                           tinyxml2::XMLElement* include,
                           tinyxml2::XMLElement* node);
  static bool GetParameters(const tinyxml2::XMLElement* include,
                            const char* valueAttribute,
                            Params& params);
  static void ResolveParametersForNode(tinyxml2::XMLElement* node, const Params& params);
  static ResolveParamsResult ResolveParameters(const std::string& strInput, std::string& strOutput, const Params& params);

  std::string ResolveConstant(const std::string &constant) const;
  std::string ResolveExpressions(const std::string &expression) const;

  std::vector<std::string> m_files;
  std::map<std::string, std::pair<std::unique_ptr<tinyxml2::XMLDocument>, Params>> m_includes;
  std::map<std::string, std::unique_ptr<tinyxml2::XMLDocument>> m_defaults;
  std::map<std::string, std::unique_ptr<tinyxml2::XMLDocument>> m_skinvariables;
  std::map<std::string, std::string> m_constants;
  std::map<std::string, std::string> m_expressions;

  std::set<std::string> m_constantAttributes;
  std::set<std::string> m_constantNodes;

  std::set<std::string> m_expressionAttributes;
  std::set<std::string> m_expressionNodes;
};
