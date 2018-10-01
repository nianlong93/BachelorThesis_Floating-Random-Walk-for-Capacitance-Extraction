/*
CREATED : Jan 31, 2013
AUTHOR  : Yu-Chung Hsiao
EMAIL   : project.caplet@gmail.com

This file is part of CAPLET.

CAPLET is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CAPLET is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the Lesser GNU General Public License
along with CAPLET.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef GEOLOADER_H
#define GEOLOADER_H

#include "gdsgeometry.h"
#include "debug.h"

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <list>
#include <cmath>
#include <exception>
#include <string>
#include <stdexcept>


//****
//*
//* LIMITATION:
//*
//**
//*
//* 1. Contact layer ca needs special care: it connects both poly and drain.
//*    Currently we consider the connection to poly only. (Ignore for now)
//* 2. Need special treatments for drain/contact and poly/contact. (Ignore for now)
//*


//@author:Dragon
//the class GDS own a static member variable zone. It represents the region of the FRW solution.
class GDS{
public:
    static Rectangle zone;
};
//@author:Dragon
//Do not forget initiating !
Rectangle GDS::zone=Rectangle();

typedef std::vector<std::vector<float> > Matrix;

class ExtractionInfo{
public:
    int nConductor;
    int nBasisFunction;
    double tTotal;
    double tSetup;
    double tSolving;
    double tBasis;
    float error;

    Matrix capacitanceMatrix;

    void printBasic(std::ostream &out = std::cout) const;
    void printBasicHeader(std::ostream &out = std::cout) const;
    void printBasicLine(std::ostream &out = std::cout) const;
    void printMatrix(std::ostream &out = std::cout) const;
    void print(std::ostream &out = std::cout) const;
    std::string toString() const;
    float compare(const ExtractionInfo& ref) const throw (std::length_error);
    std::vector<float> compareDiagonal(const ExtractionInfo& ref) const throw (std::length_error);
};

typedef std::list<ExtractionInfo> ExtractionInfoList;

class FileNotFoundError : public std::runtime_error{
public:
    explicit FileNotFoundError(const std::string &fileName)
        : runtime_error(fileName), m_msg(fileName){}
    virtual ~FileNotFoundError() throw() {}
    virtual const char* what() const throw(){
        return (std::string(m_msg)+" is not found.").c_str();
    }
private:
    std::string m_msg;
};

class GeoLoader{
public:
    GeoLoader();
    ~GeoLoader();
    void clear();

    enum SolverType { CAPLET, FASTCAP, STANDARD };

    void loadGeo( const std::string &fileName ) throw (FileNotFoundError, GeometryNotManhattanError);

    //**
    //* generate basis functions and floating point geometry
    //* - unit starts to get in
    const ConductorFPList &getGeometryConductorList(const float unit);
    const ConductorFPList &getPWCBasisFunction(const float unit, const float suggestedPanelSize);
    const ConductorFPList &getPWCBasisFunction() const;
    const ConductorFPList &getInstantiableBasisFunction(const float unit, const float archLength,
                                                        const float projectionDistance=caplet::DEFAULT_PROJECTION_DISTANCE,
                                                        const float projectionMergeDistance=caplet::DEFAULT_PROJECTION_MERGE_DISTANCE);

    void loadQui(const std::string &inputFileName) throw (FileNotFoundError);

    ExtractionInfo &runFastcap(const std::string &pathFileBaseName, const std::string &option="")
            throw (FileNotFoundError);
    ExtractionInfo &runCaplet(const std::string &pathFileBaseName, const unsigned coreNum=1 )
            throw (FileNotFoundError);
    ExtractionInfo &runCapletQui(const std::string &pathFileBaseName )
            throw (FileNotFoundError);

    std::string fileName;

    //**
    //* layer information
    int     nMetal;
    int**   metalDef;   //* size: nMetal x 2

    int     nVia;
    int**   viaDef;     //* size: nVia x 2
    int**   viaConnect; //* size: nVia x 2

    //______________________________________________________
    //*
    //* Reference result
    //*
    const ExtractionInfo& getLastResult() const;
    const ExtractionInfo& getReferenceResult() const;
    void loadReferenceResult(const std::string &filename);
    const ExtractionInfo& storeLastAsReference(const std::string &pathFileNameCmat);
    ExtractionInfoList compareAllAgainstReference() const;
    void clearResult();

    //______________________________________________________
    //* Getters and setters
    size_t getNumberOfConductor() const;

// private:
    bool                    isLoaded;
    //______________________________________________________
    //* Paramters

    ConductorList           metalConductorList;
    LayeredRectangleList    viaLayeredRectangleList;

    ConductorList           geometryConductorList;

    ConductorFPList         geometryConductorFPList;
    ConductorFPList         pwcConductorFPList;
    ConductorFPList         instantiableConductorFPList;

    double                  tPWCConstruction;
    double                  tInstantiableConstruction;

    void readGeo(
            const std::string   &geoFileName,
            LayeredPolygonList  &metalLayeredPolygonList,
            LayeredPolygonList  &viaLayeredPolygonList)
            throw (FileNotFoundError);
    void readLayerInfo(std::ifstream &fin);
    void readStruc(std::ifstream &fin, int nLayer, std::vector<PolygonList> &struc,  int metalOrVia);
    void printStruc(int nLayer, std::vector<PolygonList> &struc);

    ConductorList &generateConductorList(ConductorList &conductorList, bool flagDecomposed);

    ExtractionInfoList extractionInfoList;
    ExtractionInfo referenceResult;

};
typedef std::list< std::pair<int,int> > AdjacencyList;
typedef std::vector< AdjacencyList >    DirAdjacencyList;
typedef std::list< DirAdjacencyList >   DirAdjacencyListOfRectangleList;

void poly2rect(PolygonList &polygonList, RectangleList &rectList);
void generateConnectedRects( RectangleList &rectList, ConnectedRectangleList &rectListList );
void computeAdjacency(const RectangleList               &rectList,
                      DirAdjacencyListOfRectangleList   &adjacency,
                      DirAdjacencyListOfRectangleList   &compAdjacency);

void generate3dRects(const RectangleList                    &rect2dList,
                     const DirAdjacencyListOfRectangleList  &compAdjacency,
                     const int                              elevationBottom,
                     const int                              elevationTop,
                     const int                              layerIndex,
                     Conductor                              &cond);

void discretizeDisjointSurface(ConductorFPList &cond, const float suggestedPanelSize);
void instantiateBasisFunction (ConductorFPList &cond, const float archLength,
                               const float projectionDistance, const float projectionMergeDistance);

// int main();


//****
//*
//* File writer
//*
//*
void writeFastcapFile(
        const std::string &outputFileName,
        const ConductorFPList &cond)
        throw (FileNotFoundError);
void writeCapletFile(
        const std::string &outputFileName,
        const ConductorFPList &cond)
        throw (FileNotFoundError);


//****
//*
//* Debug functions
//*
//*
void printPolygon(Polygon &poly);
void printPolygonList(PolygonList &polyList);
void printRectList(RectangleList &rectList);
void printRectListMatlab(RectangleList &rectList, Dir dir, std::ostream &fout);
void printConductorListMatlab(ConductorList &conductorList);
void printRectMap(RectangleMap &rectMap);

#endif // GEOLOADER_H
