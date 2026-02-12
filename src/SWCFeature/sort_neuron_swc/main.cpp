#include <iostream>
#include <cstdlib>
#include "basic_surf_objs.h"
#include "swc_io.h"
#include "sort_swc.h"

using namespace std;

void printUsage()
{
    cout << "Usage: sort_neuron_swc <input.swc> <output.swc> [threshold] [root_id]" << endl;
    cout << "  input.swc   : input SWC/ESWC file" << endl;
    cout << "  output.swc  : output sorted SWC/ESWC file" << endl;
    cout << "  threshold   : (optional) distance threshold for connecting components" << endl;
    cout << "                default=1000000000 (connect all components)" << endl;
    cout << "  root_id     : (optional) node id to use as root" << endl;
    cout << "                default=auto (use first root node in file)" << endl;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printUsage();
        return 1;
    }

    QString inputFile  = QString(argv[1]);
    QString outputFile = QString(argv[2]);

    double thres = VOID_VALUE;
    V3DLONG rootid = VOID_VALUE;

    if (argc >= 4)
        thres = atof(argv[3]);
    if (argc >= 5)
        rootid = atol(argv[4]);

    NeuronTree nt = readSWC_file(inputFile);
    if (nt.listNeuron.isEmpty())
    {
        cerr << "Error: failed to read SWC file or file is empty: " << inputFile.toStdString() << endl;
        return 1;
    }

    cout << "Input neuron size: " << nt.listNeuron.size() << endl;
    if (thres != VOID_VALUE)
        cout << "Distance threshold: " << thres << endl;
    if (rootid != VOID_VALUE)
        cout << "Root id: " << rootid << endl;

    QList<NeuronSWC> result;
    if (!SortSWC(nt.listNeuron, result, rootid, thres))
    {
        cerr << "Error: SortSWC failed." << endl;
        return 1;
    }

    cout << "Output neuron size: " << result.size() << endl;

    if (!export_list2file(result, outputFile, inputFile))
    {
        cerr << "Error: failed to write output file." << endl;
        return 1;
    }

    return 0;
}
