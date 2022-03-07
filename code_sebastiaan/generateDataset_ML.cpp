//--- BSO-toolbox header files ---//
#include <bso/spatial_design/ms_building.hpp>
#include <bso/spatial_design/cf_building.hpp> 

// C++ References
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <cstring>
#include <cmath>
#include <vector>

// Eigen C++ library
#include <Eigen/Dense>

// Namespaces 
using namespace std;
using namespace Eigen;

// The simulaton function uses the origin and dimensions vectors of the BSDs ("origin_dimensions_BSDs.txt") as input
// Via the BSO-toolbox, the BSDs are transformed into Conformal designs
// INPUT:
    // The input is saved in a new text file: "inputBSD.txt", and can be used as the input features in the Machine Learning process
    // The inputBSD file can be enhanced by adding Number or One-hot Encoding: "inputBSD_NumberEncoded.txt", "inputBSD_OneHotEncoded.txt"
    // In "cornerverticesBSD.txt", the BSDs are described by the corner-vertices of the quad-hexahedron spaces.
// OUTPUT:
    // The output is the Conformal (CF) model of each BSD, described by the corner-vertices of conformal cuboids
    // Saved in "outputCF.txt"

void simulation(string inputsFile, string transformation, string geometry, vector<string> typesIncluded )
{
    // Read BSD origin and dimensions file
    string InputsLine;
    ifstream Inputs(inputsFile);                // = "origin_dimensions_BSDs.txt"                                
    ofstream msinput("ms_input_file.txt");                                   
    ofstream inputBSDf("inputBSD.txt", ios::trunc);    
    ofstream inputBSDf_OneHotEncoded("inputBSD_OneHotEncoded.txt", ios::trunc); 
    ofstream inputBSDf_NumberEncoded("inputBSD_NumberEncoded.txt", ios::trunc);    
    ofstream cornerverticesBSDf("cornerverticesBSD_ML.txt", ios::trunc);                  
    ofstream outputCFf("outputCF.txt", ios::trunc);
    int r = 0;     
    int nlines = 0;      

    int topTypes, frontTypes, behindTypes, leftTypes, rightTypes;
    if (find(typesIncluded.begin(), typesIncluded.end(), "top"   ) != typesIncluded.end()) { topTypes    = 81; } else { topTypes    = 0; }
    if (find(typesIncluded.begin(), typesIncluded.end(), "front" ) != typesIncluded.end()) { frontTypes  = 27; } else { frontTypes  = 0; }
    if (find(typesIncluded.begin(), typesIncluded.end(), "behind") != typesIncluded.end()) { behindTypes = 27; } else { behindTypes = 0; }
    if (find(typesIncluded.begin(), typesIncluded.end(), "left"  ) != typesIncluded.end()) { leftTypes   = 27; } else { leftTypes   = 0; }
    if (find(typesIncluded.begin(), typesIncluded.end(), "right" ) != typesIncluded.end()) { rightTypes  = 27; } else { rightTypes  = 0; }
    int totalTypes = topTypes + frontTypes + behindTypes + leftTypes + rightTypes;
    
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> I_matrix;
    I_matrix = Eigen::MatrixXd::Identity(totalTypes, totalTypes);
    IOFormat CleanFmt(5, 0, ", ", "\n");
    
    
    if (Inputs.is_open())
    {
        while ( !Inputs.eof() )
        {
            ofstream msinput("ms_input_file.txt", ofstream::trunc);                                              
            while ( getline(Inputs,InputsLine) && InputsLine != ""  )
            {
                msinput << InputsLine << endl;
            }
            nlines++;
            bso::spatial_design::ms_building MS("ms_input_file.txt");

            // write BSD input to "inputBSD.txt" file, so it can be used as features
            string line;
            string dna;
            ifstream input("ms_input_file.txt");                
            if (input.is_open())                                                  
            {
                while ( getline (input, line) )
                {
                    line.erase(0,4);                                               
                    line.erase(line.end()-1, line.end());                         
                    dna += line;                                                  
                }
                dna.erase(dna.end()-1, dna.end());     
                ofstream inputBSDf("inputBSD.txt", ios::app);
                inputBSDf << dna << endl;
                inputBSDf.close();

                // dna is a string with several integers
                // convert string to c++ integers 
                vector<int> inputBSD_values;
                stringstream dna_stream(dna);
                string item;
                while ( getline(dna_stream, item, ',') ) 
                {
                    inputBSD_values.push_back(stoi(item));
                }
                int width1  = inputBSD_values[0];    int width2  = inputBSD_values[6];
                int depth1  = inputBSD_values[1];    int depth2  = inputBSD_values[7];
                int height1 = inputBSD_values[2];    int height2 = inputBSD_values[8];
                int x1      = inputBSD_values[3];    int x2      = inputBSD_values[9];
                int y1      = inputBSD_values[4];    int y2      = inputBSD_values[10];
                int z1      = inputBSD_values[5];    int z2      = inputBSD_values[11];             

                // Number encoding and One-hot encoding are applied on the dataset
                ofstream inputBSDf_OneHotEncoded("inputBSD_OneHotEncoded.txt", ios::app); 
                ofstream inputBSDf_NumberEncoded("inputBSD_NumberEncoded.txt", ios::app); 
                int w, d, h, x, y, z;
                if (x1 +  width1 ==  x2 +  width2) { w = 0; }     if (x1 +  width1 < x2 +  width2) { w = 1; }     if (x1 +  width1 > x2 +  width2) { w = 2; }
                if (y1 +  depth1 ==  y2 +  depth2) { d = 0; }     if (y1 +  depth1 < y2 +  depth2) { d = 1; }     if (y1 +  depth1 > y2 +  depth2) { d = 2; }
                if (z1 + height1 ==  z2 + height2) { h = 0; }     if (z1 + height1 < z2 + height2) { h = 1; }     if (z1 + height1 > z2 + height2) { h = 2; }
                if (          x1 ==            x2) { x = 0; }     if (          x1 <           x2) { x = 1; }     if (          x1 >           x2) { x = 2; }
                if (          y1 ==            y2) { y = 0; }     if (          y1 <           y2) { y = 1; }     if (          y1 >           y2) { y = 2; }
                if (          z1 ==            z2) { z = 0; }     if (          z1 <           z2) { z = 1; }     if (          z1 >           z2) { z = 2; }
                int itype;
                if (z2 == height1) {                                       // "top"
                    itype = 0;                                  
                    for (int iw = 0; iw < 3; iw++)
                    {
                        if (iw != w) { itype+=27; continue; } else {
                            for (int id = 0; id < 3; id++)
                            {
                                if (id != d) { itype+=9; continue; } else {
                                    for (int ix = 0; ix < 3; ix++)
                                    {
                                        if (ix != x) { itype+=3; continue; } else {
                                            for (int iy = 0; iy < 3; iy++)
                                            {
                                                if (iy != y) { itype++; continue; }
                                                else {  inputBSDf_OneHotEncoded << I_matrix.block(itype,0,1,totalTypes).format(CleanFmt) << "," << dna << endl;
                                                        inputBSDf_NumberEncoded << itype+1 << "," << dna << endl; }
                                            }}
                                    }}
                            }}
                    }
                } else if (z2 == z1 && y2 == y1 - depth2) {               // "front"
                    itype = 0 + topTypes;
                    for (int iw = 0; iw < 3; iw++)
                            {
                                if (iw != w) { itype+=9; continue; } else {
                                    for (int ih = 0; ih < 3; ih++)
                                    {
                                        if (ih != h) { itype+=3; continue; } else {
                                            for (int ix = 0; ix < 3; ix++)
                                            {
                                                if (ix != x) { itype++; continue; }
                                                else {  inputBSDf_OneHotEncoded << I_matrix.block(itype,0,1,totalTypes).format(CleanFmt) << "," << dna << endl;
                                                        inputBSDf_NumberEncoded << itype+1 << "," << dna << endl; }
                                            }}
                                    }}
                            }
                } else if (z2 == z1 && y2 == y1 + depth1) {               // "behind"
                    itype = 0 + topTypes + frontTypes;
                    for (int iw = 0; iw < 3; iw++)
                            {
                                if (iw != w) { itype+=9; continue; } else {
                                    for (int ih = 0; ih < 3; ih++)
                                    {
                                        if (ih != h) { itype+=3; continue; } else {
                                            for (int ix = 0; ix < 3; ix++)
                                            {
                                                if (ix != x) { itype++; continue; }
                                                else {  inputBSDf_OneHotEncoded << I_matrix.block(itype,0,1,totalTypes).format(CleanFmt) << "," << dna << endl;
                                                        inputBSDf_NumberEncoded << itype+1 << "," << dna << endl; }
                                            }}
                                    }}
                            }
                } else if (z2 == z1 && x2 == x1 - width2) {               // "left"
                    itype = 0 + topTypes + frontTypes + behindTypes;
                    for (int id = 0; id < 3; id++)
                            {
                                if (id != d) { itype+=9; continue; } else {
                                    for (int ih = 0; ih < 3; ih++)
                                    {
                                        if (ih != h) { itype+=3; continue; } else {
                                            for (int iy = 0; iy < 3; iy++)
                                            {
                                                if (iy != y) { itype++; continue; }
                                                else {  inputBSDf_OneHotEncoded << I_matrix.block(itype,0,1,totalTypes).format(CleanFmt) << "," << dna << endl;
                                                        inputBSDf_NumberEncoded << itype+1 << "," << dna << endl; }
                                            }}
                                    }}
                            }
                } else if (z2 == z1 && x2 == x1 + width1) {               // "right"
                    itype = 0 + topTypes + frontTypes + behindTypes + leftTypes;
                    for (int id = 0; id < 3; id++)
                            {
                                if (id != d) { itype+=9; continue; } else {
                                    for (int ih = 0; ih < 3; ih++)
                                    {
                                        if (ih != h) { itype+=3; continue; } else {
                                            for (int iy = 0; iy < 3; iy++)
                                            {
                                                if (iy != y) { itype++; continue; }
                                                else {  inputBSDf_OneHotEncoded << I_matrix.block(itype,0,1,totalTypes).format(CleanFmt) << "," << dna << endl;
                                                        inputBSDf_NumberEncoded << itype+1 << "," << dna << endl; }
                                            }}
                                    }}
                            }
                }                           
                inputBSDf_OneHotEncoded.close();
                inputBSDf_NumberEncoded.close();
                input.close();
            }
            bso::spatial_design::cf_building CF(MS);

            // Write corner-vertices of BSD to "cornerverticesBSD.txt" file   
            ofstream cornerverticesBSDf("cornerverticesBSD_ML.txt", ios::app);        
            int ip = 0;
            for (const auto& i_cornerverticesBSD : CF.cfPoints())
            {
                for (const auto& j_cornerverticesBSD : *i_cornerverticesBSD)
                {
                    if (ip == 0) {
                        cornerverticesBSDf << j_cornerverticesBSD ;  
                    } else {
                        cornerverticesBSDf << "," << j_cornerverticesBSD  ; 
                    }    
                    ip++;
                }
            }
            cornerverticesBSDf << endl;
            cornerverticesBSDf.close();


    //-------------- CONFORMAL TRANSFORMATION --------------//
    //  Write targets to outputCF file (Conformal model described by corner-vertices)
            ofstream outputCFf("outputCF.txt", ios::app);
            if (transformation == "conformal")
            {
                int ii = 0;
                if (geometry == "vertex")
                {
                    for (const auto& i : CF.cfVertices())
                    {
                        if (ii > 0) { outputCFf << "," ; }
                        ii++;
                        int jj = 0;
                        for (const auto& j : *i)
                        {
                            if (jj == 0) {
                                outputCFf << j ;
                            } else if (jj >= 0) {
                                outputCFf << "," << j ; }
                            jj++;
                        }
                    }
                } else if (geometry == "line") 
                {
                    for (const auto& i : CF.cfLines())
                    {
                        if (ii > 0) { outputCFf << "," ; }
                        ii++;
                        int jj = 0;
                        for (const auto& j : *i)
                        {
                            if (jj == 0) {
                                outputCFf << j[0] << "," << j[1] << "," << j[2];
                            } else if (jj >= 0) {
                                outputCFf << "," << j[0] << "," << j[1] << "," << j[2]; }
                            jj++;
                        }
                    }
                } 
                else if (geometry == "rectangle")
                {
                    for (const auto& i : CF.cfRectangles())
                    {
                        if (ii > 0) { outputCFf << "," ; }
                        ii++;
                        int jj = 0;
                        for (const auto& j : *i)
                        {
                            if (jj == 0) {
                                outputCFf << j[0] << "," << j[1] << "," << j[2];
                            } else if (jj >= 0) {
                                outputCFf << "," << j[0] << "," << j[1] << "," << j[2]; }
                            jj++;
                        }
                    }
                } 
                else if (geometry == "cuboid")
                {
                    for (const auto& i : CF.cfCuboids())
                    {
                        if (ii > 0) { outputCFf << "," ; }
                        ii++;
                        int jj = 0;
                        for (const auto& j : *i)
                        {
                            if (jj == 0) {
                                outputCFf << j[0] << "," << j[1] << "," << j[2];
                            } else if (jj >= 0) {
                                outputCFf << "," << j[0] << "," << j[1] << "," << j[2]; }
                            jj++;
                        }
                    }
                } 
            } 
            outputCFf << endl;           
        }
    }
    Inputs.close();

    cout << "\n" << "End of simulations" << endl;    
}


// --- MAIN --- //
int main(int argc, char* argv[])
{
  //---Simulation---//
  // DEFAULT:
  //    Input file          = "origin_dimensions_BSDs.txt"
  //    Transformation      = "conformal"
  //    Geometry entity     = "cuboid"
  //    {types included}    = {"top", "front", "behind", "left", "right"}
  simulation("origin_dimensions_BSDs_ML.txt", "conformal", "cuboid", {"top", "front", "behind", "left", "right"} );
}
