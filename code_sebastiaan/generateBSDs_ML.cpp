// C++ References
#include <iostream>
#include <algorithm>
#include <functional>
#include <random>
#include <vector>
#include <fstream>
#include <string>

// Namespaces 
using namespace std;

// Data generation of building spatial designs (BSDs) with 2 spaces
// Wide range of possible configurations, randomly defined
// 5 Different variants: 'top' || 'front' || 'behind' || 'left' || 'right' 

// Input Parameters
int nBSDs = 20;            // nr. of Building Spatial Designs (BSDs) per variant.
int nSpaces = 2;            // nr. of spaces in BSD. Currently, fixed to 2
int step = 1000;            // The step in the dimensions range. 
                            // --> For example, if the width ranges from 4000-8000, the values 4000, 5000, 6000, 7000, and 8000 are considered

// The variants which are included in the dataset, can be one or multiple
vector<string> typesIncluded = {"top", "front", "behind", "left", "right"};     


// Space dimensions (w, d, h) are randomly defined 
class spaceDimensions {
    private:
        int minW, maxW, minD, maxD, minH, maxH;
    public:
        std::vector<int> width, depth, height;

        // CONSTRUCTOR: space dimensions range
        spaceDimensions(int miw, int maw, int mid, int mad, int mih, int mah) {
            minW = miw;     maxW = maw;                                         //mm        Width
            minD = mid;     maxD = mad;                                         //mm        Depth
            minH = mih;     maxH = mah;                                         //mm        Height
        }

        // METHOD: random number generator for dimensions
        void randomNumberGen() {
            for (int jw = 0; jw <= (nBSDs*typesIncluded.size())/((maxW-minW)/step+1); jw++)
            {
                for (int iw=minW; iw<=maxW; iw+=step) 
                {
                    width.push_back(iw);                    
                }
                int iw = minW;
            }
            std::random_shuffle(width.begin(), width.end());
            for (int jd = 0; jd <= (nBSDs*typesIncluded.size())/((maxD-minD)/step+1); jd++)
            {
                for (int id=minD; id<=maxD; id+=step) 
                {
                    depth.push_back(id);
                }
                int id = minD;
            }
            std::random_shuffle(depth.begin(), depth.end());
            for (int jh = 0; jh <= (nBSDs*typesIncluded.size())/((maxH-minH)/step+1); jh++)
            {
                for (int ih=minH; ih<=maxH; ih+=step) 
                {
                    height.push_back(ih);
                }
                int ih = minH;
            }
            std::random_shuffle(height.begin(), height.end());
        }
};


// --- MAIN --- //
int main(int argc, char *argv[])
{   
    std::cout << "Start BSD Generation" << std::endl;

    // Space 1  (min Width, max Width, min Depth, max Depth, min Height, max Height)
    // Define range of dimensions
    spaceDimensions space1(4000, 8000, 4000, 8000, 2000, 6000);
    space1.randomNumberGen();

    // Space 2  (min Width, max Width, min Depth, max Depth, min Height, max Height)
    // Define range of dimensions
    spaceDimensions space2(4000, 8000, 4000, 8000, 2000, 6000);
    space2.randomNumberGen();

    // Initial origin space 1 = {10000,10000,0}
    vector<int> coor10, coor0;
    for (int ic=0; ic<nBSDs*typesIncluded.size(); ic++) coor10.push_back(10000);
    for (int ic=0; ic<nBSDs*typesIncluded.size(); ic++) coor0.push_back(0);         
    std::vector<int> coorX1 = coor10, coorY1 = coor10, coorZ1 = coor0;


    // Generate wide range of possible configurations, for each included variant
    vector<int> coorX2;
    vector<int> coorY2;
    vector<int> coorZ2;
    int nrOfTypes = 0;

    for (int i = 0; i < typesIncluded.size(); i++ ) {
        if ( typesIncluded[i] == "top" )
        {
            nrOfTypes++;                                                              
            for (int i=(nrOfTypes-1)*nBSDs; i<nrOfTypes*nBSDs; i++)         
            {
                coorX2.push_back( (rand() % (((coorX1[i] + space1.width[i]) - (coorX1[i] - space2.width[i] + step)) / step )) * step + (coorX1[i] - space2.width[i] + step));
                coorY2.push_back( (rand() % (((coorY1[i] + space1.depth[i]) - (coorY1[i] - space2.depth[i] + step)) / step )) * step + (coorY1[i] - space2.depth[i] + step));
                coorZ2.push_back(space1.height[i]);
            }
        }   
        if ( typesIncluded[i] == "front" )
        {
            nrOfTypes++;
            for (int i=(nrOfTypes-1)*nBSDs; i<nrOfTypes*nBSDs; i++)
            {
                coorX2.push_back( (rand() % (((coorX1[i] + space1.width[i]) - (coorX1[i] - space2.width[i] + step)) / step )) * step + (coorX1[i] - space2.width[i] + step));
                coorY2.push_back(coorY1[i] - space2.depth[i]);
                coorZ2.push_back(coorZ1[i]);
            }
        }   
        if ( typesIncluded[i] == "behind" )
        {
            nrOfTypes++;
            for (int i=(nrOfTypes-1)*nBSDs; i<nrOfTypes*nBSDs; i++)
            {
                coorX2.push_back( (rand() % (((coorX1[i] + space1.width[i]) - (coorX1[i] - space2.width[i] + step)) / step )) * step + (coorX1[i] - space2.width[i] + step));
                coorY2.push_back(coorY1[i] + space1.depth[i]);
                coorZ2.push_back(coorZ1[i]);
            }
        }  
        if ( typesIncluded[i] == "left" )
        {
            nrOfTypes++;
            for (int i=(nrOfTypes-1)*nBSDs; i<nrOfTypes*nBSDs; i++)
            {
                coorX2.push_back(coorX1[i] - space2.width[i]);
                coorY2.push_back( (rand() % (((coorY1[i] + space1.depth[i]) - (coorY1[i] - space2.depth[i] + step)) / step )) * step + (coorY1[i] - space2.depth[i] + step));
                coorZ2.push_back(coorZ1[i]);
            }
        }   
        if ( typesIncluded[i] == "right" )
        {
            nrOfTypes++;
            for (int i=(nrOfTypes-1)*nBSDs; i<nrOfTypes*nBSDs; i++)
            {
                coorX2.push_back(coorX1[i] + space1.width[i]);
                coorY2.push_back( (rand() % (((coorY1[i] + space1.depth[i]) - (coorY1[i] - space2.depth[i] + step)) / step )) * step + (coorY1[i] - space2.depth[i] + step));
                coorZ2.push_back(coorZ1[i]);
            }
        }   
    }

    // Write origins and dimensions of BSDs in text file: "origin_dimensions_BSDs.txt"
    // This text file is according to the Movable/Sizable Model in the BSO-toolbox ("ms_input_file.txt")
    // The file is used as input for the simulation ("simulation.cpp")
    std::ofstream dataG("origin_dimensions_BSDs_ML.txt", std::ofstream::trunc);
    for (int i=0; i < nBSDs*nrOfTypes; i++)
    {
        // Space 1
        dataG << 'R' << ',' << '1' << ',';
        dataG << space1.width[i] << ',';     
        dataG << space1.depth[i] << ',';
        dataG << space1.height[i] << ',';
        dataG << coorX1[i] << ',' << coorY1[i] << ',' << coorZ1[i] << ',' << 'A' << std::endl;
        // Space 2
        dataG << 'R' << ',' << '2' << ',';
        dataG << space2.width[i] << ',';     
        dataG << space2.depth[i] << ',';
        dataG << space2.height[i] << ',';
        dataG << coorX2[i] << ',' << coorY2[i] << ',' << coorZ2[i] << ',' << 'A' << std::endl;

        if (i < nBSDs*nrOfTypes - 1) { dataG << std::endl; }
        else if (i == nBSDs*nrOfTypes - 1) {}
    }
    
    return 0;
}
