/* -*-c++-*- IfcPlusPlus - www.ifcplusplus.com - Copyright (C) 2011 Fabian Gerold
*
* This library is open source and may be redistributed and/or modified under  
* the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
* (at your option) any later version.  The full license is in LICENSE file
* included with this distribution, and on the openscenegraph.org website.
* 
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
* OpenSceneGraph Public License for more details.
*/

#pragma once
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include "ifcpp/model/shared_ptr.h"
#include "ifcpp/model/IfcPPObject.h"

// TYPE IfcBenchmarkEnum = ENUMERATION OF	(GREATERTHAN	,GREATERTHANOREQUALTO	,LESSTHAN	,LESSTHANOREQUALTO	,EQUALTO	,NOTEQUALTO	,INCLUDES	,NOTINCLUDES	,INCLUDEDIN	,NOTINCLUDEDIN);
class IfcBenchmarkEnum : virtual public IfcPPObject
{
public:
	enum IfcBenchmarkEnumEnum
	{
		ENUM_GREATERTHAN,
		ENUM_GREATERTHANOREQUALTO,
		ENUM_LESSTHAN,
		ENUM_LESSTHANOREQUALTO,
		ENUM_EQUALTO,
		ENUM_NOTEQUALTO,
		ENUM_INCLUDES,
		ENUM_NOTINCLUDES,
		ENUM_INCLUDEDIN,
		ENUM_NOTINCLUDEDIN
	};

	IfcBenchmarkEnum();
	IfcBenchmarkEnum( IfcBenchmarkEnumEnum e ) { m_enum = e; }
	~IfcBenchmarkEnum();
	virtual const char* className() const { return "IfcBenchmarkEnum"; }
	virtual shared_ptr<IfcPPObject> getDeepCopy( IfcPPCopyOptions& options );
	virtual void getStepParameter( std::stringstream& stream, bool is_select_type = false ) const;
	static shared_ptr<IfcBenchmarkEnum> createObjectFromSTEP( const std::wstring& arg );
	IfcBenchmarkEnumEnum m_enum;
};

