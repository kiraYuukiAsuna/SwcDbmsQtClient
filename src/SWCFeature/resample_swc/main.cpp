#include <iostream>
#include <cstdlib>
#include "basic_surf_objs.h"
#include "swc_io.h"
#include "resampling.h"

using namespace std;

void printUsage()
{
    cout << "Usage: resample_swc <input.swc> <output.swc> <step>" << endl;
    cout << "  input.swc   : input SWC/ESWC file" << endl;
    cout << "  output.swc  : output resampled SWC/ESWC file" << endl;
    cout << "  step        : resampling step length (positive float)" << endl;
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
    double step = atof(argv[3]);

    if (step <= 0)
    {
        cerr << "Error: step must be a positive number." << endl;
        return 1;
    }

    NeuronTree nt = readSWC_file(inputFile);
    if (nt.listNeuron.isEmpty())
    {
        cerr << "Error: failed to read SWC file or file is empty: " << inputFile.toStdString() << endl;
        return 1;
    }

    cout << "Input neuron size: " << nt.listNeuron.size() << endl;
    cout << "Resampling with step: " << step << endl;

    NeuronTree result = resample(nt, step);

    cout << "Output neuron size: " << result.listNeuron.size() << endl;

    if (!export_list2file(result.listNeuron, outputFile, inputFile))
    {
        cerr << "Error: failed to write output file." << endl;
        return 1;
    }

    return 0;
}
