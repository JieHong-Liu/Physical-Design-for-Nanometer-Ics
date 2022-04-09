#include <iostream>
#include <fstream>
#include <vector>
#include "floorplanner.h"
using namespace std;



int main(int argc, char** argv)
{
    fstream input_blk, input_net, output;
    double alpha;

#if 1
    if (argc == 5) {
        alpha = stod(argv[1]);
        input_blk.open(argv[2], ios::in);
        input_net.open(argv[3], ios::in);
        output.open(argv[4], ios::out);
        if (!input_blk) {
            cerr << "Cannot open the input file \"" << argv[2]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
        if (!input_net) {
            cerr << "Cannot open the input file \"" << argv[3]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
        if (!output) {
            cerr << "Cannot open the output file \"" << argv[4]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
    }
    else {
        cerr << "Usage: ./Floorplanner <alpha> <input block file> " <<
                "<input net file> <output file>" << endl;
        exit(1);
    }
#endif
#if 0
    alpha = 0.5;
    input_blk.open("input_pa2/ami49.block", ios::in);
    input_net.open("input_pa2/ami49.nets", ios::in);
    output.open("output.rpt", ios::out);
#endif
    Floorplanner* fp = new Floorplanner(input_blk, input_net, output, alpha);
    fp->floorplan();

    return 0;
}
