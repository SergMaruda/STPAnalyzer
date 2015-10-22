#include <iostream>

#include <STEPControl_Reader.hxx>
#include "IFSelect_SignatureList.hxx"
#include <TColStd_SequenceOfTransient.hxx>
#include <TColStd_HSequenceOfTransient.hxx>

#include <Interface_InterfaceModel.hxx>
#include <Interface_EntityIterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TColStd_HSequenceOfTransient.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <XSControl_WorkSession.hxx>
#include <Transfer_TransientProcess.hxx>
#include <TopoDS_Iterator.hxx> 
#include <StepData_StepModel.hxx> 

#include <TopExp_Explorer.hxx>

#include <Geom_Surface.hxx>
#include <Geom_Point.hxx>
#include <StepGeom_ConicalSurface.hxx>
#include <StepGeom_Plane.hxx>
#include <StepGeom_CylindricalSurface.hxx>
#include <BRep_Builder.hxx>
#include <StepToGeom_MakeSurface.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <StepGeom_Placement.hxx>
#include <GeomTools_SurfaceSet.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Precision.hxx>

#include <set>
#include <map>
#include <vector>


//-----------------------------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& i_op, const gp_Pnt& i_pt)
{
	i_op<<"(" << i_pt.X() << "," << i_pt.Y() << ","<< i_pt.Z() << ")";
	return i_op;
}

//-----------------------------------------------------------------------------------------------------
gp_Pnt SurfaceCenter(const TopoDS_Shape& i_shape)
{
	GProp_GProps props;
	BRepGProp::SurfaceProperties(i_shape, props);
	return props.CentreOfMass();
}

//-----------------------------------------------------------------------------------------------------
bool IsSurfaceInfinite(Handle(Geom_Surface) s)
{
	Standard_Real u1,u2,v1,v2;
	s->Bounds(u1,u2,v1,v2);

	return (Precision::IsPositiveInfinite(fabs(u1)) ||
			Precision::IsPositiveInfinite(fabs(u2)) ||
			Precision::IsPositiveInfinite(fabs(v1)) ||
			Precision::IsPositiveInfinite(fabs(v1)));
}

//-----------------------------------------------------------------------------------------------------
TopoDS_Shape ToShape(Handle(Geom_Surface) s) 
{
	const Standard_Real TolDegen = 1E-7;
 	Standard_Real u1,u2,v1,v2;
 	s->Bounds(u1,u2,v1,v2);
	
	BRepBuilderAPI_MakeFace mkBuilder(s, u1, u2, v1, v2, TolDegen);
	return mkBuilder.Shape();
}

//------------------------
void DumpSurfaceInfo(Handle(StepGeom_Surface) i_step_geom_surface)
{
	Handle(Geom_Surface) geom_surf;
	StepToGeom_MakeSurface::Convert(i_step_geom_surface, geom_surf);

	auto shape = ToShape(geom_surf);

	auto center = SurfaceCenter(shape );

	GeomTools_SurfaceSet::PrintSurface(geom_surf, std::cout);

	if(IsSurfaceInfinite(geom_surf) == false)
		std::cout<<"Surface center: "<< center << "\n\n\n";
	else
		std::cout<<"Surface is infinite. Can't find center.\n\n\n";
}

//------------------------
IFSelect_ReturnStatus DumpInfo(const Standard_CString& aFileName)
{
	// create additional log file
	STEPControl_Reader aReader;
	IFSelect_ReturnStatus status = aReader.ReadFile(aFileName);
	if (status != IFSelect_RetDone)
		return status;

	aReader.WS()->MapReader()->SetTraceLevel(2); // increase default trace level

	auto model = aReader.StepModel();

	auto num = model->NbEntities();

	std::set<std::string> diff_types;

	std::vector<Handle(StepGeom_Plane)> planes;
	std::vector<Handle(StepGeom_Surface)> surfaces;
	std::vector<Handle(StepGeom_CylindricalSurface)> cylinders;
	std::vector<Handle(StepGeom_Placement)> axes;

	std::map<std::string, size_t> num_types;

	std::cout<<"Analyzing... \n";

	for(Standard_Integer i = 1; i <=num; ++i)
	{
		auto ent = model->Entity(i);
		auto plane = Handle(StepGeom_Plane)::DownCast (ent );

		if(plane)
			planes.push_back(plane);
		else
		{
			auto p_cyl = Handle(StepGeom_CylindricalSurface)::DownCast (ent );
			if(p_cyl)
			{
			cylinders.push_back(p_cyl);
			}
			else
			{
			auto surfs = Handle(StepGeom_Surface)::DownCast (ent );
			if(surfs)
				{
				surfaces.push_back(surfs);
				}
				else
				{
				auto p_axis = Handle(StepGeom_Placement)::DownCast (ent );
				if(p_axis)
					axes.push_back(p_axis );
     			}
			}
		}

	}

	auto sufraces_num = surfaces.size() + cylinders.size() + planes.size();

	std::cout<<"Number of surfaces found (including "<< planes.size() <<" plane(s), "<<cylinders.size()<<" cylindrical surface(s)):" << sufraces_num << "\n";
    std::cout<<"Number of axes found: "<< axes.size()<<"\n\n";

	std::cout<<"\nDumping planes...\n";

	for(size_t i = 0; i < planes.size(); ++i)
	{
		DumpSurfaceInfo(planes[i]);
	}

	std::cout<<"\nDumping cylindrical surfaces...\n";

	for(size_t i = 0; i < cylinders.size(); ++i)
	{
		DumpSurfaceInfo(cylinders[i]);
	}


	std::cout<<"\nDumping other surfaces...\n";

	for(size_t i = 0; i < surfaces.size(); ++i)
	{
		DumpSurfaceInfo(surfaces[i]);
	}

	return status;
}

//---------------------------------------------------------------------------------------------
int main( int arc, char **  argv) 
{
	std::cout<<"STEP Files Analyzer\n";

	if(arc <= 1)
	{
		std::cout<<"Please provide file name for analysis\n";
		return 0;
	}

	const char* file_name = argv[1];

	std::cout<<"File name: "<<file_name<<"\n";


	DumpInfo(file_name);

	return 0;
}
