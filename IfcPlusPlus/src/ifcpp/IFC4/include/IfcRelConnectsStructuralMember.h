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
#include "IfcRelConnects.h"
class IfcStructuralMember;
class IfcStructuralConnection;
class IfcBoundaryCondition;
class IfcStructuralConnectionCondition;
class IfcLengthMeasure;
class IfcAxis2Placement3D;
//ENTITY
class IfcRelConnectsStructuralMember : public IfcRelConnects
{
public:
	IfcRelConnectsStructuralMember();
	IfcRelConnectsStructuralMember( int id );
	~IfcRelConnectsStructuralMember();
	virtual shared_ptr<IfcPPObject> getDeepCopy( IfcPPCopyOptions& options );
	virtual void getStepLine( std::stringstream& stream ) const;
	virtual void getStepParameter( std::stringstream& stream, bool is_select_type = false ) const;
	virtual void readStepArguments( const std::vector<std::wstring>& args, const std::map<int,shared_ptr<IfcPPEntity> >& map );
	virtual void setInverseCounterparts( shared_ptr<IfcPPEntity> ptr_self );
	virtual void getAttributes( std::vector<std::pair<std::string, shared_ptr<IfcPPObject> > >& vec_attributes );
	virtual void getAttributesInverse( std::vector<std::pair<std::string, shared_ptr<IfcPPObject> > >& vec_attributes );
	virtual void unlinkFromInverseCounterparts();
	virtual const char* className() const { return "IfcRelConnectsStructuralMember"; }


	// IfcRoot -----------------------------------------------------------
	// attributes:
	//  shared_ptr<IfcGloballyUniqueId>					m_GlobalId;
	//  shared_ptr<IfcOwnerHistory>						m_OwnerHistory;				//optional
	//  shared_ptr<IfcLabel>							m_Name;						//optional
	//  shared_ptr<IfcText>								m_Description;				//optional

	// IfcRelationship -----------------------------------------------------------

	// IfcRelConnects -----------------------------------------------------------

	// IfcRelConnectsStructuralMember -----------------------------------------------------------
	// attributes:
	shared_ptr<IfcStructuralMember>					m_RelatingStructuralMember;
	shared_ptr<IfcStructuralConnection>				m_RelatedStructuralConnection;
	shared_ptr<IfcBoundaryCondition>				m_AppliedCondition;			//optional
	shared_ptr<IfcStructuralConnectionCondition>	m_AdditionalConditions;		//optional
	shared_ptr<IfcLengthMeasure>					m_SupportedLength;			//optional
	shared_ptr<IfcAxis2Placement3D>					m_ConditionCoordinateSystem;	//optional
};

