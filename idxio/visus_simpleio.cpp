/***************************************************
** ViSUS Visualization Project                    **
** Copyright (c) 2010 University of Utah          **
** Scientific Computing and Imaging Institute     **
** 72 S Central Campus Drive, Room 3750           **
** Salt Lake City, UT 84112                       **
**                                                **
** For information about this project see:        **
** http://www.pascucci.org/visus/                 **
**                                                **
**      or contact: pascucci@sci.utah.edu         **
**                                                **
****************************************************/

#include <visus.h>
#include <string>
#include <visuscpp/db/dataset/visus_db_dataset.h>
#include <visuscpp/kernel/geometry/visus_position.h>
#include <visuscpp/kernel/core/visus_path.h>

#include "visus_simpleio.h"

#ifdef VISUS_WIN
#pragma warning (disable:4244)
#endif

using namespace VisusSimpleIO;
using namespace Visus;

//static Application app;

class DatasetImpl{

public:
  DatasetImpl(String filename){
    static Application app;

  dataset = Dataset::loadDataset(filename);
  }

  Dataset* get(){
    return dataset;
  }

  Dataset* dataset;

  ~DatasetImpl(){
    if (dataset != nullptr)
      delete dataset;
  }
};

VisusSimpleIO::SimpleDTypes convertType(DType intype){

  if (intype == DTypes::INT8 || intype.isVectorOf(DTypes::INT8))
    return VisusSimpleIO::INT8;
  else if (intype == DTypes::UINT8 || intype.isVectorOf(DTypes::UINT8))
    return VisusSimpleIO::UINT8;
  else if (intype == DTypes::INT16 || intype.isVectorOf(DTypes::INT16))
    return VisusSimpleIO::INT16;
  else if (intype == DTypes::UINT16 || intype.isVectorOf(DTypes::UINT16))
    return VisusSimpleIO::UINT16;
  else if (intype == DTypes::INT32 || intype.isVectorOf(DTypes::INT32))
    return VisusSimpleIO::INT32;
  else if (intype == DTypes::UINT32 || intype.isVectorOf(DTypes::INT32))
    return VisusSimpleIO::UINT32;
  else if (intype == DTypes::INT64 || intype.isVectorOf(DTypes::INT64))
    return VisusSimpleIO::INT64;
  else if (intype == DTypes::UINT64 || intype.isVectorOf(DTypes::UINT64))
    return VisusSimpleIO::UINT64;
  else if (intype == DTypes::FLOAT32 || intype.isVectorOf(DTypes::FLOAT32))
    return VisusSimpleIO::FLOAT32;
  else if (intype == DTypes::FLOAT64 || intype.isVectorOf(DTypes::FLOAT64))
    return VisusSimpleIO::FLOAT64;

  VisusWarning() << "No type found for conversion";
  VisusAssert(false);
  return UNKNOWN;

}

SimpleIO::~SimpleIO(){
  if (datasetImpl != nullptr)
    delete datasetImpl;
}

bool SimpleIO::openDataset(const String filename){

  String name("file://"); name += Path(filename).toString();

  datasetImpl = new DatasetImpl(name);

  Dataset* dataset = datasetImpl->get();

  dataset_url = name;

  if (!dataset){
    VisusWarning() << "Could not load dataset " << filename << std::endl;
    return false;
  }

  dims = dataset->getDimension();

  ntimesteps = std::max(1, (int)(dataset->getTimesteps()->getMax() - dataset->getTimesteps()->getMin()));

  tsteps = dataset->getTimesteps()->asVector();
  max_resolution = dataset->getMaxResolution();

  const std::vector<Field>& dfields = dataset->getFields();

  for (int i = 0; i < (int)dfields.size(); i++)
  {
    std::string fieldname = dfields[i].name;

    Field field = dataset->getFieldByName(fieldname);
    SimpleField my_field;

    my_field.type = convertType(field.dtype);
    my_field.isVector = field.dtype.isVector();
    my_field.ncomponents = field.dtype.ncomponents();
    my_field.name = fieldname;

    fields.push_back(my_field);
  }

  curr_field = fields[0];

  NdBox lb = dataset->getLogicBox();
  memcpy(logic_to_physic, dataset->getLogicToPhysic().mat, 16 * sizeof(double));

  for (int i = 0; i < 3; i++){
    logic_box.p1[i] = lb.p1()[i];
    logic_box.p2[i] = lb.p2()[i];
  }

  return true;

}

unsigned char* SimpleIO::getData(const SimpleBox box, const int timestate, const char* varname){

  Dataset* dataset = datasetImpl->get();

  if (dataset == nullptr)
  {
    VisusWarning() << "Dataset not loaded " << dataset_url << std::endl;
    return NULL;
  }

  if (!dataset->getTimesteps()->containsTimestep(timestate)){
    return NULL;
  }

  Access* access = dataset->createAccess();

  Field field = dataset->getFieldByName(varname);

  curr_field.type = convertType(field.dtype);
  curr_field.isVector = field.dtype.isVector();
  curr_field.ncomponents = field.dtype.ncomponents();
  curr_field.name = varname;

  NdBox my_box;
  int zp2 = (dims == 2) ? 1 : box.p2.z;

  NdPoint p1(box.p1.x, box.p1.y, box.p1.z);
  NdPoint p2(box.p2.x, box.p2.y, zp2, 1, 1);
  my_box.setP1(p1);
  my_box.setP2(p2);

  //    std::cout << " Box query " << my_box.p1().toString() << " p2 " << my_box.p2().toString() << " variable " << varname << " time " << timestate;

  Query* box_query = new Query(dataset, 'r');

  box_query->setLogicPosition(my_box);
  box_query->setField(dataset->getFieldByName(varname));

  box_query->setTime(timestate);

  box_query->setStartResolution(0);
  box_query->addEndResolution(max_resolution);
  box_query->setMaxResolution(max_resolution);

  // -------- This can be used for lower resolution queries
  //    box_query->addEndResolution(sres);
  //    box_query->addEndResolution(hr);
  //    box_query->setMergeMode(Query::InterpolateSamples);
  // --------

  box_query->setAccess(access);
  box_query->begin();

  VisusReleaseAssert(!box_query->end());
  VisusReleaseAssert(box_query->execute());

  // -------- This can be used for lower resolution queries
  //    box_query->next();
  //    VisusReleaseAssert(!box_query->end());
  // --------

  //    printf("idx query result (dim %dx%dx%d) = %lld:\n", box_query->getBuffer()->getWidth(), box_query->getBuffer()->getHeight(), box_query->getBuffer()->getDepth(), box_query->getBuffer()->c_size());

  delete access;

  SharedPtr<Array> data = box_query->getBuffer();

  //    if( data->c_ptr() != NULL)
  //         std::cout << "size data bytes " << data->c_size();

  return (unsigned char*)data->c_ptr();
}

