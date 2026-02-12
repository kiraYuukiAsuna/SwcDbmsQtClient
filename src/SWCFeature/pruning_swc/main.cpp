#include <iostream>
#include <cstdlib>
#include <cstring>
#include "basic_surf_objs.h"
#include "swc_io.h"
#include "pruning_swc_algo.h"

using namespace std;

void printUsage()
{
    cout << "Usage: pruning_swc <input.swc> <output.swc> <threshold> [iterative]" << endl;
    cout << "  input.swc   : input SWC/ESWC file" << endl;
    cout << "  output.swc  : output pruned SWC/ESWC file" << endl;
    cout << "  threshold   : maximum pruned segment length (positive float, unit: pixel)" << endl;
    cout << "  iterative   : (optional) if specified, use iterative pruning mode" << endl;
    cout << endl;
    cout << "Examples:" << endl;
    cout << "  pruning_swc input.swc output.swc 10.0" << endl;
    cout << "  pruning_swc input.swc output.swc 2000.0 iterative" << endl;
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        printUsage();
        return 1;
    }

    QString inputFile  = QString(argv[1]);
    QString outputFile = QString(argv[2]);
    double threshold   = atof(argv[3]);

    bool use_iterative = false;
    if (argc >= 5 && strcmp(argv[4], "iterative") == 0)
        use_iterative = true;

    if (threshold <= 0)
    {
        cerr << "Error: threshold must be a positive number." << endl;
        return 1;
    }

    NeuronTree nt = readSWC_file(inputFile);
    if (nt.listNeuron.isEmpty())
    {
        cerr << "Error: failed to read SWC file or file is empty: " << inputFile.toStdString() << endl;
        return 1;
    }

    cout << "Input neuron size: " << nt.listNeuron.size() << endl;
    cout << "Threshold: " << threshold << endl;
    cout << "Mode: " << (use_iterative ? "iterative" : "simple") << endl;

    bool pruned = false;
    NeuronTree result;

    if (use_iterative)
        result = prune_swc_iterative(nt, threshold, pruned);
    else
        result = prune_swc_simple(nt, threshold, pruned);

    cout << "Output neuron size: " << result.listNeuron.size() << endl;

    if (!export_list2file(result.listNeuron, outputFile, inputFile))
    {
        cerr << "Error: failed to write output file." << endl;
        return 1;
    }

    return 0;
}
