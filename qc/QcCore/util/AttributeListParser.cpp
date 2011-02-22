/*
 * This file is part of QuickCPP.
 * (c) Copyright 2011 Jie Wang(twj31470952@gmail.com)
 *
 * QuickCPP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QuickCPP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QuickCPP.  If not, see <http://www.gnu.org/licenses/>.
 */
 
//==============================================================================
//
// $Revision$
// $Date$
//
//==============================================================================

#include "AttributeListParser.h"

#include "QcCore/base/StringUtils.h"
#include "QcCore/base/debug.h"

#include <algorithm>

QC_UTIL_NAMESPACE_BEGIN

const String sNull;

//=============================================================================
// AttributeListParser::parseString
// 
// A pseudo attribute list is normally imbedded within a xml processing
// instruction, or an HTTP response header.
//
// Attlist ::= (S Attribute)*
// Attribute ::= Name ['=' Value]
// Value ::= ['"' | "'"] Char* ['"' | "'"]
//
//=============================================================================
bool AttributeListParser::parseString(const String& in)
{
	static const String ws = QC_T(" \n\t");
	static const String sep = QC_T(" \n\t=");
	static const String delim = QC_T("\"'");

	size_t pos = 0;
	bool bRet = true;

	while(pos != String::npos)
	{
		//
		// skip white space
		//
		pos = in.find_first_not_of(ws, pos);

		if(pos != String::npos)
		{
			//
			// skip over the attribute name to the "="
			//
			size_t sepPos = in.find_first_of(sep, pos);
			if(sepPos != String::npos)
			{
				String attrName = in.substr(pos, sepPos-pos);
				
				// skip white-space up to the "=" (if there is one!)
				pos = in.find_first_not_of(ws, sepPos);
				if(pos == String::npos || in[sepPos] != '=')
				{
					//
					// If there is no separator, treat it like a standalone 
					// attribute with a value equal to the attribute name
					// 
					m_list.push_back(std::make_pair(attrName, attrName));
				}
				else
				{
					//
					// The next char must be a "="
					//
					QC_DBG_ASSERT(in[pos] == '=');
					
					//
					// skip over white-space
					//
					pos = in.find_first_not_of(ws, pos+1);

					if(pos != String::npos)
					{
						const CharType cDelim = in[pos];
						if(cDelim == '"' || cDelim == '\'')
						{
							size_t endDelim = in.find_first_of(cDelim, pos+1);
							if(endDelim != String::npos)
							{
								String attrValue = in.substr(pos+1, endDelim-(pos+1));
								m_list.push_back(std::make_pair(attrName, attrValue));
								pos = (endDelim < in.size())
									? endDelim+1
									: String::npos;
							}
							else
							{
								// unterminated quote
								bRet = false;
								break;
							}
						}
						else
						{
							//
							// An attribute without quotes uses white-space as a delimiter
							//
							size_t endDelim = in.find_first_of(ws, pos);
							size_t attrSize = (endDelim != String::npos) ? (endDelim - pos)
							                                             : String::npos;

							String attrValue = in.substr(pos, attrSize);
							m_list.push_back(std::make_pair(attrName, attrValue));
							pos = (endDelim < in.size())
								? endDelim+1
								: String::npos;
						}
					}
					else
					{
						// "=" followed by only white-space is illegal
						bRet = false;
						break;
					}
				}
			}
			else
			{
				//
				// If there is no separator, treat it like a standalone 
				// attribute with a value equal to the attribute name
				// 
				String attrName = in.substr(pos);
				m_list.push_back(std::make_pair(attrName, attrName));
				break;
			}
		}
	}
	return bRet;
}

//==============================================================================
// AttributeListParser::containsAttribute
//
//==============================================================================
bool AttributeListParser::containsAttribute(const String& name) const
{
	for(AttributeList::const_iterator i = m_list.begin();
	    i != m_list.end(); ++i)
	{
		if(name == ((*i).first))
		{
			return true;
		}
	}
	return false;
}

//==============================================================================
// AttributeListParser::getAttributeValue
//
//==============================================================================
String AttributeListParser::getAttributeValue(const String& name) const
{
	for(AttributeList::const_iterator i = m_list.begin();
	    i != m_list.end(); ++i)
	{
		if(name == ((*i).first))
		{
			return (*i).second;
		}
	}
	return sNull;
}

//==============================================================================
// AttributeListParser::getAttributeValueICase
//
// Case-insensitive version of getAttributeValue(name)
//==============================================================================
String AttributeListParser::getAttributeValueICase(const String& name) const
{
	for(AttributeList::const_iterator i = m_list.begin();
	    i != m_list.end(); ++i)
	{
		if(StringUtils::CompareNoCase(name, (*i).first) == 0)
		{
			return (*i).second;
		}
	}
	return sNull;
}

//==============================================================================
// AttributeListParser::getAttributeName
//
//==============================================================================
String AttributeListParser::getAttributeName(size_t index) const
{
	// missing from GNU libstdc++
	//return m_list.at(index).first;
	if(index < m_list.size())
	{
		return m_list[index].first;
	}
	else
	{
		QC_DBG_ASSERT(false);	
		return sNull;
	}
}

//==============================================================================
// AttributeListParser::getAttributeValue
//
//==============================================================================
String AttributeListParser::getAttributeValue(size_t index) const
{
	// missing from GNU libstdc++
	//return m_list.at(index).second;
	if(index < m_list.size())
	{
		return m_list[index].second;
	}
	else
	{
		QC_DBG_ASSERT(false);	
		return sNull;
	}
}

//==============================================================================
// AttributeListParser::getAttributeCount
//
//==============================================================================
size_t AttributeListParser::getAttributeCount() const
{
	return m_list.size();
}

// Helper functor class for removeAttribute 
struct AttrEquals : std::binary_function<std::pair<String, String>, String, bool>
{
	bool operator()(const std::pair<String, String>& lhs, const String& rhs) const
	{
		return (StringUtils::CompareNoCase(lhs.first, rhs) == 0);
	}
};

//==============================================================================
// AttributeListParser::removeAttribute
//
//==============================================================================
void AttributeListParser::removeAttribute(const String& name)
{
	AttributeList::iterator last = 
		std::remove_if(m_list.begin(),
		               m_list.end(), 
		               std::bind2nd(AttrEquals(), name));

	m_list.erase(last, m_list.end());
}

QC_UTIL_NAMESPACE_END
