//--- C References ---//
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
#include <map>
#include <iterator>
#include <set>
#include <unordered_map>

#include <limits>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <bso/spatial_design/ms_space.hpp>
#include <bso/spatial_design/ms_building.hpp>
#include <bso/utilities/geometry.hpp>


// namespaces
using namespace std;

// ------------------------------ //
// --- GA1 : Quad-Hexahedrons --- //
// ------------------------------ //

// collect coordinates (existing x, y, z values) of the building spatial design (BSD) 
// INPUT    = "testDesign" : BSD described by corner-vertices
// OUTPUT   + nSpaces : nr. of spaces in BSD
//          + total_volume_BSD 
//          + hexaSpaces : spaces of BSD described by quad-hexahedrons according to the BSO-toolbox ("ms_space")
//          + x,y,z_coordinates : vectors with integers of all existing x, y, z values. Duplicates are removed by using the map container.
//          + "ms.txt" : text file with corner-vertices of BSD. This is used for "visualisation.cpp"
void collectCoordinates( string inputsFileBSD,
                            int &nSpaces, double &total_volume_MS,
                            vector<bso::utilities::geometry::quad_hexahedron> &hexaSpaces,
                            vector<int> &x_coordinates, vector<int> &y_coordinates, vector<int> &z_coordinates)
{
    total_volume_MS = 0.0;  nSpaces = 0;
    map<int, int> x_coordinates_map;    map<int, int> y_coordinates_map;    map<int, int> z_coordinates_map; 
    string line;
    ifstream input(inputsFileBSD);  
    ofstream inputBSD("bsd.txt"); 
    if (input.is_open())                                                  
    {
        while ( getline (input, line) )
        {
            nSpaces++;
            bso::spatial_design::ms_space MS(line);   
            bso::utilities::geometry::quad_hexahedron cornerpoints = MS.getGeometry();
            hexaSpaces.push_back(cornerpoints);
            total_volume_MS += cornerpoints.getVolume();
            for (int p = 0; p < 8; p++) {
                x_coordinates_map[cornerpoints.getVertices()[p][0]] = cornerpoints.getVertices()[p][0];
                y_coordinates_map[cornerpoints.getVertices()[p][1]] = cornerpoints.getVertices()[p][1];
                z_coordinates_map[cornerpoints.getVertices()[p][2]] = cornerpoints.getVertices()[p][2];
                inputBSD << cornerpoints.getVertices()[p][0] << "\n" << cornerpoints.getVertices()[p][1] << "\n" << cornerpoints.getVertices()[p][2] << endl;
            }       
        }
    }
    typedef map<int, int> MapType;
    for( MapType::iterator it = x_coordinates_map.begin(); it != x_coordinates_map.end(); ++it ) 
    { x_coordinates.push_back(it->second); }
    for( MapType::iterator it = y_coordinates_map.begin(); it != y_coordinates_map.end(); ++it ) 
    { y_coordinates.push_back(it->second); }
    for( MapType::iterator it = z_coordinates_map.begin(); it != z_coordinates_map.end(); ++it ) 
    { z_coordinates.push_back(it->second); }
}

// get all vertices to create point cloud grid. 
// INPUT    = nSpaces, hexaSpaces, x,y,z_coordinates
// OUTPUT   + allVertices : Make all possible combinations of the previously defined x,y,z values. Duplicates are removed.
//                          AND vertices outside the BSD spaces are removed.
//          + "pnts.txt" : text file with vertices of Point cloud. This is used for "visualisation.cpp"
void generatePointCloud(    int nSpaces, vector<bso::utilities::geometry::quad_hexahedron> hexaSpaces,
                            vector<int> x_coordinates, vector<int> y_coordinates, vector<int> z_coordinates,
                            vector<vector<double>> &PointCloud)
{
    map<vector<double>, vector<double>> PointCloud_map;            
    for (int ix = 0; ix < x_coordinates.size(); ix++) {
        for (int iy = 0; iy < y_coordinates.size(); iy++) {
            for (int iz = 0; iz < z_coordinates.size(); iz++) 
            {
                PointCloud_map[{double(x_coordinates[ix]), double(y_coordinates[iy]), double(z_coordinates[iz])}] = {double(x_coordinates[ix]), double(y_coordinates[iy]), double(z_coordinates[iz])};
            }
        }
    }
    ofstream PointCloudf("pnts.txt"); 
    typedef map<vector<double>, vector<double>> MapbType;
    vector<vector<double>> PointCloudv;
    double mTol = 1e-3;     
    int a = 0;
    for( MapbType::iterator it = PointCloud_map.begin(); it != PointCloud_map.end(); ++it ) 
    {
        // if point is Inside or On BSD space[i], add to PointCloud:
        bso::utilities::geometry::vertex point = {it->second[0], it->second[1], it->second[2]}; 
        for (int s = 0; s < nSpaces; s++)
        {
            if ( hexaSpaces[s].isInsideOrOn(point, mTol) )
            {
                PointCloudv.push_back( it->second );
                PointCloudf << it->second[0] << endl << it->second[1] << endl << it->second[2] << endl;
                break;
            }
        }
    }
    cout << "total unique vertices in Point cloud (inside BSD spaces):    " << PointCloudv.size() << endl;
    PointCloud = PointCloudv; 
}


// Individuals of GA1: Quad-hexahedrons described by a Class.
//          + points8 : combination of 8 vertices that form the quad-hexahedron
//          + fitness function GA1
//              - Orthogonal designs:       (O)
//              - Non-Orthogonal designs:   (NO)
//          + Convexity : check if quad-hexahedron is convex or not. 
class individual_GA1 {
    public:
        vector<vector<double>> points8;        

        double fitness_GA1() {
            double fitness;

        // ------------ begin fitness Orthogonal --------------- //
        //     
        //     int score_nx = 0; int score_ny = 0; int score_nz = 0;
        //  // X:
        //     if ((points8[0][0] == points8[1][0]) && (points8[0][0] == points8[2][0]) && (points8[0][0] == points8[3][0]) ) {score_nx++  ;}
        //     if ((points8[4][0] == points8[5][0]) && (points8[4][0] == points8[6][0]) && (points8[4][0] == points8[7][0]) ) {score_nx++  ;}
        //     if (points8[0][0] == points8[4][0]) {score_nx-=1 ;}     // avoid line instead of rectangle
        //  // Y:
        //     if ((points8[0][1] == points8[1][1]) && (points8[0][1] == points8[4][1]) && (points8[0][1] == points8[5][1]) ) {score_ny++  ;}
        //     if ((points8[2][1] == points8[3][1]) && (points8[2][1] == points8[6][1]) && (points8[2][1] == points8[7][1]) ) {score_ny++  ;}
        //     if (points8[0][1] == points8[2][1]) {score_ny-=1 ;}     
        //  // Z:
        //     if ((points8[0][2] == points8[2][2]) && (points8[0][2] == points8[4][2]) && (points8[0][2] == points8[6][2]) ) {score_nz++  ;}
        //     if ((points8[1][2] == points8[3][2]) && (points8[1][2] == points8[5][2]) && (points8[1][2] == points8[7][2]) ) {score_nz++  ;}
        //     if (points8[0][2] == points8[1][2]) {score_nz-=1 ;}

        //     fitness =  4 + score_nx + score_ny + score_nz ;       ;
        // ------------ end   fitness Orthogonal --------------- //


        // ------------ begin fitness NON-Orthogonal --------------- //
            int score_nx = 0; int score_ny = 0; int score_nz = 0;
        // Walls are VERTICAL
         // X:
            if (points8[0][0] == points8[1][0]) {score_nx++  ;}
            if (points8[2][0] == points8[3][0]) {score_nx++  ;}
            if (points8[4][0] == points8[5][0]) {score_nx++  ;}
            if (points8[6][0] == points8[7][0]) {score_nx++  ;}
            // prevent duplicates, 4 unique 'columns'
            if ( (points8[0][0] == points8[2][0]) && (points8[0][0] == points8[4][0]) && (points8[0][0] == points8[6][0]) ) {score_nx-=1 ;}  
            // avoid line (3 points on one) instead of rectangle
            if ((points8[0][0] == points8[2][0]) && (points8[0][0] == points8[4][0])) {score_nx-=1 ;}     
            if ((points8[0][0] == points8[2][0]) && (points8[0][0] == points8[6][0])) {score_nx-=1 ;}
            if ((points8[0][0] == points8[4][0]) && (points8[0][0] == points8[6][0])) {score_nx-=1 ;}  
            if ((points8[2][0] == points8[4][0]) && (points8[2][0] == points8[6][0])) {score_nx-=1 ;}
         // Y:
            if (points8[0][1] == points8[1][1]) {score_ny++  ;}
            if (points8[2][1] == points8[3][1]) {score_ny++  ;}
            if (points8[4][1] == points8[5][1]) {score_ny++  ;}
            if (points8[6][1] == points8[7][1]) {score_ny++  ;}
            if ( (points8[0][1] == points8[2][1]) && (points8[0][1] == points8[4][1]) && (points8[0][1] == points8[6][1]) ) {score_ny-=1 ;}  
            if ((points8[0][1] == points8[2][1]) && (points8[0][1] == points8[4][1])) {score_ny-=1 ;}     
            if ((points8[0][1] == points8[2][1]) && (points8[0][1] == points8[6][1])) {score_ny-=1 ;}   
            if ((points8[0][1] == points8[4][1]) && (points8[0][1] == points8[6][1])) {score_ny-=1 ;}   
            if ((points8[2][1] == points8[4][1]) && (points8[2][1] == points8[6][1])) {score_ny-=1 ;}    
       
        // Floors are HORIZONTAL
         // Z:
            if ((points8[0][2] == points8[2][2]) && (points8[0][2] == points8[4][2]) && (points8[0][2] == points8[6][2]) ) {score_nz++  ;}
            if ((points8[1][2] == points8[3][2]) && (points8[1][2] == points8[5][2]) && (points8[1][2] == points8[7][2]) ) {score_nz++  ;}
            if (points8[0][2] == points8[1][2]) {score_nz-=1 ;}

        // CHECK 3 vertices on one LINE
            bso::utilities::geometry::vertex vx1 = {points8[0][0], points8[0][1], points8[0][2]};
            bso::utilities::geometry::vertex vx2 = {points8[2][0], points8[2][1], points8[2][2]};
            bso::utilities::geometry::vertex vx3 = {points8[4][0], points8[4][1], points8[4][2]};
            bso::utilities::geometry::vertex vx4 = {points8[6][0], points8[6][1], points8[6][2]};
            int score_nL = 0;
            bso::utilities::geometry::line_segment lineA = {vx1, vx2};
            bso::utilities::geometry::line_segment lineB = {vx2, vx4};
            bso::utilities::geometry::line_segment lineC = {vx4, vx3};
            bso::utilities::geometry::line_segment lineD = {vx3, vx1};
            bso::utilities::geometry::vector vectorA = lineA.getVector();
            bso::utilities::geometry::vector vectorB = lineB.getVector();
            bso::utilities::geometry::vector vectorC = lineC.getVector();
            bso::utilities::geometry::vector vectorD = lineD.getVector();
            if (vectorA.isParallel(vectorB))
            { score_nL-=1; }
            if (vectorB.isParallel(vectorC))
            { score_nL-=1; }
            if (vectorC.isParallel(vectorD))
            { score_nL-=1; }
            if (vectorD.isParallel(vectorA))
            { score_nL-=1; }
            
            fitness = score_nx + score_ny + score_nz + score_nL;
        // ------------ end   fitness NON-Orthogonal --------------- //
   
            return fitness;
        }

        int convexity() {
            // CHECK CONVEXITY  
            bso::utilities::geometry::vertex vx1 = {points8[0][0], points8[0][1], points8[0][2]};
            bso::utilities::geometry::vertex vx2 = {points8[2][0], points8[2][1], points8[2][2]};
            bso::utilities::geometry::vertex vx3 = {points8[4][0], points8[4][1], points8[4][2]};
            bso::utilities::geometry::vertex vx4 = {points8[6][0], points8[6][1], points8[6][2]};
            bso::utilities::geometry::line_segment diagonal1 = {vx1, vx4};
            bso::utilities::geometry::line_segment diagonal2 = {vx2, vx3};
            bso::utilities::geometry::line_segment diagonal3 = {vx1, vx3};
            bso::utilities::geometry::line_segment diagonal4 = {vx2, vx4};
            bso::utilities::geometry::vertex pIntersectionE;
            double mETol = 1e-3;
            int convex;
            if ( (diagonal1.intersectsWith(diagonal2, pIntersectionE, mETol)) || (diagonal3.intersectsWith(diagonal4, pIntersectionE, mETol)) )
            { convex = 1;     // CONVEX
            } else {            
              convex = 0;     // NON-CONVEX
            }   
            return convex;
        }
 };

// Initialise population of GA1: Quad-hexahedrons. Make N combinations of 8 points
// INPUT    = N : Population Size
//          = PointCloud : all vertices in Point cloud
// OUTPUT   + population_GA1 : all Individuals GA1
void initializePopulation_GA1(  int N, vector<vector<double>> PointCloud,
                            vector<individual_GA1> &population_GA1)
{
    cout << "\nInitialise Population GA1 (N = " << N << ")" << endl;
    population_GA1.reserve(N);
    for(int n = 0; n < N; n++)
    {
        population_GA1.push_back(individual_GA1());
        for(int i = 0; i < 8; i++)
        {
            vector<double> randomPoint = PointCloud[rand() % PointCloud.size()];
            population_GA1[n].points8.push_back(randomPoint);
        }
    }       
}

// Mutation (chromosome) 
// Only unique individuals_GA1. Duplicate individuals are removed and replaced by a new unique individual.
// INPUT    = population_GA1, N, PointCloud
// OUTPUT   + unique individuals 
void mutation1_GA1( vector<individual_GA1> population_GA1, int N, vector<vector<double>> PointCloud,
                        vector<individual_GA1> &mutated1_population_GA1)
{           
    map< vector<vector<double>>, vector<vector<double>> > population_GA1_map;
    for (int n = 0; n < N; n++)
    { 
        population_GA1_map[population_GA1[n].points8] = population_GA1[n].points8; 
    }
    if (population_GA1_map.size() < N) 
    {
        while (population_GA1_map.size() < N)
        {
            for (int f = 0; f < (N - population_GA1_map.size()); f++)
            {
                individual_GA1 new_individual;
                for(int p = 0; p < 8; p++)
                {
                    vector<double> randomPoint = PointCloud[rand() % PointCloud.size()];
                    new_individual.points8.push_back(randomPoint);
                }
                population_GA1_map[new_individual.points8] = new_individual.points8;
                new_individual.points8.clear();
            }
        }       
    }
    typedef map< vector<vector<double>>, vector<vector<double>> > mapuType;
    int u = 0;
    for( mapuType::iterator it = population_GA1_map.begin(); it != population_GA1_map.end(); ++it )  
    { 
        mutated1_population_GA1.push_back(individual_GA1());
        mutated1_population_GA1[u].points8 = it->second ; 
        u++; 
    }
}

// Crossovers
// 2 parents create 4 new offsprings by crossover operator
// Crossover X (1-point crossover)
void crossoverX( vector<individual_GA1> population_GA1, int N,
                 vector<individual_GA1> &offspring)
{
    offspring.reserve(N*2);
    for (int n = 0; n < (population_GA1.size()*2); n+=4)
    {
        offspring.push_back(individual_GA1());
        offspring.push_back(individual_GA1());
        offspring.push_back(individual_GA1());
        offspring.push_back(individual_GA1());
        int i = 0;
        for (int p = 0; p < 8; p+=4)
        {
            int k = i%2;
            int m = (i+1)%2;
            offspring[n+0].points8.push_back(population_GA1[(n/2)+k].points8[0+0]);        // AC
            offspring[n+0].points8.push_back(population_GA1[(n/2)+k].points8[0+1]);
            offspring[n+0].points8.push_back(population_GA1[(n/2)+k].points8[0+2]);
            offspring[n+0].points8.push_back(population_GA1[(n/2)+k].points8[0+3]);

            offspring[n+1].points8.push_back(population_GA1[(n/2)+k].points8[4+0]);        // BD
            offspring[n+1].points8.push_back(population_GA1[(n/2)+k].points8[4+1]);
            offspring[n+1].points8.push_back(population_GA1[(n/2)+k].points8[4+2]);
            offspring[n+1].points8.push_back(population_GA1[(n/2)+k].points8[4+3]);

            offspring[n+2].points8.push_back(population_GA1[(n/2)+k].points8[(k*4)+0]);    // AD
            offspring[n+2].points8.push_back(population_GA1[(n/2)+k].points8[(k*4)+1]);
            offspring[n+2].points8.push_back(population_GA1[(n/2)+k].points8[(k*4)+2]);
            offspring[n+2].points8.push_back(population_GA1[(n/2)+k].points8[(k*4)+3]);

            offspring[n+3].points8.push_back(population_GA1[(n/2)+k].points8[(m*4)+0]);    // BC
            offspring[n+3].points8.push_back(population_GA1[(n/2)+k].points8[(m*4)+1]);
            offspring[n+3].points8.push_back(population_GA1[(n/2)+k].points8[(m*4)+2]);
            offspring[n+3].points8.push_back(population_GA1[(n/2)+k].points8[(m*4)+3]);

            i++;
        }   
    }
}           
// Crossover Y (3-point crossover)
void crossoverY( vector<individual_GA1> population_GA1, int N,
                 vector<individual_GA1> &offspring)
{
    offspring.reserve(N*2);
    for (int n = 0; n < population_GA1.size()*2; n+=4)
    {
        offspring.push_back(individual_GA1());
        offspring.push_back(individual_GA1());
        offspring.push_back(individual_GA1());
        offspring.push_back(individual_GA1());
        for (int p = 0; p < 8; p+=4)
        {
            offspring[n+0].points8.push_back(population_GA1[(n/2)+0].points8[p+0]);        // AD
            offspring[n+0].points8.push_back(population_GA1[(n/2)+0].points8[p+1]);
            offspring[n+0].points8.push_back(population_GA1[(n/2)+1].points8[p+2]);        
            offspring[n+0].points8.push_back(population_GA1[(n/2)+1].points8[p+3]);

            offspring[n+1].points8.push_back(population_GA1[(n/2)+0].points8[p+0]);        // AC
            offspring[n+1].points8.push_back(population_GA1[(n/2)+0].points8[p+1]);
            offspring[n+1].points8.push_back(population_GA1[(n/2)+1].points8[p+0]);        
            offspring[n+1].points8.push_back(population_GA1[(n/2)+1].points8[p+1]);

            offspring[n+2].points8.push_back(population_GA1[(n/2)+1].points8[p+0]);        // BC
            offspring[n+2].points8.push_back(population_GA1[(n/2)+1].points8[p+1]);
            offspring[n+2].points8.push_back(population_GA1[(n/2)+0].points8[p+2]);        
            offspring[n+2].points8.push_back(population_GA1[(n/2)+0].points8[p+3]);

            offspring[n+3].points8.push_back(population_GA1[(n/2)+0].points8[p+2]);        // BD
            offspring[n+3].points8.push_back(population_GA1[(n/2)+0].points8[p+3]);
            offspring[n+3].points8.push_back(population_GA1[(n/2)+1].points8[p+2]);        
            offspring[n+3].points8.push_back(population_GA1[(n/2)+1].points8[p+3]);
        }   
    }
}         
// Crossover Z (7-point crossover)
void crossoverZ( vector<individual_GA1> population_GA1, int N,
                 vector<individual_GA1> &offspring)
{
    offspring.reserve(N*2);
    for (int n = 0; n < population_GA1.size()*2; n+=4)
    {
        offspring.push_back(individual_GA1());
        offspring.push_back(individual_GA1());
        offspring.push_back(individual_GA1());
        offspring.push_back(individual_GA1());
        for (int p = 0; p < 8; p+=2)
        {
            offspring[n+0].points8.push_back(population_GA1[(n/2)+0].points8[p+0]);        // AD
            offspring[n+0].points8.push_back(population_GA1[(n/2)+1].points8[p+1]);

            offspring[n+1].points8.push_back(population_GA1[(n/2)+0].points8[p+0]);        // AC
            offspring[n+1].points8.push_back(population_GA1[(n/2)+1].points8[p+0]);

            offspring[n+2].points8.push_back(population_GA1[(n/2)+0].points8[p+1]);        // BC
            offspring[n+2].points8.push_back(population_GA1[(n/2)+1].points8[p+0]);          

            offspring[n+3].points8.push_back(population_GA1[(n/2)+0].points8[p+1]);        // BD
            offspring[n+3].points8.push_back(population_GA1[(n/2)+1].points8[p+1]);
        }   
    }
}           

// 2 Parents and 4 offsprings are merged to intermediate population (with increased size)
// OUTPUT   + merged_population_GA1 : intermediate population
void merge( vector<individual_GA1> population_GA1, vector<individual_GA1> offspring, int N,
            vector<individual_GA1> &merged_population_GA1)
{
    merged_population_GA1.reserve(N*3);
    int o1 = 0; int o2 = 1;
    for (int i = 0; i < N*3; i+=3)
    {
        merged_population_GA1.push_back(individual_GA1());
        merged_population_GA1.push_back(individual_GA1());
        merged_population_GA1.push_back(individual_GA1());
        for (int p = 0; p < 8; p++)
        {
            merged_population_GA1[i].points8.push_back(population_GA1[i/3].points8[p]);
            merged_population_GA1[i+1].points8.push_back(offspring[o1].points8[p]);
            merged_population_GA1[i+2].points8.push_back(offspring[o2].points8[p]);
        }
        o1+=2; o2+=2;
    }
}           

// Mutation (gene)
// Duplicate vertices in an individual are replaced by new unique vertices randomly chosen from Point cloud.
void mutation2_GA1( vector<individual_GA1> merged_population_GA1, vector<vector<double>> PointCloud, 
                    vector<individual_GA1> &mutated2_population_GA1)
{
    map<vector<double>, vector<double>> points8_map;
    for (int n = 0; n < merged_population_GA1.size(); n++)
    {
        for (int p = 0; p < 8; p++)
        {
            points8_map[merged_population_GA1[n].points8[p]] = merged_population_GA1[n].points8[p];
        }
        while (points8_map.size() < 8)
        {
            for (int f = 0; f < (8 - points8_map.size()); f++)
            {
                int randomN = rand() % PointCloud.size();
                points8_map[PointCloud[randomN]] = PointCloud[randomN];
            }
        }
        merged_population_GA1[n].points8.clear();
        mutated2_population_GA1.push_back(individual_GA1());
        typedef map< vector<double>, vector<double> > mapbType;
        for( mapbType::iterator it = points8_map.begin(); it != points8_map.end(); ++it ) 
        { 
            mutated2_population_GA1[n].points8.push_back( it->second ); 
        }
        points8_map.clear();
    }
}

// Selection of the fittest
//  2 best individuals are chosen from 2 parents and their 4 offsprings
void selection_GA1( vector<individual_GA1> mutated2_population_GA1, 
                    vector<individual_GA1> &new_population_GA1)
{
    multimap<double, individual_GA1> mapE;
    for (int i = 0; i < mutated2_population_GA1.size(); i+=6)
    {
        for (int j = 0; j < 6; j++)
        {
            mapE.insert(mapE.end(), pair<double, individual_GA1>(mutated2_population_GA1[i+j].fitness_GA1(), mutated2_population_GA1[i+j]) );
        }
        new_population_GA1.push_back(individual_GA1());
        new_population_GA1.push_back(individual_GA1());
        for (int p = 0; p < 8; p++)
        {
            new_population_GA1[int(i/3)].points8.push_back((*prev(mapE.rbegin())).second.points8[p]);
            new_population_GA1[int((i/3)+1)].points8.push_back((*prev(mapE.end())).second.points8[p]);
        }
        mapE.clear();
    }
    double total_fitness = 0.0;
    for (int k = 0; k < new_population_GA1.size(); k++)
    {
        total_fitness = total_fitness + new_population_GA1[k].fitness_GA1();
    }
    double average_fitness = total_fitness/new_population_GA1.size();
    cout << "Average fitness:                                  " << average_fitness << endl;
}    

// If an individual gains the maximum fitness score, it as a perfect quad-hexahedron
// The perfect quad-hexahedrons are used for next GA2.
void get_perfect_Quad_Hexahedron(   vector<individual_GA1> population_GA1,
                                    vector<individual_GA1> &perfect_Quad_Hexahedron)
{           
    map< vector<vector<double>>, vector<vector<double>> > population_GA1_map;
    for (int n = 0; n < population_GA1.size(); n++)
    {
        // ONLY population_GA1 with maximum fitness!
        if (population_GA1[n].fitness_GA1() >= 10)
        { population_GA1_map[population_GA1[n].points8] = population_GA1[n].points8; }
    }
    typedef map< vector<vector<double>>, vector<vector<double>> > mapuType;
    int u = 0;
    for( mapuType::iterator it = population_GA1_map.begin(); it != population_GA1_map.end(); ++it ) { 
        perfect_Quad_Hexahedron.push_back(individual_GA1());
        perfect_Quad_Hexahedron[u].points8 = it->second ; 
        u++; }       
    cout << "size of perfect unique population_GA1:        " << perfect_Quad_Hexahedron.size() << endl;
}           
void save_perfect_Quad_Hexahedron( vector<individual_GA1> perfect_Quad_Hexahedron, 
                                   map< vector<vector<double>>, vector<vector<double>> > &saved_Quad_Hexahedrons_map,
                                   vector<individual_GA1> &saved_Quad_Hexahedrons)
{
    for (int n = 0; n < perfect_Quad_Hexahedron.size(); n++)
    { 
        saved_Quad_Hexahedrons_map[perfect_Quad_Hexahedron[n].points8] = perfect_Quad_Hexahedron[n].points8; 
    }
    cout << "size of saved perfect unique objects:  " << saved_Quad_Hexahedrons_map.size() << endl;
    typedef map< vector<vector<double>>, vector<vector<double>> > mapuType;
    int u = 0;
    for( mapuType::iterator it = saved_Quad_Hexahedrons_map.begin(); it != saved_Quad_Hexahedrons_map.end(); ++it )
    { 
        saved_Quad_Hexahedrons.push_back(individual_GA1());
        saved_Quad_Hexahedrons[u].points8 = it->second ; 
        u++; 
    }
}     

// The quad-hexahedrons are checked if they are inside the BSD spaces.
// Thereafter, the quad-hexahedrons are desrcibed by the BSO-toolbox class "bso:utilities:geometry:quad_hexahdron"
void isInsideBSD(    vector<individual_GA1> saved_Quad_Hexahedrons,
                     vector<bso::utilities::geometry::quad_hexahedron> hexaSpaces, 
                     vector<vector<individual_GA1>> &Quad_Hexahedrons_InsidePerSpace,
                     vector<vector<bso::utilities::geometry::quad_hexahedron>> &Quad_Hexahedrons_perSpace,
                     vector<bso::utilities::geometry::quad_hexahedron> &Quad_Hexahedrons)
{
    for (int ss = 0; ss < hexaSpaces.size(); ss++)
    {
        Quad_Hexahedrons_InsidePerSpace.push_back(vector<individual_GA1>());
        Quad_Hexahedrons_perSpace.push_back(vector<bso::utilities::geometry::quad_hexahedron>());
    }
    int nroftotalelements = 0;
    int nrofelements = 0;
    for (int i = 0; i < saved_Quad_Hexahedrons.size(); i++)
    {
        if (saved_Quad_Hexahedrons[i].convexity() != 0) 
        {
            nroftotalelements++;
            for (int s = 0; s < hexaSpaces.size(); s++)   
            {
                int is = 0;
                for (int p = 0; p < 8; p++)
                {
                    bso::utilities::geometry::vertex point = { saved_Quad_Hexahedrons[i].points8[p][0], saved_Quad_Hexahedrons[i].points8[p][1], saved_Quad_Hexahedrons[i].points8[p][2] };
                    if (hexaSpaces[s].isInsideOrOn(point)) { is++; }
                }
                if (is == 8)
                {
                    nrofelements++;
                    Quad_Hexahedrons_InsidePerSpace[s].push_back(individual_GA1());    
                    int sizepu = Quad_Hexahedrons_InsidePerSpace[s].size();
                    Quad_Hexahedrons_InsidePerSpace[s][sizepu-1].points8 = saved_Quad_Hexahedrons[i].points8;
                }  
            }
        }
    }
    // Quad-hexahedrons are now described by the BSO-toolbox class. These are used for next GA2.
    for (int s = 0; s < Quad_Hexahedrons_InsidePerSpace.size(); s++)
    {
        for (int c = 0; c < Quad_Hexahedrons_InsidePerSpace[s].size(); c++)
        {       
            Quad_Hexahedrons_perSpace[s].push_back(
                bso::utilities::geometry::quad_hexahedron({ {double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[0][0]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[0][1]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[0][2])},
                                                            {double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[1][0]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[1][1]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[1][2])},
                                                            {double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[2][0]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[2][1]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[2][2])},
                                                            {double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[3][0]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[3][1]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[3][2])},
                                                            {double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[4][0]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[4][1]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[4][2])},
                                                            {double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[5][0]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[5][1]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[5][2])},
                                                            {double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[6][0]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[6][1]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[6][2])},
                                                            {double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[7][0]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[7][1]), double(Quad_Hexahedrons_InsidePerSpace[s][c].points8[7][2])}}) );
        }
    }
    for(const auto &v: Quad_Hexahedrons_perSpace) 
    {
        Quad_Hexahedrons.insert(Quad_Hexahedrons.end(), v.begin(), v.end());  
    }
}
// Finally, a list of Quad-Hexahedrons is created and used for next GA2

// -------------------------------- //
// --- GA2 : Conformal geometry --- //
// -------------------------------- //

class individual_GA2 {
    public: 

        // Binary notation of individual GA2
        vector<int> quad_cluster;

        // Each '1' in quad_cluster represents a Quad-Hexahedron
        // CFquads = Conformal quad-hexahedrons
        vector<bso::utilities::geometry::quad_hexahedron> CFquads(vector<bso::utilities::geometry::quad_hexahedron> Quad_Hexahedrons) {
            vector<bso::utilities::geometry::quad_hexahedron> CFquads;
            for (int i = 0; i < quad_cluster.size(); i++)
            {
                if (quad_cluster[i] == 1)
                {
                    CFquads.push_back(Quad_Hexahedrons[i]);
                }
            }
            return CFquads;
        }
        
        // fitness calculation of GA2
        vector<double> fitness_GA2(vector<bso::utilities::geometry::quad_hexahedron> Quad_Hexahedrons)
        {
            vector<bso::utilities::geometry::quad_hexahedron> CF_quads;
            double volume_CF = 0;
            for (int i = 0; i < quad_cluster.size(); i++)
            {
                if (quad_cluster[i] == 1)
                {
                    CF_quads.push_back(Quad_Hexahedrons[i]);
                    volume_CF = volume_CF + Quad_Hexahedrons[i].getVolume(); 
                }
            }
            // nCr: combinations claculator
            double nCr;
            double n = CF_quads.size();  
            double r = 2;
            double nr = n - r;
            long double n_fact = 1.0;
            for(int i = 1; i <= n; ++i) { n_fact *= i; }
            long double r_fact = 1.0;
            for(int i = 1; i <= r; ++i) { r_fact *= i; }
            long double nr_fact = 1.0;
            for(int i = 1; i <= nr; ++i) { nr_fact *= i; }
            nCr = n_fact / ( r_fact * nr_fact ) ;

            vector<double> fitness_CF;
            double score_nIntersect; 
            double nIntersections = 0.0;
            double isIntersecting = 0.0;
            double score_nVolume = 0.0;
            double score_nLink;
            double score_nHyperlink;
            double isHyperlinked = 0.0;

            // initialize quad combinations
            set<vector<int>> quadCombinations;
            int k = 2;
            int nCombi = 0;
            for (int j = 1; j < CF_quads.size(); j++) { nCombi += j; }
            for (int k = 0; k < CF_quads.size(); k++) {
                for (int m = k+1; m < CF_quads.size(); m++) {
                    quadCombinations.insert({k , m});
                }
            }

         // VOLUME fitness 
            int nSpaces; double volume_MS2; vector<bso::utilities::geometry::quad_hexahedron> hexaSpaces;
            vector<int> x_coordinates; vector<int> y_coordinates; vector<int> z_coordinates; 
            collectCoordinates("cornerverticesBSD_GA.txt", nSpaces, volume_MS2, hexaSpaces, x_coordinates, y_coordinates, z_coordinates);
            double total_volume_Q = 0;
            for (int i = 0; i < Quad_Hexahedrons.size(); i++) { total_volume_Q = total_volume_Q + Quad_Hexahedrons[i].getVolume(); }
        
            // Compute fitness score of Intersections
            score_nVolume = abs(volume_CF - volume_MS2) / ( total_volume_Q - volume_MS2 );

            fitness_CF.push_back(score_nVolume);
            

         // INTERSECTION fitness
            bso::utilities::geometry::vertex pIntersectioncf;
            double mTol = 1e-3;
            int intersectionsLL = 0;
            int intersectionsLR = 0;
            int intersectionsRR = 0;
            double link = 0.0;               // perfect link between 2 quads, no intersections/overlap
            
            for (auto const &c : quadCombinations)  // check intersections for each combination
            {
                // INTERSECTION (Line - Line)
                intersectionsLL = 0;
                for (unsigned int i = 0; i < CF_quads[c[0]].getLines().size(); ++i)
                {
                    for (unsigned int j = 0; j < CF_quads[c[1]].getLines().size(); ++j)
                    {
                        if (CF_quads[c[0]].getLines()[i].intersectsWith(CF_quads[c[1]].getLines()[j], pIntersectioncf, mTol))
                        {
                            intersectionsLL++;
                            isIntersecting++;
                            goto newquadcombi;
                        }
                    }
                }
                // INTERSECTION (Line - Polygon)
                intersectionsLR = 0;
                for (unsigned int i = 0; i < CF_quads[c[0]].getPolygons().size(); ++i)
                {
                    for (unsigned int j = 0; j < CF_quads[c[1]].getLines().size(); ++j)
                    {
                        if (CF_quads[c[0]].getPolygons()[i]->intersectsWith(CF_quads[c[1]].getLines()[j], pIntersectioncf, mTol))
                        {
                            intersectionsLR++;
                            isIntersecting++;
                            goto newquadcombi;
                        }
                    }
                }
                // INTERSECTION (Polygon - Line)
                for (unsigned int i = 0; i < CF_quads[c[1]].getPolygons().size(); ++i)
                {
                    for (unsigned int j = 0; j < CF_quads[c[0]].getLines().size(); ++j)
                    {
                        if (CF_quads[c[1]].getPolygons()[i]->intersectsWith(CF_quads[c[0]].getLines()[j], pIntersectioncf, mTol))
                        {
                            intersectionsLR++;
                            isIntersecting++;
                            goto newquadcombi;
                        }
                    }
                }

                newquadcombi:
                // INTERSECTION (Overlap)
                if ((intersectionsLL == 0) && (intersectionsLR == 0))
                {
                    intersectionsRR = 0;
                    for (unsigned int j = 0; j < CF_quads[c[1]].getVertices().size(); ++j)
                    {
                        if (CF_quads[c[0]].isInsideOrOn(CF_quads[c[1]].getVertices()[j], mTol))
                        {
                            intersectionsRR++;
                        }
                    }
                    if (intersectionsRR > 4)  
                    {  
                        isIntersecting++;  
                        continue;
                    }
                    intersectionsRR = 0;
                    for (unsigned int j = 0; j < CF_quads[c[0]].getVertices().size(); ++j)
                    {
                        if (CF_quads[c[1]].isInsideOrOn(CF_quads[c[0]].getVertices()[j], mTol))
                        {
                            intersectionsRR++;
                        }
                    }
                    if (intersectionsRR > 4)  
                    {  
                        isIntersecting++;  
                        continue;
                    }

                    if (intersectionsRR == 4)  
                    {  
                        link++;
                    }
                }
            }

            // Compute fitness score of Intersections
            double double_link = double(link);
            if ( (isIntersecting > 0) || (double_link > 0) )
            {
                score_nIntersect = isIntersecting / (isIntersecting + double_link);
            } else {
                score_nIntersect = 0;
            }

            fitness_CF.push_back(score_nIntersect);


            return fitness_CF;
        }
};

// Initialise population of GA2: Conformal model. Make M combinations of quad-hexahedrons
// INPUT    = M : Population Size
//          = Quad-Hexahedrons : found in GA1
//          = probability(min,max) of ones in individual (Chromosome) binary notation. Default = 1%, 100%
// OUTPUT   + population_GA2 : all Individuals GA2
void initializePopulation_GA2(  int M, int probability_1_min, int probability_1_max,  vector<bso::utilities::geometry::quad_hexahedron> Quad_Hexahedrons,
                                vector<individual_GA2> &population_GA2)
{
    population_GA2.reserve(M);
    int nGenes = Quad_Hexahedrons.size();
    int randbinary;
    int randprob;
    double probability_i;
    for (int i = 0; i < M; i++)
    {
        probability_i = double(probability_1_min) + i*(double((probability_1_max-probability_1_min)/M));
        population_GA2.push_back(individual_GA2());
        for (int j = 0; j < nGenes; j++)
        {
            randprob = rand() % 100;         
            if (randprob < probability_i) {
                randbinary = 1;
            } else {
                randbinary = 0;
            }
            population_GA2[i].quad_cluster.push_back( randbinary );
        }
    }
}

// Selection of the fittest individuals
// INPUT    = M, population, Quad-Hexahedrons
// OUTPUT   + parent_GA2 : fittest half (50%) is chosen as parents
//          + fitness_vector : fitness scores (fV & fI) are saved in a 2D vector
void selection_GA2(int M, vector<individual_GA2> population_GA2, vector<bso::utilities::geometry::quad_hexahedron> Quad_Hexahedrons,
                 vector<individual_GA2> &parent_GA2, vector<vector<double>> &fitness_vector)
{
    vector<int> dominate_count(M); 
    vector<int> dominatedBy_count(M);
    for (int i = 0; i < M; i++)
    {
        fitness_vector.push_back(population_GA2[i].fitness_GA2(Quad_Hexahedrons));
    }
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < M; j++)
        {
            if (i != j) 
            {
                // Dominated by other individuals based on 2 objectives
                if ((fitness_vector[i][0] <= fitness_vector[j][0]) && (fitness_vector[i][1] <= fitness_vector[j][1]) )
                {
                    if ((fitness_vector[i][0] < fitness_vector[j][0])  || (fitness_vector[i][1] < fitness_vector[j][1]) ) 
                    {
                        ++dominatedBy_count[j];
                    }
                }
            }
        }
    }
    multimap<int, individual_GA2> mapCF;
    multimap< int, vector<double> > mapFV;
    vector<individual_GA2> sortCF;
    sortCF.reserve(M);
    parent_GA2.reserve(M/2);
    for (int i = 0; i < M; i++)
    {
        mapCF.insert(mapCF.end(), pair<int, individual_GA2>(dominatedBy_count[i], population_GA2[i]) );
        mapFV.insert(mapFV.end(), pair<int, vector<double> >(dominatedBy_count[i], fitness_vector[i]) );
    }
    typedef multimap<int, individual_GA2> mapcfType;
    int i = 0;
    for( mapcfType::iterator it = mapCF.begin(); it != mapCF.end(); ++it )  
    { 
        sortCF.push_back(individual_GA2());
        sortCF[i] = it->second;
        i++;
    }  
    typedef multimap<int, vector<double>> mapfvType;
    int k = 0;
    for( mapfvType::iterator it = mapFV.begin(); it != mapFV.end(); ++it )  
    { 
        cout << "(" << it->first << ") ,  n quads =  " << sortCF[k].CFquads(Quad_Hexahedrons).size() << "\t [ " << it->second[0] << ", " << it->second[1] << " ]   " << endl;  // << ", " << it->second[2] << " ]   " << endl; 
        k++;
    }  
    int j = 0;
    for (int i = 0; i < M/2; i++)
    {
        parent_GA2.push_back(individual_GA2());
        parent_GA2[j] = sortCF[i];
        j++;
    }
}

// Crossover GA2 (Uniform crossover)
// INPUT    = parents GA2
// OUTPUT   + offspring GA2 : 2 parents create 2 new offspring by uniform crossover
void crossover_GA2_uniform( int M, vector<individual_GA2> parent_GA2, vector<bso::utilities::geometry::quad_hexahedron> Quad_Hexahedrons, 
                            vector<individual_GA2> &offspring_GA2)
{
    int sameBit;
    offspring_GA2.reserve(M/2);
    for (int i = 0; i < M/2; i+=2)
    {
        sameBit = 0;
        offspring_GA2.push_back(individual_GA2());
        offspring_GA2.push_back(individual_GA2());
        for (int j = 0; j < Quad_Hexahedrons.size(); j++)
        {
            int rand_j = rand() % 2;  
            int rand_k = (rand_j+1)%2;             
            offspring_GA2[i+0].quad_cluster.push_back(parent_GA2[i+rand_j].quad_cluster[j]);            
            offspring_GA2[i+1].quad_cluster.push_back(parent_GA2[i+rand_k].quad_cluster[j]);
            if (parent_GA2[i+rand_j].quad_cluster[j] == parent_GA2[i+rand_k].quad_cluster[j])
            { sameBit++; }
        }
        if (sameBit == Quad_Hexahedrons.size()) // if parents are exactly the same, the parents are shuffled
        { 
            random_shuffle(offspring_GA2[i+0].quad_cluster.begin(), offspring_GA2[i+0].quad_cluster.end()) ;
            random_shuffle(offspring_GA2[i+1].quad_cluster.begin(), offspring_GA2[i+1].quad_cluster.end()) ;
        }   
    }
}

// Merge parents and offspring 
void merge_GA2(int M, vector<individual_GA2> parent_GA2, vector<individual_GA2> offspring_GA2, 
            vector<individual_GA2> &merged_population_GA2)
{
    merged_population_GA2.reserve(M);
    for (int m = 0; m < M; m+=2)
    {
        merged_population_GA2.push_back(individual_GA2());
        merged_population_GA2.push_back(individual_GA2());
        merged_population_GA2[m] = parent_GA2[m/2];
        merged_population_GA2[m+1] = offspring_GA2[m/2];
    }
}

// Mutation GA2
// INPUT    = mutation_rate_population : if 0.50, then 50% of population size (M) is mutated
//          = mutation_rate_individual : if 1.00, one binary in the quad-cluster is flipped. 0 becomes 1 or vice versa
// OUTPUT   + Mutated population GA2
void mutation_GA2(  int M, double mutation_rate_population, double mutation_rate_individual, vector<individual_GA2> merged_population_GA2, 
                    vector<individual_GA2> &mutated_population_GA2)
{   
    cout << mutation_rate_population*merged_population_GA2.size() << endl;
    for (int i = 0; i < (mutation_rate_population*merged_population_GA2.size()); i++)   
    {
        for (int j = 0; j < mutation_rate_individual; j++ ) 
        {
            int randi = rand() % merged_population_GA2[i].quad_cluster.size(); 
            if (merged_population_GA2[i].quad_cluster[randi] == 0) {
                merged_population_GA2[i].quad_cluster[randi] = 1;
            } else if (merged_population_GA2[i].quad_cluster[randi] == 1) {
                merged_population_GA2[i].quad_cluster[randi] = 0;
            }
        }
    }
    mutated_population_GA2 = merged_population_GA2;
}


int main(int argc, char *argv[])
{
 // Pre-Processing BSD Data (collect Coordinates & generate Point Cloud)
    int nSpaces; double total_volume_MS;
    vector<bso::utilities::geometry::quad_hexahedron> hexaSpaces;
    vector<int> x_coordinates; vector<int> y_coordinates; vector<int> z_coordinates;
    collectCoordinates("cornerverticesBSD_GA.txt", nSpaces, total_volume_MS, hexaSpaces, x_coordinates, y_coordinates, z_coordinates);
    
    vector<vector<double>> PointCloud;
    generatePointCloud(nSpaces, hexaSpaces, x_coordinates, y_coordinates, z_coordinates, PointCloud);


 // GA1 : Quad-Hexahedrons
    // Input Parameters
    int N = 128;                                // Population size
    int g = 0;                                  // generation counter
    vector<int> cho = {0,0,0};                  // crossover indicator
    int n = 100;                                // after n generations where no new unique Quad-Hexahedron is found, GA1 stops
    vector<int> vec_n; 
    int x = 1; 
    int uniqueQHs = 2;

    // Initialize Population GA1
    vector<individual_GA1> population_GA1;
    initializePopulation_GA1( N, PointCloud, population_GA1);
    
    vector<individual_GA1> perfect_Quad_Hexahedron;
    vector<individual_GA1> saved_Quad_Hexahedrons;
    map< vector<vector<double>>, vector<vector<double>> > saved_Quad_Hexahedrons_map;
    

    // Start evolution GA1
    while ( uniqueQHs > 1 )
    {
        cout << "\ngeneration (" << g << ") " << endl;
        perfect_Quad_Hexahedron.clear();
        saved_Quad_Hexahedrons.clear();

        // Mutation 1
        vector<individual_GA1> mutated1_population_GA1;
        mutation1_GA1(population_GA1, N, PointCloud, mutated1_population_GA1);     
        population_GA1 = mutated1_population_GA1; 
        random_shuffle(population_GA1.begin(), population_GA1.end());

        // Crossovers X, Y, Z: alternately applied
        vector<individual_GA1> offspring;
        int choI = min_element(cho.begin(), cho.end()) - cho.begin();
        if      (choI == 0) { crossoverX(population_GA1, N, offspring); cho[0]++; cout << "crossoverX" << endl; } 
        else if (choI == 1) { crossoverY(population_GA1, N, offspring); cho[1]++; cout << "crossoverY" << endl; } 
        else if (choI == 2) { crossoverZ(population_GA1, N, offspring); cho[2]++; cout << "crossoverZ" << endl; }
        
        // Merge
        vector<individual_GA1> merged_population_GA1;
        merge(population_GA1, offspring, N, merged_population_GA1);

        // Mutation 2
        vector<individual_GA1> mutated2_population_GA1;
        mutation2_GA1(merged_population_GA1, PointCloud, mutated2_population_GA1);

        // Selection
        vector<individual_GA1> new_population_GA1;
        selection_GA1(mutated2_population_GA1, new_population_GA1);
        population_GA1 = new_population_GA1;
        
        offspring.clear();
        merged_population_GA1.clear();
        mutated2_population_GA1.clear();
        new_population_GA1.clear();

        // Save individuals if they are perfect Quad-hexahedrons
        get_perfect_Quad_Hexahedron(population_GA1, perfect_Quad_Hexahedron);
        save_perfect_Quad_Hexahedron(perfect_Quad_Hexahedron, saved_Quad_Hexahedrons_map, saved_Quad_Hexahedrons);

        // Check if new quad-hexahedron is found
        vec_n.push_back(saved_Quad_Hexahedrons_map.size());
        if (g == x*n) { uniqueQHs = unique(vec_n.begin(), vec_n.end()) - vec_n.begin();  x++; vec_n.clear(); }

        g++;
    }
    // Check if found quad-hexahedrons are inside one of the BSD spaces
    vector<bso::utilities::geometry::quad_hexahedron> Quad_Hexahedrons;
    vector<vector<individual_GA1>> Quad_Hexahedrons_InsidePerSpace;
    vector<vector<bso::utilities::geometry::quad_hexahedron>> Quad_Hexahedrons_perSpace;
    isInsideBSD(saved_Quad_Hexahedrons, hexaSpaces, Quad_Hexahedrons_InsidePerSpace, Quad_Hexahedrons_perSpace, Quad_Hexahedrons);

    // Print relevant information about GA1
    cout << "\n\n --- GA1: Quad-Hexahedrons  --- " << endl;
    cout << "Population size N:        " << N << endl;
    cout << "Total generations:        " << g << endl;
    cout << "Quad-Hexahedrons found:   " << Quad_Hexahedrons.size() << endl;
    cout << "Volume BSD:               " << total_volume_MS << endl;
    cout << "\n";
    cout << "\n \n" ;
    for ( int i = 0; i < Quad_Hexahedrons.size(); i++)
    {
        cout << "quad: (" << i << ") " << endl;
        for (int p = 0; p < 8; p++)
        {
            cout << Quad_Hexahedrons[i].getVertices()[p][0] << ", " << Quad_Hexahedrons[i].getVertices()[p][1] << ", " << Quad_Hexahedrons[i].getVertices()[p][2] << endl;
        }
    }


 // GA2 : Conformal model
    // Input Parameters:
    int M = 8;                                  // Population size
    int probability_1_min = 10;                 // minimal probability of ones in individual GA2
    int probability_1_max = 100;                // maximal probability of ones in individual GA2
    double mutation_rate_population = 0.5;      // mutation rate of population (nr. of individuals to mutate)
    double mutation_rate_individual = 1;        // mutate rate of individuals  (nr. of bits to flip)
    int g2 = 0;
    bool CFmodelIsNotFound = true;

    // Initialize Population GA2
    vector<individual_GA2> population_GA2;
    initializePopulation_GA2( M, probability_1_min, probability_1_max , Quad_Hexahedrons, population_GA2 );
    
    ofstream fv("fitness_vector.txt");
    vector<individual_GA2> perfect_CFmodel;
    
    // Start evolution (GA2)
    while (CFmodelIsNotFound)
    {
        cout << "! --- generation " << g2 << endl;

        // Selection
        random_shuffle(population_GA2.begin(), population_GA2.end());
        vector<individual_GA2> parent_GA2;
        vector<vector<double>> fitness_vector;
        selection_GA2(M, population_GA2, Quad_Hexahedrons, parent_GA2, fitness_vector);
        for (int i = 0; i < M; i++)
        {  fv << fitness_vector[i][0] << ", " << fitness_vector[i][1] << endl; }
        for (int i = 0; i < parent_GA2.size(); i++)
        {
            cout << "{ " ;
            for (int j = 0; j < Quad_Hexahedrons.size(); j++)
            {
                cout << parent_GA2[i].quad_cluster[j] << "," ;
            }
            cout << " }" << endl;
        }

        // Check if perfect conformal model is found  (fitness vector = [0,0])
        for (int i = 0; i < fitness_vector.size(); i++)
        {
            if ( (fitness_vector[i][0] == 0 ) && ( fitness_vector[i][1] == 0 )) {    //&& (parent_GA2.back().fitness(Quad_Hexahedrons)[2] <= 0.5  ) ) {
                perfect_CFmodel.push_back(individual_GA2());
                perfect_CFmodel[0] = population_GA2[i];
                CFmodelIsNotFound = false;
            }
        }

        // Crossover (Uniform)
        random_shuffle(parent_GA2.begin(), parent_GA2.end());
        vector<individual_GA2> offspring_GA2;
        crossover_GA2_uniform(M, parent_GA2, Quad_Hexahedrons, offspring_GA2);

        // Merge
        vector<individual_GA2> merged_population_GA2;
        merge_GA2(M, parent_GA2, offspring_GA2, merged_population_GA2);
        
        // Mutation
        random_shuffle(merged_population_GA2.begin(), merged_population_GA2.end());
        vector<individual_GA2> mutated_population_GA2;
        mutation_GA2(M, mutation_rate_population, mutation_rate_individual,  merged_population_GA2, mutated_population_GA2);
        
        population_GA2.clear();
        population_GA2 = mutated_population_GA2;
        parent_GA2.clear();
        offspring_GA2.clear();
        merged_population_GA2.clear();
        mutated_population_GA2.clear();

        g2++;
        cout << endl << endl;
    }

    // Save Conformal model in "cf.txt" file, so it can be used and visualised
    ofstream resultCF("cf.txt");
    for (int q = 0; q < perfect_CFmodel[0].CFquads(Quad_Hexahedrons).size(); q++)
    {
        for (int p = 0; p < 8; p++) {
            resultCF << perfect_CFmodel[0].CFquads(Quad_Hexahedrons)[q].getVertices()[p][0] << "\n" << perfect_CFmodel[0].CFquads(Quad_Hexahedrons)[q].getVertices()[p][1] << "\n" << perfect_CFmodel[0].CFquads(Quad_Hexahedrons)[q].getVertices()[p][2] << endl; 
        }
    }
    
    
    return 0;
}
