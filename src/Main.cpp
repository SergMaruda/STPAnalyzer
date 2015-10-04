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

#include <set>
#include <map>
#include <vector>


//------------------------
void DumpSurfaceInfo(Handle(StepGeom_Surface) i_surface)
	{
		Handle(Geom_Surface) res;
		StepToGeom_MakeSurface::Convert(i_surface, res);
		GeomTools_SurfaceSet::PrintSurface(res, std::cout);
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

		auto dn_t = ent->DynamicType();
		diff_types.insert(dn_t->Name());
		++num_types[dn_t->Name()];
	}

	std::cout<<"Number of surfaces: "<< surfaces.size() + cylinders.size() + planes.size() <<"\n";
	std::cout<<"Number of planes: "<< planes.size()<<"\n";
	std::cout<<"Number of cylindrical surface: "<< cylinders.size()<<"\n";
    std::cout<<"Number of axes: "<< axes.size()<<"\n";

	return status;
}

//---------------------------------------------------------------------------------------------
int main( int arc, char **  argv) 
{
	std::cout<<"STEP Files Analyzer\n";

	if(arc <= 1)
		std::cout<<"Please provide file name for analisys\n";

	const char* file_name = argv[1];

	auto res = DumpInfo(file_name);

	return 0;
}
