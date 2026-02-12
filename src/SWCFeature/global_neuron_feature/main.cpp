#include <iostream>
#include <fstream>
#include <cstdlib>
#include "basic_surf_objs.h"
#include "compute_morph.h"

using namespace std;

void printUsage()
{
    cout << "Usage: global_neuron_feature <input.swc> [output.txt]" << endl;
    cout << "  input.swc   : input SWC/ESWC file" << endl;
    cout << "  output.txt  : (optional) output feature file; if omitted, prints to stdout" << endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printUsage();
        return 1;
    }

    QString inputFile = QString(argv[1]);
    NeuronTree nt = readSWC_file(inputFile);
    if (nt.listNeuron.isEmpty())
    {
        cerr << "Error: failed to read SWC file or file is empty: " << inputFile.toStdString() << endl;
        return 1;
    }

    double features[FNUM] = {0};
    computeFeature(nt, features);

    if (argc >= 3)
    {
        QString outputFile = QString(argv[2]);
        ofstream ofs(outputFile.toStdString().c_str());
        if (!ofs.is_open())
        {
            cerr << "Error: cannot open output file: " << outputFile.toStdString() << endl;
            return 1;
        }

        const char* featureNames[FNUM] = {
            "Number of Nodes",
            "Soma Surface",
            "Number of Stems",
            "Number of Bifurcations",
            "Number of Branches",
            "Number of Tips",
            "Overall Width",
            "Overall Height",
            "Overall Depth",
            "Average Diameter",
            "Total Length",
            "Total Surface",
            "Total Volume",
            "Max Euclidean Distance",
            "Max Path Distance",
            "Max Branch Order",
            "Average Contraction",
            "Average Fragmentation",
            "Average Parent-daughter Ratio",
            "Average Bifurcation Angle Local",
            "Average Bifurcation Angle Remote",
            "Hausdorff Dimension"
        };

        for (int i = 0; i < FNUM; i++)
            ofs << featureNames[i] << "\t" << features[i] << endl;

        ofs.close();
        cout << "Features written to: " << outputFile.toStdString() << endl;
    }
    else
    {
        printFeature(features);
    }

    return 0;
}
