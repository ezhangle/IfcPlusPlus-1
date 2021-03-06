/* -*-c++-*- IfcPlusPlus - www.ifcplusplus.com  - Copyright (C) 2011 Fabian Gerold
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

#define _USE_MATH_DEFINES 
#include <math.h>

#include <ifcpp/IFC4/include/IfcProfileDef.h>
#include <ifcpp/IFC4/include/IfcCartesianPoint.h>
#include <ifcpp/IFC4/include/IfcParameterizedProfileDef.h>
#include <ifcpp/IFC4/include/IfcArbitraryOpenProfileDef.h>
#include <ifcpp/IFC4/include/IfcArbitraryClosedProfileDef.h>
#include <ifcpp/IFC4/include/IfcCompositeProfileDef.h>
#include <ifcpp/IFC4/include/IfcDerivedProfileDef.h>
#include <ifcpp/IFC4/include/IfcCurve.h>
#include <ifcpp/IFC4/include/IfcArbitraryProfileDefWithVoids.h>
#include <ifcpp/IFC4/include/IfcBoundedCurve.h>
#include <ifcpp/IFC4/include/IfcCartesianTransformationOperator2D.h>
#include <ifcpp/IFC4/include/IfcRectangleProfileDef.h>
#include <ifcpp/IFC4/include/IfcCircleProfileDef.h>
#include <ifcpp/IFC4/include/IfcCircleHollowProfileDef.h>
#include <ifcpp/IFC4/include/IfcTrapeziumProfileDef.h>
#include <ifcpp/IFC4/include/IfcCenterLineProfileDef.h>
#include <ifcpp/IFC4/include/IfcPositiveLengthMeasure.h>
#include <ifcpp/IFC4/include/IfcRectangleHollowProfileDef.h>
#include <ifcpp/IFC4/include/IfcRoundedRectangleProfileDef.h>
#include <ifcpp/IFC4/include/IfcPositiveLengthMeasure.h>
#include <ifcpp/IFC4/include/IfcLengthMeasure.h>
#include <ifcpp/IFC4/include/IfcNonNegativeLengthMeasure.h>
#include <ifcpp/IFC4/include/IfcEllipseProfileDef.h>
#include <ifcpp/IFC4/include/IfcIShapeProfileDef.h>
#include <ifcpp/IFC4/include/IfcAsymmetricIShapeProfileDef.h>
#include <ifcpp/IFC4/include/IfcLShapeProfileDef.h>
#include <ifcpp/IFC4/include/IfcUShapeProfileDef.h>
#include <ifcpp/IFC4/include/IfcCShapeProfileDef.h>
#include <ifcpp/IFC4/include/IfcZShapeProfileDef.h>
#include <ifcpp/IFC4/include/IfcPlaneAngleMeasure.h>
#include <ifcpp/IFC4/include/IfcTShapeProfileDef.h>
#include <ifcpp/IFC4/include/IfcAxis2Placement2D.h>
#include <ifcpp/IFC4/include/IfcRationalBSplineSurfaceWithKnots.h>

#include <ifcpp/model/IfcPPModel.h>
#include <ifcpp/model/UnitConverter.h>
#include <ifcpp/model/IfcPPException.h>

#include "GeometrySettings.h"
#include "GeomUtils.h"
#include "RepresentationConverter.h"
#include "PlacementConverter.h"
#include "CurveConverter.h"
#include "PointConverter.h"
#include "ProfileConverter.h"


ProfileConverter::ProfileConverter( shared_ptr<CurveConverter>& cc, shared_ptr<SplineConverter>& sc )
	:  m_curve_converter(cc), m_spline_converter(sc)
{
}

ProfileConverter::~ProfileConverter()
{
}

void removeDuplicates( std::vector<std::vector<vector2d_t > >&	paths )
{
	for( auto it_all_loops = paths.begin(); it_all_loops != paths.end(); ++it_all_loops )
	{
		std::vector<vector2d_t >& loop = *it_all_loops;
		if( loop.size() > 1 )
		{
			auto it_loop = loop.begin();
			double previous_x = ( *it_loop ).x;
			double previous_y = ( *it_loop ).y;
			++it_loop;

			while( it_loop != loop.end() )
			{
				vector2d_t& current_point = *it_loop;
				if( std::abs( current_point.x - previous_x ) < 0.00001 )
				{
					if( std::abs( current_point.y - previous_y ) < 0.00001 )
					{
						previous_x = current_point.x;
						previous_y = current_point.y;
						it_loop = loop.erase( it_loop );
						continue;
					}
				}
				previous_x = current_point.x;
				previous_y = current_point.y;
				++it_loop;
			}
		}
	}
}

void ProfileConverter::computeProfile( shared_ptr<IfcProfileDef> profile_def )
{
	// ENTITY IfcProfileDef SUPERTYPE OF(ONEOF(IfcArbitraryClosedProfileDef, IfcArbitraryOpenProfileDef, IfcCompositeProfileDef, IfcDerivedProfileDef, IfcParameterizedProfileDef));
	shared_ptr<IfcArbitraryClosedProfileDef> arbitrary_closed = dynamic_pointer_cast<IfcArbitraryClosedProfileDef>( profile_def );
	if( arbitrary_closed )
	{
		convertIfcArbitraryClosedProfileDef( arbitrary_closed, m_paths );
		removeDuplicates( m_paths );
		return;
	}

	shared_ptr<IfcArbitraryOpenProfileDef> arbitrary_open = dynamic_pointer_cast<IfcArbitraryOpenProfileDef>( profile_def );
	if( arbitrary_open )
	{
		convertIfcArbitraryOpenProfileDef( arbitrary_open, m_paths );
		removeDuplicates( m_paths );
		return;
	}

	shared_ptr<IfcCompositeProfileDef> composite = dynamic_pointer_cast<IfcCompositeProfileDef>( profile_def );
	if( composite )
	{
		convertIfcCompositeProfileDef( composite, m_paths );
		removeDuplicates( m_paths );
		return;
	}

	shared_ptr<IfcDerivedProfileDef> derived = dynamic_pointer_cast<IfcDerivedProfileDef>( profile_def );
	if( derived )
	{
		convertIfcDerivedProfileDef( derived, m_paths );
		removeDuplicates( m_paths );
		return;
	}

	shared_ptr<IfcParameterizedProfileDef> parameterized = dynamic_pointer_cast<IfcParameterizedProfileDef>( profile_def );
	if( parameterized )
	{
		convertIfcParameterizedProfileDefWithPosition( parameterized, m_paths );
		removeDuplicates( m_paths );
		return;
	}

	messageCallback( "Profile not supported", StatusCallback::MESSAGE_TYPE_WARNING, __FUNC__, profile_def.get() );
}

void ProfileConverter::addAvoidingDuplicates( const std::vector<vector2d_t >& polygon, std::vector<std::vector<vector2d_t > >& paths )
{
	if( polygon.size() < 1 )
	{
		return;
	}

	std::vector<vector2d_t > polygon_add;
	polygon_add.push_back( polygon[0] );
	for( int i = 1; i < polygon.size(); ++i )
	{
		const vector2d_t & point = polygon[i];
		const vector2d_t & point_previous = polygon[i - 1];

		// omit duplicate points
		if( std::abs( point.x - point_previous.x ) > 0.00001 )
		{
			polygon_add.push_back( point );
			continue;
		}

		if( std::abs( point.y - point_previous.y ) > 0.00001 )
		{
			polygon_add.push_back( point );
			continue;
		}
	}
	paths.push_back( polygon_add );
}


void ProfileConverter::convertIfcArbitraryClosedProfileDef( const shared_ptr<IfcArbitraryClosedProfileDef>& profile, std::vector<std::vector<vector2d_t > >& paths )
{
	shared_ptr<IfcCurve> outer_curve = profile->m_OuterCurve;
	std::vector<vector2d_t > curve_polygon;
	std::vector<vector2d_t > segment_start_points;

	m_curve_converter->convertIfcCurve2D( outer_curve, curve_polygon, segment_start_points );

	deleteLastPointIfEqualToFirst( curve_polygon );
	addAvoidingDuplicates( curve_polygon, paths );

	// IfcArbitraryProfileDefWithVoids
	shared_ptr<IfcArbitraryProfileDefWithVoids> profile_with_voids = dynamic_pointer_cast<IfcArbitraryProfileDefWithVoids>( profile );
	if( profile_with_voids )
	{
		std::vector<shared_ptr<IfcCurve> > inner_curves = profile_with_voids->m_InnerCurves;
		for( int i = 0; i < inner_curves.size(); ++i )
		{
			shared_ptr<IfcCurve> inner_ifc_curve = inner_curves[i];
			std::vector<vector2d_t > inner_curve_polygon;
			std::vector<vector2d_t > segment_start_points;

			m_curve_converter->convertIfcCurve2D( inner_ifc_curve, inner_curve_polygon, segment_start_points );
			deleteLastPointIfEqualToFirst( inner_curve_polygon );
			addAvoidingDuplicates( inner_curve_polygon, paths );
		}
	}
}

void ProfileConverter::convertIfcArbitraryOpenProfileDef( const shared_ptr<IfcArbitraryOpenProfileDef>& profile, std::vector<std::vector<vector2d_t > >& paths )
{
	// ENTITY IfcArbitraryOpenProfileDef
	//	SUPERTYPE OF(IfcCenterLineProfileDef)
	//	SUBTYPE OF IfcProfileDef;
	//	Curve	 :	IfcBoundedCurve;

	shared_ptr<IfcCurve> ifc_curve = profile->m_Curve;
	
	//shared_ptr<GeometrySettings>& gs = m_curve_converter->m_geom_settings;
	shared_ptr<UnitConverter>& uc = m_curve_converter->m_unit_converter;
	//CurveConverter curve_converter( gs, uc, m_curve_converter->m_point_converter, m_spline_converter, m_curve_converter->m_placement_converter );

	// IfcCenterLineProfileDef
	shared_ptr<IfcCenterLineProfileDef> center_line_profile_def = dynamic_pointer_cast<IfcCenterLineProfileDef>( profile );
	if( center_line_profile_def )
	{
		if( center_line_profile_def->m_Thickness )
		{
			const double thickness = center_line_profile_def->m_Thickness->m_value * uc->getLengthInMeterFactor();
			std::vector<carve::geom::vector<3> > segment_start_points;
			std::vector<carve::geom::vector<3> > basis_curve_points;
			m_curve_converter->convertIfcCurve( ifc_curve, basis_curve_points, segment_start_points );

			int num_base_points = basis_curve_points.size();
			if( num_base_points < 2 )
			{
#ifdef _DEBUG
				std::cout << "IfcCenterLineProfileDef: num curve points < 2";
#endif
				return;
			}

			carve::math::Matrix matrix_sweep;
			carve::geom::vector<3> local_z( carve::geom::VECTOR( 0, 0, 1 ) );
			std::vector<carve::geom::vector<3> > left_points;
			std::vector<carve::geom::vector<3> > right_points;
			carve::geom::vector<3> point_left( carve::geom::VECTOR( 0.0, -thickness*0.5, 0.0 ) );
			carve::geom::vector<3> point_right( carve::geom::VECTOR( 0.0, thickness*0.5, 0.0 ) );

			for( int ii = 0; ii < num_base_points; ++ii )
			{
				carve::geom::vector<3> vertex_current = basis_curve_points[ii];
				carve::geom::vector<3> vertex_next;
				carve::geom::vector<3> vertex_before;
				if( ii == 0 )
				{
					// first point
					vertex_next = basis_curve_points[ii + 1];
					carve::geom::vector<3> delta_element = vertex_next - vertex_current;
					vertex_before = vertex_current - ( delta_element );
				}
				else if( ii == num_base_points - 1 )
				{
					// last point
					vertex_before = basis_curve_points[ii - 1];
					carve::geom::vector<3> delta_element = vertex_current - vertex_before;
					vertex_next = vertex_before + ( delta_element );
				}
				else
				{
					// inner point
					vertex_next = basis_curve_points[ii + 1];
					vertex_before = basis_curve_points[ii - 1];
				}

				carve::geom::vector<3> bisecting_normal;
				GeomUtils::bisectingPlane( vertex_before, vertex_current, vertex_next, bisecting_normal );

				if( ii == num_base_points - 1 )
				{
					bisecting_normal *= -1.0;
				}

				local_z.x = 0;
				local_z.y = 0;
				local_z.z = -1;
				GeomUtils::convertPlane2Matrix( bisecting_normal, vertex_current, local_z, matrix_sweep );

				left_points.push_back( matrix_sweep*point_left );
				right_points.push_back( matrix_sweep*point_right );
			}

			std::reverse( right_points.begin(), right_points.end() );
			std::vector<vector2d_t > polygon;
			for( int i2 = 0; i2 < left_points.size(); ++i2 )
			{
				carve::geom::vector<3>& point3d = left_points[i2];
				polygon.push_back( carve::geom::VECTOR( point3d.x, point3d.y ) );
			}
			for( int i2 = 0; i2 < right_points.size(); ++i2 )
			{
				carve::geom::vector<3>& point3d = right_points[i2];
				polygon.push_back( carve::geom::VECTOR( point3d.x, point3d.y ) );
			}
			addAvoidingDuplicates( polygon, paths );
		}
	}
	else
	{
		std::vector<vector2d_t > polygon;
		std::vector<vector2d_t > segment_start_points;
		m_curve_converter->convertIfcCurve2D( ifc_curve, polygon, segment_start_points );
		addAvoidingDuplicates( polygon, paths );
	}
}

void ProfileConverter::convertIfcCompositeProfileDef( const shared_ptr<IfcCompositeProfileDef>& composite_profile, std::vector<std::vector<vector2d_t > >& paths )
{
	std::vector<int> temploop_counts;
	std::vector<int> tempcontour_counts;

	std::vector<shared_ptr<IfcProfileDef> >& profiles = composite_profile->m_Profiles;
	std::vector<shared_ptr<IfcProfileDef> >::iterator it;

	for( it = profiles.begin(); it != profiles.end(); ++it )
	{
		shared_ptr<IfcProfileDef> profile_def = ( *it );

		shared_ptr<IfcParameterizedProfileDef> parameterized = dynamic_pointer_cast<IfcParameterizedProfileDef>( profile_def );
		if( parameterized )
		{
			convertIfcParameterizedProfileDefWithPosition( parameterized, paths );
			continue;
		}

		shared_ptr<IfcArbitraryOpenProfileDef> open = dynamic_pointer_cast<IfcArbitraryOpenProfileDef>( profile_def );
		if( open )
		{
			convertIfcArbitraryOpenProfileDef( open, paths );
			continue;
		}

		shared_ptr<IfcArbitraryClosedProfileDef> closed = dynamic_pointer_cast<IfcArbitraryClosedProfileDef>( profile_def );
		if( closed )
		{
			convertIfcArbitraryClosedProfileDef( closed, paths );
			continue;
		}

		shared_ptr<IfcCompositeProfileDef> composite = dynamic_pointer_cast<IfcCompositeProfileDef>( profile_def );
		if( composite )
		{
			convertIfcCompositeProfileDef( composite, paths );
			continue;
		}

		shared_ptr<IfcDerivedProfileDef> derived = dynamic_pointer_cast<IfcDerivedProfileDef>( profile_def );
		if( derived )
		{
			convertIfcDerivedProfileDef( derived, paths );
			continue;
		}

		messageCallback( "Profile not supported", StatusCallback::MESSAGE_TYPE_WARNING, __FUNC__, profile_def.get() );
	}
}

void ProfileConverter::convertIfcDerivedProfileDef( const shared_ptr<IfcDerivedProfileDef>& derived_profile, std::vector<std::vector<vector2d_t > >& paths )
{
	const double length_factor = m_curve_converter->m_unit_converter->getLengthInMeterFactor();
	ProfileConverter temp_profiler( m_curve_converter, m_spline_converter );
	temp_profiler.computeProfile( derived_profile->m_ParentProfile );
	const std::vector<std::vector<vector2d_t > >& parent_paths = temp_profiler.getCoordinates();

	shared_ptr<IfcCartesianTransformationOperator2D> transf_op_2D = derived_profile->m_Operator;

	carve::math::Matrix transform( carve::math::Matrix::IDENT() );
	m_curve_converter->m_placement_converter->convertTransformationOperator( transf_op_2D, length_factor, transform );
	for( int i = 0; i < parent_paths.size(); ++i )
	{
		const std::vector<vector2d_t >& loop_parent = parent_paths[i];
		std::vector<vector2d_t > loop;

		for( int j = 0; j < loop_parent.size(); ++j )
		{
			const vector2d_t& pt = loop_parent[j];
			carve::geom::vector<3> pt3d( carve::geom::VECTOR( pt.x, pt.y, 0 ) );
			pt3d = transform*pt3d;
			loop.push_back( carve::geom::VECTOR( pt3d.x, pt3d.y ) );
		}
		paths.push_back( loop );
	}
}

void ProfileConverter::convertIfcParameterizedProfileDefWithPosition( const shared_ptr<IfcParameterizedProfileDef>& parameterized, std::vector<std::vector<vector2d_t > >& paths )
{
	const double length_factor = m_curve_converter->m_unit_converter->getLengthInMeterFactor();
	std::vector<std::vector<vector2d_t > > temp_paths;
	convertIfcParameterizedProfileDef( parameterized, temp_paths );

	// local coordinate system
	if( parameterized->m_Position )
	{
		shared_ptr<IfcAxis2Placement2D> axis2Placement2D = parameterized->m_Position;
		carve::math::Matrix transform( carve::math::Matrix::IDENT() );
		m_curve_converter->m_placement_converter->convertIfcPlacement( axis2Placement2D, length_factor, transform );

		for( int i = 0; i < temp_paths.size(); ++i )
		{
			std::vector<vector2d_t >& path_loop = temp_paths[i];
			for( int j = 0; j < path_loop.size(); ++j )
			{
				vector2d_t& pt = path_loop[j];
				carve::geom::vector<3> pt_3d( carve::geom::VECTOR( pt.x, pt.y, 0 ) );
				pt_3d = transform*pt_3d;
				pt.x = pt_3d.x;
				pt.y = pt_3d.y;
			}
			paths.push_back( path_loop );
		}
	}
	else
	{
		for( int i = 0; i < temp_paths.size(); ++i )
		{
			std::vector<vector2d_t >& path_loop = temp_paths[i];
			paths.push_back( path_loop );
		}
	}
}

void ProfileConverter::convertIfcParameterizedProfileDef( const shared_ptr<IfcParameterizedProfileDef>& profile, std::vector<std::vector<vector2d_t > >& paths )
{
	//IfcParameterizedProfileDef ABSTRACT SUPERTYPE OF (ONEOF
	//	(IfcCShapeProfileDef, IfcCircleProfileDef, IfcEllipseProfileDef, IfcIShapeProfileDef, IfcLShapeProfileDef,
	//	IfcRectangleProfileDef, IfcTShapeProfileDef, IfcTrapeziumProfileDef, IfcUShapeProfileDef, IfcZShapeProfileDef))

	shared_ptr<UnitConverter>& uc = m_curve_converter->m_unit_converter;
	if( !uc )
	{
		return;
	}
	
	shared_ptr<GeometrySettings>& gs = m_curve_converter->m_geom_settings;
	if( !gs )
	{
		return;
	}

	double length_factor = uc->getLengthInMeterFactor();
	double angle_factor = uc->getAngleInRadianFactor();
	std::vector<vector2d_t > outer_loop;

	// Rectangle profile
	shared_ptr<IfcRectangleProfileDef> rectangle_profile = dynamic_pointer_cast<IfcRectangleProfileDef>( profile );
	if( rectangle_profile )
	{
		if( rectangle_profile->m_XDim && rectangle_profile->m_YDim )
		{
			double x_dim = rectangle_profile->m_XDim->m_value*length_factor;
			double y_dim = rectangle_profile->m_YDim->m_value*length_factor;

			shared_ptr<IfcRectangleHollowProfileDef> hollow = dynamic_pointer_cast<IfcRectangleHollowProfileDef>( rectangle_profile );
			if( hollow )
			{
				if( hollow->m_WallThickness )
				{
					const double t = hollow->m_WallThickness->m_value*length_factor;
					double r1 = 0;
					if( hollow->m_OuterFilletRadius && !m_curve_converter->m_geom_settings->m_ignore_profile_radius )
					{
						r1 = hollow->m_InnerFilletRadius->m_value*length_factor;
					}

					double r2 = 0;
					if( hollow->m_InnerFilletRadius && !gs->m_ignore_profile_radius )
					{
						r2 = hollow->m_InnerFilletRadius->m_value*length_factor;
					}

					// Outer
					if( r1 != 0 )
					{
						addArc( outer_loop, r1, 0,			M_PI_2,		x_dim*0.5 - r1,		y_dim*0.5 - r1 );
						addArc( outer_loop, r1, M_PI_2,		M_PI_2,		-x_dim*0.5 + r1,	y_dim*0.5 - r1 );
						addArc( outer_loop, r1, M_PI,		M_PI_2,		-x_dim*0.5 + r1,	-y_dim*0.5 + r1 );
						addArc( outer_loop, r1, 3 * M_PI_2, M_PI_2,		x_dim*0.5 - r1,		-y_dim*0.5 + r1 );
					}
					else
					{
						outer_loop.push_back( carve::geom::VECTOR( -x_dim*0.5, -y_dim*0.5 ) );
						outer_loop.push_back( carve::geom::VECTOR( x_dim*0.5, -y_dim*0.5 ) );
						outer_loop.push_back( carve::geom::VECTOR( x_dim*0.5, y_dim*0.5 ) );
						outer_loop.push_back( carve::geom::VECTOR( -x_dim*0.5, y_dim*0.5 ) );
					}

					// Inner
					std::vector<vector2d_t > inner_loop;
					x_dim -= 2 * t;
					y_dim -= 2 * t;
					if( r2 != 0 )
					{
						addArc( inner_loop, r2, 0,			M_PI_2,		x_dim*0.5 - r2,		y_dim*0.5 - r2 );
						addArc( inner_loop, r2, M_PI_2,		M_PI_2,		-x_dim*0.5 + r2,	y_dim*0.5 - r2 );
						addArc( inner_loop, r2, M_PI,		M_PI_2,		-x_dim*0.5 + r2,	-y_dim*0.5 + r2 );
						addArc( inner_loop, r2, 3 * M_PI_2, M_PI_2,		x_dim*0.5 - r2,		-y_dim*0.5 + r2 );
					}
					else
					{
						inner_loop.push_back( carve::geom::VECTOR( -x_dim*0.5, -y_dim*0.5 ) );
						inner_loop.push_back( carve::geom::VECTOR( x_dim*0.5, -y_dim*0.5 ) );
						inner_loop.push_back( carve::geom::VECTOR( x_dim*0.5, y_dim*0.5 ) );
						inner_loop.push_back( carve::geom::VECTOR( -x_dim*0.5, y_dim*0.5 ) );
					}
					paths.push_back( outer_loop );
					paths.push_back( inner_loop );
				}
				return;
			}

			// RoundedRectangle
			shared_ptr<IfcRoundedRectangleProfileDef> rounded_rectangle = dynamic_pointer_cast<IfcRoundedRectangleProfileDef>( rectangle_profile );
			if( rounded_rectangle && !gs->m_ignore_profile_radius )
			{
				if( rounded_rectangle->m_RoundingRadius )
				{
					double rr = rounded_rectangle->m_RoundingRadius->m_value*length_factor;
					addArc( outer_loop, rr, 0,			M_PI_2, x_dim*0.5 - rr,		y_dim*0.5 - rr );
					addArc( outer_loop, rr, M_PI_2,		M_PI_2, -x_dim*0.5 + rr,	y_dim*0.5 - rr );
					addArc( outer_loop, rr, M_PI,		M_PI_2, -x_dim*0.5 + rr,	-y_dim*0.5 + rr );
					addArc( outer_loop, rr, 3 * M_PI_2,	M_PI_2, x_dim*0.5 - rr,		-y_dim*0.5 + rr );
					paths.push_back( outer_loop );
				}
				return;
			}


			// else it's a standard rectangle
			outer_loop.push_back( carve::geom::VECTOR( -x_dim*0.5, -y_dim*0.5 ) );
			outer_loop.push_back( carve::geom::VECTOR( x_dim*0.5, -y_dim*0.5 ) );
			outer_loop.push_back( carve::geom::VECTOR( x_dim*0.5, y_dim*0.5 ) );
			outer_loop.push_back( carve::geom::VECTOR( -x_dim*0.5, y_dim*0.5 ) );
			paths.push_back( outer_loop );
			return;
		}
	}

	// Trapezium profile
	shared_ptr<IfcTrapeziumProfileDef> trapezium = dynamic_pointer_cast<IfcTrapeziumProfileDef>( profile );
	if( trapezium )
	{
		if( trapezium->m_BottomXDim && trapezium->m_TopXDim && trapezium->m_TopXOffset && trapezium->m_YDim )
		{
			double xBottom = trapezium->m_BottomXDim->m_value*length_factor;
			double xTop = trapezium->m_TopXDim->m_value*length_factor;
			double xOffset = trapezium->m_TopXOffset->m_value*length_factor;
			double y_dim = trapezium->m_YDim->m_value*length_factor;
			outer_loop.push_back( carve::geom::VECTOR( -xBottom*0.5, -y_dim*0.5 ) );
			outer_loop.push_back( carve::geom::VECTOR( xBottom*0.5, -y_dim*0.5 ) );
			outer_loop.push_back( carve::geom::VECTOR( -xBottom*0.5 + xOffset + xTop, y_dim*0.5 ) );
			outer_loop.push_back( carve::geom::VECTOR( -xBottom*0.5 + xOffset, y_dim*0.5 ) );
			paths.push_back( outer_loop );
		}
		return;
	}

	// Circle profile
	shared_ptr<IfcCircleProfileDef> circle_profile_def = dynamic_pointer_cast<IfcCircleProfileDef>( profile );
	if( circle_profile_def )
	{
		shared_ptr<IfcPositiveLengthMeasure> radius_measure = circle_profile_def->m_Radius;
		if( radius_measure )
		{
			double radius = radius_measure->m_value*length_factor;
			if( radius < 0.000001 )
			{
				return;
			}
			int num_segments = gs->m_num_vertices_per_circle; // TODO: adapt to model size and complexity
			double angle = 0;
			for( int i = 0; i < num_segments; ++i )
			{
				outer_loop.push_back( carve::geom::VECTOR( ( radius * cos( angle ) ), ( radius * sin( angle ) ) ) );
				angle += 2.0*M_PI / double( num_segments );
			}
			paths.push_back( outer_loop );

			// CircleHollow
			std::vector<vector2d_t > inner_loop;
			shared_ptr<IfcCircleHollowProfileDef> hollow = dynamic_pointer_cast<IfcCircleHollowProfileDef>( profile );
			if( hollow )
			{
				angle = 0;
				radius -= hollow->m_WallThickness->m_value*length_factor;

				int num_segments2 = gs->m_num_vertices_per_circle; // TODO: adapt to model size and complexity
				for( int i = 0; i < num_segments2; ++i )
				{
					inner_loop.push_back( carve::geom::VECTOR( ( radius * cos( angle ) ), ( radius * sin( angle ) ) ) );
					angle += 2.0*M_PI / double( num_segments2 );
				}
				paths.push_back( inner_loop );
			}
		}
		return;
	}

	// Ellipse profile
	shared_ptr<IfcEllipseProfileDef> ellipse_profile_def = dynamic_pointer_cast<IfcEllipseProfileDef>( profile );
	if( ellipse_profile_def )
	{
		if( ellipse_profile_def->m_SemiAxis1 )
		{
			if( ellipse_profile_def->m_SemiAxis2 )
			{
				double xRadius = ellipse_profile_def->m_SemiAxis1->m_value*length_factor;
				double yRadius = ellipse_profile_def->m_SemiAxis2->m_value*length_factor;
				double radiusMax = std::max( xRadius, yRadius );
				int num_segments = gs->m_num_vertices_per_circle; // TODO: adapt to model size and complexity
				double angle = 0;
				for( int i = 0; i < num_segments; ++i )
				{
					outer_loop.push_back( carve::geom::VECTOR( ( xRadius * cos( angle ) ), ( yRadius * sin( angle ) ) ) );
					angle += 2.0*M_PI / double( num_segments );
				}
				paths.push_back( outer_loop );
			}
		}
		return;
	}

	// I-shaped profile
	shared_ptr<IfcIShapeProfileDef> i_shape = dynamic_pointer_cast<IfcIShapeProfileDef>( profile );
	if( i_shape )
	{
		if( i_shape->m_OverallDepth && i_shape->m_OverallWidth && i_shape->m_WebThickness && i_shape->m_FlangeThickness )
		{
			const double h = i_shape->m_OverallDepth->m_value*length_factor;
			const double b = i_shape->m_OverallWidth->m_value*length_factor;
			const double tw = i_shape->m_WebThickness->m_value*length_factor;
			const double tf = i_shape->m_FlangeThickness->m_value*length_factor;
			double radius = 0;
			if( i_shape->m_FilletRadius && !gs->m_ignore_profile_radius )
			{
				radius = i_shape->m_FilletRadius->m_value*length_factor;
			}

			outer_loop.push_back( carve::geom::VECTOR( b*0.5, -h*0.5 ) );
			outer_loop.push_back( carve::geom::VECTOR( b*0.5, ( -h*0.5 + tf ) ) );

			if( radius != 0 )
			{
				addArc( outer_loop, radius, 3 * M_PI_2, -M_PI_2, tw*0.5 + radius, -h*0.5 + tf + radius );
			}
			else
			{
				outer_loop.push_back( carve::geom::VECTOR( tw*0.5, ( -h*0.5 + tf ) ) );
			}

			shared_ptr<IfcAsymmetricIShapeProfileDef> asym_I_profile = dynamic_pointer_cast<IfcAsymmetricIShapeProfileDef>( i_shape );
			if( asym_I_profile )
			{
				if( asym_I_profile->m_TopFlangeWidth )
				{
					const double bTop = asym_I_profile->m_TopFlangeWidth->m_value*length_factor;
					double tfTop = tf;

					if( asym_I_profile->m_TopFlangeThickness )
					{
						tfTop = asym_I_profile->m_TopFlangeThickness->m_value*length_factor;
					}
					double rTop = radius;
					if( asym_I_profile->m_TopFlangeFilletRadius && !gs->m_ignore_profile_radius )
					{
						rTop = asym_I_profile->m_TopFlangeFilletRadius->m_value*length_factor;
					}

					if( rTop != 0 )
					{
						addArc( outer_loop, rTop, M_PI, -M_PI_2, tw*0.5 + rTop, h*0.5 - tfTop - rTop );
					}
					else
					{
						outer_loop.push_back( carve::geom::VECTOR( tw*0.5, ( h*0.5 - tfTop ) ) );
					}
					outer_loop.push_back( carve::geom::VECTOR( bTop*0.5, ( h*0.5 - tfTop ) ) );
					outer_loop.push_back( carve::geom::VECTOR( bTop*0.5, h*0.5 ) );
				}
			}
			else
			{
				// symmetric: mirror horizontally along x-Axis
				mirrorCopyPathReverse( outer_loop, false, true );
			}

			// mirror vertically along y-axis
			mirrorCopyPathReverse( outer_loop, true, false );
			paths.push_back( outer_loop );
		}
		return;
	}

	// L-shaped profile
	shared_ptr<IfcLShapeProfileDef> l_shape = dynamic_pointer_cast<IfcLShapeProfileDef>( profile );
	if( l_shape )
	{
		if( l_shape->m_Depth && l_shape->m_Thickness )
		{
			const double h = l_shape->m_Depth->m_value*length_factor;
			double b = h;

			if( l_shape->m_Width )
			{
				b = l_shape->m_Width->m_value*length_factor;
			}

			double t = l_shape->m_Thickness->m_value*length_factor;

			double r1 = 0;
			if( l_shape->m_FilletRadius && !gs->m_ignore_profile_radius )
			{
				r1 = l_shape->m_FilletRadius->m_value*length_factor;
			}

			double r2 = 0;
			if( l_shape->m_EdgeRadius && !gs->m_ignore_profile_radius )
			{
				r2 = l_shape->m_EdgeRadius->m_value*length_factor;
			}

			double ls = 0;
			if( l_shape->m_LegSlope )
			{
				ls = l_shape->m_LegSlope->m_value*angle_factor;
			}

			outer_loop.push_back( carve::geom::VECTOR( -b*0.5, -h*0.5 ) );
			outer_loop.push_back( carve::geom::VECTOR( b*0.5, -h*0.5 ) );

			if( r2 != 0 )
			{
				addArc( outer_loop, r2, 0, M_PI_2 - ls, b*0.5 - r2, -h*0.5 + t - r2 );
			}
			else
			{
				outer_loop.push_back( carve::geom::VECTOR( b*0.5, ( -h*0.5 + t ) ) );
			}

			const double s = sin( ls );
			const double c = cos( ls );
			const double z1 = ( -s*( ( c - s )*( r1 + r2 + t ) - c*b + s*h ) ) / ( 2 * c*c - 1 );
			const double z2 = ( -s*( ( c - s )*( r1 + r2 + t ) - c*h + s*b ) ) / ( 2 * c*c - 1 );
			if( r1 != 0 )
			{
				addArc( outer_loop, r1, 3 * M_PI_2 - ls, -M_PI_2 + 2 * ls, -b*0.5 + t + z2 + r1, -h*0.5 + t + z1 + r1 );
			}
			else
			{
				outer_loop.push_back( carve::geom::VECTOR( ( -b*0.5 + t + z2 ), ( -h*0.5 + t + z1 ) ) );
			}

			if( r2 != 0 )
			{
				addArc( outer_loop, r2, ls, M_PI_2 - ls, -b*0.5 + t - r2, h*0.5 - r2 );
			}
			else
			{
				outer_loop.push_back( carve::geom::VECTOR( ( -b*0.5 + t ), h*0.5 ) );
			}

			outer_loop.push_back( carve::geom::VECTOR( -b*0.5, h*0.5 ) );
			paths.push_back( outer_loop );
		}
		return;
	}

	// U-shaped profile
	shared_ptr<IfcUShapeProfileDef> u_shape = dynamic_pointer_cast<IfcUShapeProfileDef>( profile );
	if( u_shape )
	{
		if( u_shape->m_Depth && u_shape->m_FlangeWidth && u_shape->m_WebThickness && u_shape->m_FlangeThickness )
		{
			const double h = u_shape->m_Depth->m_value*length_factor;
			const double b = u_shape->m_FlangeWidth->m_value*length_factor;
			const double tw = u_shape->m_WebThickness->m_value*length_factor;
			const double tf = u_shape->m_FlangeThickness->m_value*length_factor;
			double r1 = 0;
			if( u_shape->m_FilletRadius && !gs->m_ignore_profile_radius )
			{
				r1 = u_shape->m_FilletRadius->m_value*length_factor;
			}
			double r2 = 0;
			if( u_shape->m_EdgeRadius && !gs->m_ignore_profile_radius )
			{
				r2 = u_shape->m_EdgeRadius->m_value*length_factor;
			}
			double fs = 0;
			if( u_shape->m_FlangeSlope )
			{
				fs = u_shape->m_FlangeSlope->m_value*angle_factor;
			}

			outer_loop.push_back( carve::geom::VECTOR( -b*0.5, -h*0.5 ) );
			outer_loop.push_back( carve::geom::VECTOR( b*0.5, -h*0.5 ) );

			double z = tan( fs )*( b*0.5 - r2 );
			if( r2 != 0 )
			{
				addArc( outer_loop, r2, 0, M_PI_2 - fs, b*0.5 - r2, -h*0.5 + tf - z - r2 );
			}
			else
			{
				outer_loop.push_back( carve::geom::VECTOR( b*0.5, ( -h*0.5 + tf - z ) ) );
			}

			z = tan( fs )*( b*0.5 - tw - r1 );
			if( r1 != 0 )
			{
				addArc( outer_loop, r1, 3 * M_PI_2 - fs, -M_PI_2 + fs, -b*0.5 + tw + r1, -h*0.5 + tf + z + r1 );
			}
			else
			{
				outer_loop.push_back( carve::geom::VECTOR( ( -b*0.5 + tw ), ( -h*0.5 + tf + z ) ) );
			}

			// mirror horizontally along x-Axis
			mirrorCopyPathReverse( outer_loop, false, true );
			paths.push_back( outer_loop );
		}
		return;
	}

	// C-shaped profile
	shared_ptr<IfcCShapeProfileDef> c_shape = dynamic_pointer_cast<IfcCShapeProfileDef>( profile );
	if( c_shape )
	{
		if( c_shape->m_Depth && c_shape->m_Width && c_shape->m_Girth && c_shape->m_WallThickness )
		{
			const double h = c_shape->m_Depth->m_value*length_factor;
			const double b = c_shape->m_Width->m_value*length_factor;
			const double g = c_shape->m_Girth->m_value*length_factor;
			const double t = c_shape->m_WallThickness->m_value*length_factor;
			double r1 = 0;
			if( c_shape->m_InternalFilletRadius && !gs->m_ignore_profile_radius )
			{
				r1 = c_shape->m_InternalFilletRadius->m_value*length_factor;
			}

			if( r1 != 0 )
			{
				addArc( outer_loop, r1 + t, M_PI, M_PI_2, -b*0.5 + t + r1, -h*0.5 + t + r1 );
			}
			else
			{
				outer_loop.push_back( carve::geom::VECTOR( -b*0.5, -h*0.5 ) );
			}

			if( r1 != 0 )
			{
				addArc( outer_loop, r1 + t, 3 * M_PI_2, M_PI_2, b*0.5 - t - r1, -h*0.5 + t + r1 );
			}
			else
			{
				outer_loop.push_back( carve::geom::VECTOR( b*0.5, -h*0.5 ) );
			}

			outer_loop.push_back( carve::geom::VECTOR( b*0.5, ( -h*0.5 + g ) ) );
			outer_loop.push_back( carve::geom::VECTOR( ( b*0.5 - t ), ( -h*0.5 + g ) ) );

			if( r1 != 0 )
			{
				addArc( outer_loop, r1, 0, -M_PI_2, b*0.5 - t - r1, -h*0.5 + t + r1 );
			}
			else
			{
				outer_loop.push_back( carve::geom::VECTOR( ( b*0.5 - t ), ( -h*0.5 + t ) ) );
			}

			if( r1 != 0 )
			{
				addArc( outer_loop, r1, 3 * M_PI_2, -M_PI_2, -b*0.5 + t + r1, -h*0.5 + t + r1 );
			}
			else
			{
				outer_loop.push_back( carve::geom::VECTOR( ( -b*0.5 + t ), ( -h*0.5 + t ) ) );
			}
			// mirror horizontally along x-Axis
			mirrorCopyPathReverse( outer_loop, false, true );
			paths.push_back( outer_loop );
		}
		return;
	}

	// Z-shape profile
	shared_ptr<IfcZShapeProfileDef> z_shape = dynamic_pointer_cast<IfcZShapeProfileDef>( profile );
	if( z_shape )
	{
		if( z_shape->m_Depth && z_shape->m_FlangeWidth && z_shape->m_WebThickness && z_shape->m_FlangeThickness )
		{
			const double h = z_shape->m_Depth->m_value*length_factor;
			const double b = z_shape->m_FlangeWidth->m_value*length_factor;
			const double tw = z_shape->m_WebThickness->m_value*length_factor;
			const double tf = z_shape->m_FlangeThickness->m_value*length_factor;
			double r1 = 0;
			if( z_shape->m_FilletRadius && !gs->m_ignore_profile_radius )
			{
				r1 = z_shape->m_FilletRadius->m_value*length_factor;
			}

			double r2 = 0;
			if( z_shape->m_EdgeRadius && !gs->m_ignore_profile_radius )
			{
				r2 = z_shape->m_EdgeRadius->m_value*length_factor;
			}

			outer_loop.push_back( carve::geom::VECTOR( ( -tw*0.5 ), -h*0.5 ) );
			outer_loop.push_back( carve::geom::VECTOR( ( b - tw*0.5 ), -h*0.5 ) );

			if( r2 != 0 )
			{
				addArc( outer_loop, r2, 0, M_PI_2, b - tw*0.5 - r2, -h*0.5 + tf - r2 );
			}
			else
			{
				outer_loop.push_back( carve::geom::VECTOR( ( b - tw*0.5 ), ( -h*0.5 + tf ) ) );
			}

			if( r1 != 0 )
			{
				addArc( outer_loop, r1, 3 * M_PI_2, -M_PI_2, tw*0.5 + r1, -h*0.5 + tf + r1 );
			}
			else
			{
				outer_loop.push_back( carve::geom::VECTOR( ( tw*0.5 ), ( -h*0.5 + tf ) ) );
			}

			// mirror horizontally and vertically
			mirrorCopyPath( outer_loop, true, true );
			paths.push_back( outer_loop );
		}
		return;
	}

	// T-shape profile
	shared_ptr<IfcTShapeProfileDef> t_shape = dynamic_pointer_cast<IfcTShapeProfileDef>( profile );
	if( t_shape )
	{
		const double h = t_shape->m_Depth->m_value*length_factor;
		const double b = t_shape->m_FlangeWidth->m_value*length_factor;
		const double tw = t_shape->m_WebThickness->m_value*length_factor*0.5;
		const double tf = t_shape->m_FlangeThickness->m_value*length_factor;

		double r1 = 0;
		if( t_shape->m_FilletRadius && !gs->m_ignore_profile_radius )
		{
			r1 = t_shape->m_FilletRadius->m_value*length_factor;
		}

		double r2 = 0;
		if( t_shape->m_FlangeEdgeRadius && !gs->m_ignore_profile_radius )
		{
			r2 = t_shape->m_FlangeEdgeRadius->m_value*length_factor;
		}

		double r3 = 0;
		if( t_shape->m_WebEdgeRadius && !gs->m_ignore_profile_radius )
		{
			r3 = t_shape->m_WebEdgeRadius->m_value*length_factor;
		}
		double fs = 0;

		if( t_shape->m_FlangeSlope )
		{
			fs = t_shape->m_FlangeSlope->m_value*angle_factor;
		}

		double ws = 0;
		if( t_shape->m_WebSlope )
		{
			ws = t_shape->m_WebSlope->m_value*angle_factor;
		}

		outer_loop.push_back( carve::geom::VECTOR( -b*0.5, h*0.5 ) );

		const double zf = tan( fs )*( b*0.25 - r2 );
		const double zw = tan( ws )*( h*0.5 - r3 );
		if( r2 != 0 )
		{
			addArc( outer_loop, r2, M_PI, M_PI_2 - fs, -b*0.5 + r2, h*0.5 - tf + zf + r2 );
		}
		else
		{
			outer_loop.push_back( carve::geom::VECTOR( -b*0.5, ( h*0.5 - tf + zf ) ) );
		}

		const double cf = cos( fs );
		const double sf = sin( fs );
		const double cw = cos( ws );
		const double sw = sin( ws );
		const double z1 = ( sf*( ( b - 2 * ( r1 + r2 + tw - zw ) )*cw - 2 * ( h - r3 - r1 - tf + zf )*sw ) ) / ( 2 * ( cf*cw - sf*sw ) );
		const double z2 = tan( ws )*( h - r3 - r1 - z1 - tf + zf );
		if( r1 != 0 )
		{
			addArc( outer_loop, r1, M_PI_2 - fs, -M_PI_2 + fs + ws, -tw + zw - z2 - r1, h*0.5 - tf + zf - z1 - r1 );
		}
		else
		{
			outer_loop.push_back( carve::geom::VECTOR( ( -tw + zw - z2 ), ( h*0.5 - tf + zf - z1 ) ) );
		}

		if( r3 != 0 )
		{
			addArc( outer_loop, r3, M_PI + ws, M_PI_2 - ws, -tw + zw + r3, -h*0.5 + r3 );
		}
		else
		{
			outer_loop.push_back( carve::geom::VECTOR( ( -tw + zw ), -h*0.5 ) );
		}

		// mirror vertically along y-Axis
		mirrorCopyPathReverse( outer_loop, true, false );
		paths.push_back( outer_loop );
		return;
	}

	messageCallback( "Profile not supported", StatusCallback::MESSAGE_TYPE_WARNING, __FUNC__, profile.get() );
}

void ProfileConverter::deleteLastPointIfEqualToFirst( std::vector<vector2d_t >& coords )
{
	while( coords.size() > 2 )
	{
		vector2d_t & first = coords.front();
		vector2d_t & last = coords.back();

		if( std::abs( first.x - last.x ) < 0.00000001 )
		{
			if( std::abs( first.y - last.y ) < 0.00000001 )
			{
				coords.pop_back();
				continue;
			}
		}
		break;
	}
}

void ProfileConverter::simplifyPaths()
{
	simplifyPaths( m_paths );
}

void ProfileConverter::simplifyPaths( std::vector<std::vector<vector2d_t > >& paths )
{
	for( std::vector<std::vector<vector2d_t > >::iterator it_paths = paths.begin(); it_paths != paths.end(); ++it_paths )
	{
		std::vector<vector2d_t >& path = ( *it_paths );
		if( path.size() < 3 )
		{
			continue;
		}
		simplifyPath( path );
	}
}

void ProfileConverter::simplifyPath( std::vector<vector2d_t >& path )
{
	if( path.size() < 3 )
	{
		return;
	}

	for( size_t i = 1; i < path.size(); )
	{
		vector2d_t& previous = path[i - 1];
		vector2d_t& current = path[i];

		vector2d_t segment1 = current - previous;
		if( segment1.length2() < 0.000000001 )
		{
			path.erase( path.begin() + i );
			continue;
		}
		++i;
	}

	for( size_t i = 1; i < path.size() - 1; )
	{
		vector2d_t& previous = path[i - 1];
		vector2d_t& current = path[i];
		vector2d_t& next = path[i + 1];

		vector2d_t segment1 = current - previous;
		segment1.normalize();
		vector2d_t segment2 = next - current;
		segment2.normalize();
		double angle = std::abs( segment1.x*segment2.x + segment1.y*segment2.y );
		if( std::abs( angle - 1 ) < 0.00001 )
		{
			// points are colinear, current point can be removed
			path.erase( path.begin() + i );
			continue;
		}
		++i;
	}

	// 1-----0 5-----4      0-----------3         1---0 4---3      0-----------2
	// |             |  ->  |           |         |   _ ---    ->  |   _ ---
	// 2-------------3      1-----------2         2--              1---

	if( path.size() > 4 )
	{
		vector2d_t& first = path.front();
		vector2d_t& last = path.back();

		if( ( last - first ).length2() < 0.000001 )
		{
			vector2d_t first_segment = path[1] - first;
			first_segment.normalize();
			vector2d_t last_segment = last - path[path.size() - 2];
			last_segment.normalize();
			double angle = std::abs( first_segment.x*last_segment.x + first_segment.y*last_segment.y );
			if( std::abs( angle - 1 ) < 0.00001 )
			{
				// remove first and last point
				path.erase( path.begin() );
				path.pop_back();
			}
		}
	}
}


void ProfileConverter::addArc( std::vector<vector2d_t >& coords, double radius, double start_angle, double opening_angle, double xM, double yM, int num_segments ) const
{
	shared_ptr<GeometrySettings>& gs = m_curve_converter->m_geom_settings;
	if( !gs )
	{
		return;
	}

	if( num_segments < 0 )
	{
		num_segments = (int)( std::abs( opening_angle ) / ( 2.0*M_PI )*gs->m_num_vertices_per_circle ); // TODO: adapt to model size and complexity
	}

	if( num_segments < gs->m_min_num_vertices_per_arc )
	{
		num_segments = gs->m_min_num_vertices_per_arc;
	}

	if( num_segments > 100 )
	{
		num_segments = 100;
	}

	double angle = start_angle;
	double angle_delta = opening_angle / (double)num_segments;
	for( int i = 0; i < num_segments; ++i )
	{
		coords.push_back( carve::geom::VECTOR( ( radius*cos( angle ) + xM ), ( radius*sin( angle ) + yM ) ) );
		angle += angle_delta;
	}
}

void ProfileConverter::addArcWithEndPoint( std::vector<vector2d_t >& coords, double radius, double start_angle, double opening_angle, double xM, double yM ) const
{
	shared_ptr<GeometrySettings>& gs = m_curve_converter->m_geom_settings;
	if( !gs )
	{
		return;
	}
	int num_segments = (int)( std::abs( opening_angle ) / ( 2.0*M_PI )*gs->m_num_vertices_per_circle ); // TODO: adapt to model size and complexity
	if( num_segments < gs->m_min_num_vertices_per_arc )
	{
		num_segments = gs->m_min_num_vertices_per_arc;
	}

	if( num_segments > 100 )
	{
		num_segments = 100;
	}

	double angle = start_angle;
	double angle_delta = opening_angle / (double)( num_segments - 1 );
	for( int i = 0; i < num_segments; ++i )
	{
		coords.push_back( carve::geom::VECTOR( radius*cos( angle ) + xM, radius*sin( angle ) + yM ) );
		angle += angle_delta;
	}
}

void ProfileConverter::addArcWithEndPoint( std::vector<vector2d_t >& coords, double radius, double start_angle, double opening_angle, double xM, double yM, int num_segments )
{
	if( num_segments < 3 )
	{
		num_segments = 3;
	}

	if( num_segments > 100 )
	{
		num_segments = 100;
	}

	double angle = start_angle;
	double angle_delta = opening_angle / (double)( num_segments - 1 );
	for( int i = 0; i < num_segments; ++i )
	{
		coords.push_back( carve::geom::VECTOR( radius*cos( angle ) + xM, radius*sin( angle ) + yM ) );
		angle += angle_delta;
	}
}

void ProfileConverter::mirrorCopyPath( std::vector<vector2d_t >& coords, bool mirror_on_y_axis, bool mirror_on_x_axis )
{
	int points_count = coords.size();
	double x, y;
	for( int i = 0; i < points_count; ++i )
	{
		vector2d_t & p = coords[i];
		if( mirror_on_y_axis )
		{
			x = -p.x;
		}
		else
		{
			x = p.x;
		}
		if( mirror_on_x_axis )
		{
			y = -p.y;
		}
		else
		{
			y = p.y;
		}
		coords.push_back( carve::geom::VECTOR( x, y ) );
	}
}

void ProfileConverter::mirrorCopyPathReverse( std::vector<vector2d_t >& coords, bool mirror_on_y_axis, bool mirror_on_x_axis )
{
	int points_count = coords.size();
	double x, y;
	for( int i = points_count - 1; i >= 0; --i )
	{
		vector2d_t & p = coords[i];
		if( mirror_on_y_axis )
		{
			x = -p.x;
		}
		else
		{
			x = p.x;
		}
		if( mirror_on_x_axis )
		{
			y = -p.y;
		}
		else
		{
			y = p.y;
		}

		coords.push_back( carve::geom::VECTOR( x, y ) );
	}
}
